#include "StdAfx.h"
#include "SqLiteRecordsetImpl.h"

#include "sqlite_copy_table.h"
#include "sqlite_bind_util.h"
#include "sqlite_table_info.h"
#include "sqlite_include.h"

#include "SqLiteDatabaseImpl.h"
#include "SqLiteErrorHandler.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// http://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite

//http://www.sqlite.org/cvstrac/wiki?p=SampleCode
   /** Timeout values on busy or lock conditions   **/
   /** if you observe lock errors you might try to **/
   /** increase the values.                        **/
   //#define SQLTM_COUNT       200  /** -> SQLTM_COUNT*SQLTM_TIME ms timeout **/
   //#define SQLTM_TIME        50
/*
  int    PrepareSql(sqlite3 *db, sqlite3_stmt  **hs, char *SqlStr, int handle)
   {
       int        n, rc;

       n = 0;

       do
       {
           rc = sqlite3_prepare_v2(db, SqlStr, -1, hs, 0);

           if ( (rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED) )
           {
               n++;
               Sleep(SQLTM_TIME);
           }
       }while( (n < SQLTM_COUNT) && ((rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED)));

       if ( rc != SQLITE_OK)
       {
           EnterCriticalSection(&stderr_lock);
           fprintf(stderr, "SqlPrepare-Error-H(%d): (%d) %s \n", handle, rc, sqlite3_errmsg(db));
           fprintf(stderr, "Statement: %s \n", SqlStr);
           LeaveCriticalSection(&stderr_lock);

           return(0);
       }

       return(1);
   }

  */

static void OnColumnIndexFailed(CSqLiteErrorHandler *pErrorHandler, const char *sFieldName, const char *sFunctionName, const char *sTableName) 
{
    std::string sError   = "Unable to find field: ";
                sError += sFieldName;
                sError += ".";
                sError += " Table: ";
                sError += sTableName;
                sError += ".";

    pErrorHandler->OnError(sError.c_str(), sFunctionName);
}

CSqLiteRecordsetImpl::CSqLiteRecordsetImpl(CSqLiteDatabaseImpl *pDatabase, CSqLiteErrorHandler *pErrorHandler)
: m_pDB(pDatabase), m_pErrorHandler(pErrorHandler), m_bEOF(true), m_stmt(nullptr), m_nEditRowId(-1), 
  m_pSaveData(nullptr), m_pFieldInfoData(nullptr), m_insert_stmt(nullptr), m_bSQLOpened(false), m_update_stmt(nullptr)
{
    
}

CSqLiteRecordsetImpl::~CSqLiteRecordsetImpl() 
{ 
    delete m_pSaveData;
    sqlite3_finalize(m_update_stmt);
    sqlite3_finalize(m_insert_stmt);
	sqlite3_finalize(m_stmt);
}

void CSqLiteRecordsetImpl::CloseStatement()
{
    int rc = sqlite3_finalize(m_stmt);
    rc;
    m_stmt = nullptr;
    m_name_to_index.clear();
    m_bEOF = true;
}

bool CSqLiteRecordsetImpl::Open(LPCTSTR sTableName)
{
	m_sTable = ds_str_conv::ConvertToUTF8(sTableName);
    return true;
}

bool CSqLiteRecordsetImpl::OpenSQL(LPCTSTR sSQL)
{
    const std::string sSQLUTF8 = ds_str_conv::ConvertToUTF8(sSQL);
	const bool bRet = OpenSQLUTF8(sSQLUTF8.c_str());
	m_bSQLOpened = true;
	return bRet;
}

bool CSqLiteRecordsetImpl::OpenSQLUTF8(const char *sSQL)
{
	CloseStatement();

    if ( OpenImpl(sSQL) ) {
        return true;
    }

    return false;
}

bool CSqLiteRecordsetImpl::OpenView(LPCTSTR sViewName)
{
    // NOTE: OpenImpl() is not completely correct for the VIEW
    // VIEW does not contain ROWID
	m_sTable = ds_str_conv::ConvertToUTF8(sViewName);
    return true;
}

void CSqLiteRecordsetImpl::SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)
{
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataBinary(pData, nSize);
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

void CSqLiteRecordsetImpl::GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize)
{
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldBinary", m_sTable.c_str());
        return;
    }
	nSize = ::sqlite3_column_bytes(m_stmt, nColumnIndex);
    *pData = (unsigned char *)::sqlite3_column_blob(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::FreeBinary(unsigned char *pData)
{
    //#24398
    //http://stackoverflow.com/questions/325158/does-the-pointer-get-from-sqlite3-column-blob-method-need-to-be-delete
    // > From the sqllite documentation:
    // > The pointers returned are valid until a type conversion occurs or until sqlite3_step() or sqlite3_reset() or
    // > sqlite3_finalize() is called. The memory space used to hold strings and BLOBs is freed automatically.
    // > Do not pass the pointers returned by sqlite3_column_blob() into sqlite3_free().
    //
    //free(pData); //original delete from old DAO implementation
}

static sqlite3_stmt *Prepare(sqlite3 *pDB, const char *sql, CSqLiteErrorHandler *pErrorHandler)
{
    const char *tail   = nullptr;
    sqlite3_stmt *stmt = nullptr;
    const int rc = ::sqlite3_prepare_v2(pDB, sql, -1, &stmt, &tail);

    if (rc != SQLITE_OK)  {
        const char* localError = sqlite3_errmsg(pDB);
        pErrorHandler->OnError(rc, localError, _T("Prepare(sqlite3 *pDB....)"));
        CStdString sError = _T("SQL statement: ");
        sError += ds_str_conv::ConvertFromUTF8(sql);
        pErrorHandler->OnError(sError.c_str(), _T("Prepare(sqlite3 *pDB....)"));
        ::sqlite3_finalize(stmt);
        return nullptr;
    }

    return stmt;
}

bool CSqLiteRecordsetImpl::Delete()
{
    const int nRowId = ::sqlite3_column_int(m_stmt, 0); // 0 column always holds row id

    const std::string sRowId = ds_str_conv::long_to_string(nRowId);
    
    std::string sDelete  = "DELETE FROM ";
                sDelete += m_sTable;
                sDelete += " WHERE ROWID = ";
                sDelete += sRowId;
	//sDelete.Format("DELETE FROM %s WHERE ROWID = %d", m_sTable.c_str(), nRowId);
    const int nRetVal = m_pDB->ExecuteUTF8(sDelete.c_str());
    if ( nRetVal == -1 ) {
        OnErrorCode(nRetVal, _T("CSqLiteRecordsetImpl::Delete()"));
        return false;
    }
    return true;
}
/*
 To avoid problems, the current thread should use sqlite3_mutex_enter() to acquire exclusive access to the database 
 connection before the initial API call is made. The thread can release the mutex after sqlite3_last_insert_rowid() is called. 
 In “multithread” mode, it is the responsibility of the application to control access to the database connection.

QString DBBrowserDB::emptyInsertStmt(const sqlb::Table& t, const QString& pk_value) const
{
    QString stmt = QString("INSERT INTO `%1`").arg(t.name());

    QStringList vals;
    QStringList fields;
    foreach(sqlb::FieldPtr f, t.fields()) 
    {
        if (f->primaryKey()) 
        {
            fields << f->name();

            if (!pk_value.isNull())
            {
                vals << pk_value;
            } else {
                if (f->notnull())
                {
                    QString maxval = this->max(t, f);
                    vals << QString::number(maxval.toLongLong() + 1);
                }
                else
                {
                    vals << "NULL";
                }
            }
        } 
        else if (f->notnull() && f->defaultValue().length() == 0) 
        {
            fields << f->name();

            if (f->isInteger())
                vals << "0";
            else
                vals << "''";
        } 
        else 
        {
            // don't insert into fields with a default value
            // or we will never see it.
            if (f->defaultValue().length() == 0)
            {
                fields << f->name();
                vals << "NULL";
            }
        }
    }

    if (!fields.empty())
    {
        stmt.append("(`");
        stmt.append(fields.join("`,`"));
        stmt.append("`)");
    }
    stmt.append(" VALUES (");
    stmt.append(vals.join(","));
    stmt.append(");");

    return stmt;
}
*/
void CSqLiteRecordsetImpl::PrepareInsert()
{
    ASSERT(!m_pSaveData);
    m_pSaveData = new sqlite_util::CFieldDataMap;
}

void CSqLiteRecordsetImpl::AddNew()
{
    //ASSERT(m_stmt); // open should be called
    ASSERT(m_nEditRowId == -1);
    PrepareInsert();
  
    DoInsertDefault(); // do insert default record if possible
                // required to support logic like this:
                //  AddNew()
                //      GetFieldLong("ID"); // primary key
                //  Update()
    SeekByLongUTF8("ROWID", (long)m_nEditRowId); 
}

void CSqLiteRecordsetImpl::Edit()
{
    ASSERT(!m_pSaveData);
    ASSERT(m_stmt); // open should be called
    m_pSaveData = new sqlite_util::CFieldDataMap;
    m_nEditRowId = ::sqlite3_column_int(m_stmt, 0); // 0 column always holds row id
}

namespace internal
{
    static void AddValueImpl(std::string &sValues, const char *value)
    {
        if ( sValues.empty() ) {
            sValues = value;
        }
        else {
            sValues += ", ";
            sValues += value;
        }
    }
};

void CSqLiteRecordsetImpl::DoInsertDefault()
{
    ASSERT(m_sTable.length() > 0);

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    int rc = SQLITE_OK;
    if ( !m_insert_stmt )
    {
        if ( !m_pFieldInfoData ) {
            m_pFieldInfoData = m_pDB->GetTableFieldInfoImpl(m_sTable.c_str()); // could be a cache on the database level
            if ( !m_pFieldInfoData ) {
                return;
            }
        }

        std::string sColumns;
        std::string sValues;

        // do generate empty statement like: DBBrowserDB::emptyInsertStmt
        auto beg_it = m_pFieldInfoData->begin();
        auto end_it = m_pFieldInfoData->end();
        for (auto it = beg_it; it != end_it; ++it) 
        {
            if ( it->second.m_bPrimary ) 
            {
                //  primary key value as paramater unsupported
                //fields << f->name();
                //if ( !pk_value.isNull() ) {
                //    vals << pk_value;
                //} else {
                if ( it->second.m_bNotNull || it->second.m_bAutoIncrement )
                {
                    // unsupported situation, auto increment expected
                    //QString maxval = this->max(t, f);
                    //vals << QString::number(maxval.toLongLong() + 1);
                }
                else {
                    internal::AddValueImpl(sColumns, it->first.c_str());    
                    internal::AddValueImpl(sValues, "NULL");
                }
                //}
            } 
            else if ( it->second.m_bNotNull ) 
            {
                internal::AddValueImpl(sColumns, it->first.c_str());

                if ( it->second.m_sDefault.length() == 0 ) {
                    if ( it->second.m_sType == "INTEGER" || it->second.m_sType == "REAL" ) {
                        internal::AddValueImpl(sValues, "0");
                    }
                    else {
                        internal::AddValueImpl(sValues, "''");
                    }
                }
                else {
                    internal::AddValueImpl(sValues, it->second.m_sDefault.c_str());
                }
            } 
            else if ( it->second.m_sDefault.length() > 0 ) {
                internal::AddValueImpl(sColumns, it->first.c_str());
                internal::AddValueImpl(sValues, it->second.m_sDefault.c_str());
            }
            else {
                internal::AddValueImpl(sColumns, it->first.c_str());
                internal::AddValueImpl(sValues, "NULL");
            }
        }
    
        std::string sSql = "INSERT INTO ";
        sSql += m_sTable.c_str();
        sSql += " (";
        sSql += sColumns.c_str();
        sSql += ")";
        sSql += " VALUES (";
        sSql += sValues.c_str();
        sSql += ")";
   
        const char* pTail = nullptr;
        rc = ::sqlite3_prepare_v2(pDB, sSql.c_str(), -1, &m_insert_stmt, &pTail);

        if (rc != SQLITE_OK) 
        {
           OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_prepare_v2)"));
           CStdString sError = _T("SQL statement:");
           sError += ds_str_conv::ConvertFromUTF8(sSql.c_str()).c_str();
           m_pErrorHandler->OnError(sError.c_str(), _T("CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_prepare_v2)"));
           return;
        }
    }

    ASSERT(rc == SQLITE_OK);
    
    sqlite_util::sqlite_bind_statements(*m_pSaveData, m_insert_stmt);
    rc = ::sqlite3_step(m_insert_stmt);
    if ( rc != SQLITE_DONE ) {
        const char *sql = sqlite3_sql(m_insert_stmt);
        CStdString sError = _T("SQL statement:");
        sError += ds_str_conv::ConvertFromUTF8(sql).c_str();
        m_pErrorHandler->OnError(sError.c_str(), _T("CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_step)"));
        OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_step)"));
    }
    ::sqlite3_clear_bindings(m_insert_stmt);
    ::sqlite3_reset(m_insert_stmt);
    
    // multi-threading unsuported
    m_nEditRowId = ::sqlite3_last_insert_rowid(pDB);
}

static inline std::string save_data_to_update_values_string(sqlite_util::CFieldDataMap *pSaveData)
{
    ASSERT(pSaveData);

    std::string sValues;
    auto end_it = pSaveData->end();
    auto beg_it = pSaveData->begin();
    for (auto it = beg_it; it != end_it; ++it) {
        if ( sValues.empty() ) {
            sValues += it->first;
            sValues += "=?";
        }
        else {
            sValues += ",";
            sValues += it->first;
            sValues += "=?";
        }
    }
    return sValues;
}

bool CSqLiteRecordsetImpl::DoUpdate()
{
    //sqlite3_finalize(m_update_stmt);
    //m_update_stmt = nullptr;

    const std::string sValues = save_data_to_update_values_string(m_pSaveData);
    if ( m_sUpdateValues != sValues ) 
    {
        m_sUpdateValues = sValues;
        sqlite3_finalize(m_update_stmt);
        m_update_stmt = nullptr;
    }
    
    ASSERT(m_nEditRowId > 0);
   
    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    int rc = 0;

    if ( !m_update_stmt )
    {
        const char* pTail = nullptr;
        CStdStringA sSql;
        sSql.Format("UPDATE %s SET %s WHERE ROWID = ?", m_sTable.c_str(), sValues.c_str());     
        rc = ::sqlite3_prepare_v2(pDB, sSql.c_str(), -1, &m_update_stmt, &pTail);
        if (rc != SQLITE_OK) 
        {
            OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoUpdate()(sqlite3_prepare_v2)"));
            sqlite3_finalize(m_update_stmt);
            m_update_stmt = nullptr;
        }
    }

    if (m_update_stmt) 
    {
        const int nIndex = sqlite_util::sqlite_bind_statements(*m_pSaveData, m_update_stmt);
        ::sqlite3_bind_int64(m_update_stmt, nIndex, m_nEditRowId);                            
        rc = ::sqlite3_step(m_update_stmt);

        ::sqlite3_clear_bindings(m_update_stmt);
        ::sqlite3_reset(m_update_stmt);
    }
   
    if ( rc != SQLITE_DONE ) 
    {
        CStdStringA sError;
        sError.Format("SQL statement: UPDATE %s SET %s WHERE ROWID = %d", m_sTable.c_str(), sValues.c_str(), m_nEditRowId);     
        m_pErrorHandler->OnError(rc, sError.c_str(), _T("CSqLiteRecordsetImpl::DoUpdate()(sqlite3_finalize)"));
        return false;
    }

    return true;
}

bool CSqLiteRecordsetImpl::Update()
{
    ASSERT(m_pSaveData);

    const bool bRetVal = DoUpdate();

    delete m_pSaveData;
    m_pSaveData = nullptr;

    m_nEditRowId = -1;

	return bRetVal;
}

static inline std::string save_data_to_insert_columns_string(sqlite_util::CFieldDataMap *pSaveData)
{
    ASSERT(pSaveData);

    std::string sValues;
    auto end_it = pSaveData->end();
    auto beg_it = pSaveData->begin();
    for (auto it = beg_it; it != end_it; ++it) {
        if ( sValues.empty() ) {
            sValues += it->first;
        }
        else {
            sValues += ",";
            sValues += it->first;
        }
    }
    return sValues;
}

static inline std::string save_data_to_insert_values_string(sqlite_util::CFieldDataMap *pSaveData)
{
    ASSERT(pSaveData);

    std::string sValues;
    auto end_it = pSaveData->end();
    auto beg_it = pSaveData->begin();
    for (auto it = beg_it; it != end_it; ++it) {
        if ( sValues.empty() ) {
            sValues += "?";
        }
        else {
            sValues += ",";
            sValues += "?";
        }
    }
    return sValues;
}

void CSqLiteRecordsetImpl::CommitInsert()
{
    ASSERT(m_pSaveData);
    // TODO: save/update/insert/add_new operations should be extracted from CSqLiteRecordsetImpl
    // CSqLiteRecordsetImpl should delegate only calls for the extracted operations

    const std::string sColumns = save_data_to_insert_columns_string(m_pSaveData);
    const std::string sValues  = save_data_to_insert_values_string(m_pSaveData);

    std::string sSql  = "INSERT INTO ";
                sSql += m_sTable.c_str();
                sSql += " (";
                sSql += sColumns.c_str();
                sSql += ")";
                sSql += " VALUES (";
                sSql += sValues.c_str();
                sSql += ")";

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    const char* pTail = nullptr;
    sqlite3_stmt *pStmt = nullptr;
    int rc = ::sqlite3_prepare_v2(pDB, sSql.c_str(), -1, &pStmt, &pTail);
    if (rc == SQLITE_OK) {
        sqlite_util::sqlite_bind_statements(*m_pSaveData, pStmt);
        rc = ::sqlite3_step(pStmt);
    }
    else {
        if ( rc != SQLITE_DONE ) {
            CStdString sSQL = ds_str_conv::ConvertFromUTF8(sSql.c_str());
            m_pErrorHandler->OnError(sSQL.c_str(), _T("CSqLiteRecordsetImpl::CommitInsert()(1)"));
            OnErrorCode(rc, _T("CSqLiteRecordsetImpl::CommitInsert()(1)"));
        }
    }
    ::sqlite3_finalize(pStmt);

    if ( rc != SQLITE_DONE ) {
        CStdString sSQL = ds_str_conv::ConvertFromUTF8(sSql.c_str());
        m_pErrorHandler->OnError(sSQL.c_str(), _T("CSqLiteRecordsetImpl::CommitInsert()(1)"));
        OnErrorCode(rc, _T("CSqLiteRecordsetImpl::CommitInsert()(2)"));
    }

    delete m_pSaveData;
    m_pSaveData = nullptr;
}

void CSqLiteRecordsetImpl::OnErrorCode(int rc, LPCTSTR sFunctionName)
{
    sqlite3 *pDB = m_pDB->GetSqLiteDB();
    const char *localError = ::sqlite3_errmsg(pDB);
    m_pErrorHandler->OnError(rc, localError, sFunctionName);
}

long CSqLiteRecordsetImpl::GetRecordCount()
{
    std::string sSQL = "SELECT COUNT(*) FROM ";
                sSQL += m_sTable.c_str();

    CSqLiteRecordsetImpl loader(m_pDB, m_pErrorHandler);
    if ( !loader.OpenSQLUTF8(sSQL.c_str()) ) {
        return -1;
    }

    if ( !loader.MoveFirstImpl() ) {
        return false;
    }
    
    const long nCount = loader.GetFieldLong(_T("COUNT(*)"));
    return nCount;
}

bool CSqLiteRecordsetImpl::DoesFieldExist(LPCTSTR sFieldName) 
{
    m_pFieldInfoData = m_pDB->GetTableFieldInfoImpl(m_sTable.c_str());
    if ( !m_pFieldInfoData ) {
        return false;
    }

    std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
    if ( m_pFieldInfoData->find(sFieldNameUTF8.c_str()) != m_pFieldInfoData->end() ) {
        return true;
    }
	return false;
}

bool CSqLiteRecordsetImpl::SeekByString(LPCTSTR sIndex, LPCTSTR sValue)
{
    CloseStatement();

    const std::string sIndexUTF8 = ds_str_conv::ConvertToUTF8(sIndex);
    const std::string sValueUTF8 = ds_str_conv::ConvertToUTF8(sValue);
    // TODO:
    // http://zetcode.com/db/sqlitec/ -> could be done optimization -> 
    // do use prepared statements and bind operations
    // do map all prepared statements
    CStdStringA sFind;
	sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s'", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());
    // temporal workaround while mdb2sqlite conversion will be fixed
    //sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s' COLLATE NOCASE", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());

    if ( OpenImpl(sFind.c_str()) ) {
        if ( MoveFirstImpl() ) {
            return true;
        }
    }

    return false;
}

bool CSqLiteRecordsetImpl::SeekByLong(LPCTSTR sIndex, long nValue)
{
    const std::string sIndexUTF8 = ds_str_conv::ConvertToUTF8(sIndex);
    return SeekByLongUTF8(sIndexUTF8.c_str(), nValue);
}

bool CSqLiteRecordsetImpl::SeekByLongUTF8(const char *sIndexUTF8, long nValue)
{
    CloseStatement();

    // TODO:
    // http://zetcode.com/db/sqlitec/ -> could be done optimization -> 
    // do use prepared statements and bind operations
    // do map all prepared statements
    CStdStringA sFind;
	sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%d'", m_sTable.c_str(), sIndexUTF8, nValue);

    if ( OpenImpl(sFind.c_str()) ) {
        if ( MoveFirstImpl() ) {
            return true;
        }
    }

    return false;
}

void CSqLiteRecordsetImpl::SetFieldStringUTF8(const char *sFieldName, const char *sValue)
{
    ASSERT(m_pSaveData);
    if ( strlen(sValue) <= 0 ) {
        // This is default realization, if it's not required please do add compile define
        // DAO: inline COleVariant MakeVariant(LPCTSTR strSrc) does the same
        // It's quite usefull: 
        // e.g. FOREIGN KEY and you can not write empty string.
        // you should do something like:
        // strlen(sValue) == 0 ) SetFieldValueNull(); else SetFieldValue(sValue);
        sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataNull();
       (*m_pSaveData)[sFieldName] = pFieldData;
    }
    else {
        sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataText(sValue);
        (*m_pSaveData)[sFieldName] = pFieldData;
    }
}

std::string CSqLiteRecordsetImpl::GetFieldStringUTF8(const char *sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(ds_str_conv::ConvertFromUTF8(sFieldName).c_str());
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, "CSqLiteRecordsetImpl::GetFieldStringUTF8", m_sTable.c_str());
        return "";
    }
	const char *sValue = (const char *)sqlite3_column_text(m_stmt, nColumnIndex);
    if ( !sValue ) {
        return "";
    }
    return sValue;
}

CStdString CSqLiteRecordsetImpl::GetFieldString(LPCTSTR sFieldName)
{
    //int nRowId = sqlite3_column_int(m_stmt, 0);
    //nRowId;
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldString", m_sTable.c_str());
        return _T("");
    }
	const char *localValue = (const char*)sqlite3_column_text(m_stmt, nColumnIndex);
    return ds_str_conv::ConvertFromUTF8(localValue);
}

void CSqLiteRecordsetImpl::SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue)
{
    SetFieldStringUTF8(ds_str_conv::ConvertToUTF8(sFieldName).c_str(), ds_str_conv::ConvertToUTF8(sValue).c_str());
}

long CSqLiteRecordsetImpl::GetFieldLong(LPCTSTR sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldLong", m_sTable.c_str());
        return 0;
    }
	return sqlite3_column_int(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldLong(LPCTSTR sFieldName, long lValue)
{
    ASSERT(m_pSaveData);
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataLong(lValue);
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

void CSqLiteRecordsetImpl::SetFieldValueNull(LPCTSTR sFieldName)
{
    ASSERT(m_pSaveData);
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataNull;
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

double CSqLiteRecordsetImpl::GetFieldDouble(LPCTSTR sFieldName) 
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldDouble", m_sTable.c_str());
        return 0.;
    }
	return sqlite3_column_double(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDouble(LPCTSTR sFieldName, double dValue)
{
    ASSERT(m_pSaveData);
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDouble(dValue);
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

time_t CSqLiteRecordsetImpl::GetFieldDateTime(LPCTSTR sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldDateTime", m_sTable.c_str());
        return -1;
    }
    return sqlite3_column_int64(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDateTime(LPCTSTR sFieldName, const time_t &time)
{
    ASSERT(m_pSaveData);
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDateTime(time);
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

bool CSqLiteRecordsetImpl::IsFieldValueNull(LPCTSTR sFieldName)
{
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::IsFieldValueNull", m_sTable.c_str());
        return false;
    }
    return sqlite3_column_type(m_stmt, nColumnIndex) == SQLITE_NULL;
}

int CSqLiteRecordsetImpl::FindColumnIndex(LPCTSTR sFieldName)
{
    CStdString sName = sFieldName;
    sName.MakeLower();
    auto found = m_name_to_index.find(sName);
    if ( found == m_name_to_index.end() ) {
        return -1;
    }
    return found->second;
}

bool CSqLiteRecordsetImpl::IsEOF()
{
    return m_bEOF;
}

bool CSqLiteRecordsetImpl::MoveNext() 
{
	const int rc = sqlite3_step(m_stmt);
    switch (rc)
    {
    case SQLITE_DONE:
        {   // no rows 
            m_bEOF = true;
            return true;
        }
        break;
    case SQLITE_ROW:
        {   // one or more rows
            m_bEOF = false;
            return true;
        }
        break;
    }

    CloseStatement();
    const char *localError= sqlite3_errmsg(m_pDB->GetSqLiteDB());
    m_pErrorHandler->OnError(rc, localError, _T("CSqLiteRecordsetImpl::MoveNext()"));

    return false;
}

bool CSqLiteRecordsetImpl::MoveFirst() 
{
	if ( !m_bSQLOpened )//already opened by OpenSQL
	{
		CloseStatement(); // sqlite3_reset would be enough if m_stmt and m_stmt.sql == sSQL

		ASSERT(!m_sTable.empty());
		std::string sSQL  = "SELECT ROWID,* FROM ";
					sSQL += m_sTable.c_str();
		if ( !OpenImpl(sSQL.c_str()) ) {
            return false;
        }
	}
    
    return MoveFirstImpl();
}

bool CSqLiteRecordsetImpl::MoveFirstImpl()
{
    ASSERT(m_stmt);
    const int rc = sqlite3_step(m_stmt);
    switch (rc)
    {
    case SQLITE_DONE:
        {   // no rows 
            m_bEOF = true;
            return false;
        }
        break;
    case SQLITE_ROW:
        {   // one or more rows
            m_bEOF = false;
            return true;
        }
        break;
    }

    CloseStatement();
    const char *localError= sqlite3_errmsg(m_pDB->GetSqLiteDB());
    m_pErrorHandler->OnError(rc, localError, _T("CSqLiteRecordsetImpl::MoveFirstImpl()"));

    return false;
}

bool CSqLiteRecordsetImpl::OpenImpl(const char *sql)
{
	if ( m_stmt ) {
		return true;
	}

    m_stmt = Prepare(m_pDB->GetSqLiteDB(), sql, m_pErrorHandler);

    if ( !m_stmt ) {
        return false;
    }

    ASSERT(m_name_to_index.size() == 0);

    const int nColCnt = sqlite3_column_count(m_stmt);
    for (int nIndex = 0; nIndex < nColCnt; ++nIndex)
    {
        const char *sName = sqlite3_column_name(m_stmt, nIndex);
        CStdString sColName = ds_str_conv::ConvertFromUTF8(sName);
        sColName.MakeLower();
        m_name_to_index[sColName] = nIndex;
    } 

    return true;
}

bool CSqLiteRecordsetImpl::DeleteAllByStringValue(LPCTSTR sField, LPCTSTR sValue)
{
    ASSERT(!m_sTable.empty());

    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = ds_str_conv::ConvertToUTF8(sValue);

    std::string strSQL  = "DELETE FROM ";
                strSQL += m_sTable;
                strSQL += " WHERE ";
                strSQL += sFieldUTF8;
                strSQL += " = '";
                strSQL += sValueUTF8;
                strSQL += "'";

    if ( m_pDB->ExecuteUTF8(strSQL.c_str()) ) {
        return true;
    }

    return false;
}

bool CSqLiteRecordsetImpl::DeleteAllByLongValue(LPCTSTR sField, long nValue)
{
    ASSERT(!m_sTable.empty());

    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = ds_str_conv::long_to_string(nValue);

    std::string strSQL  = "DELETE FROM ";
                strSQL += m_sTable;
                strSQL += " WHERE ";
                strSQL += sFieldUTF8;
                strSQL += " = ";
                strSQL += sValueUTF8;

    if ( m_pDB->ExecuteUTF8(strSQL.c_str()) ) {
        return true;
    }

    return false;
}

void CSqLiteRecordsetImpl::Flush()
{
    ASSERT(!m_sTable.empty());

    std::string strSQL  = "DELETE FROM ";
                strSQL += m_sTable;
    m_pDB->ExecuteUTF8(strSQL.c_str());
}

bool CSqLiteRecordsetImpl::DeleteByLongValue(LPCTSTR sField, long nValue)
{   
    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = ds_str_conv::long_to_string(nValue);

    // https://groups.google.com/forum/#!topic/android-developers/rrmbsKyKRCE
    // Unfortunately SQLite does not support the LIMIT clause in DELETE statements. But I think you could get around with something else:
    // DELETE FROM MyTable WHERE _id IN (SELECT _id FROM MyTable WHERE XYZ ORDER BY Col LIMIT 5);

    std::string sDelete;
    sDelete  = "DELETE FROM ";
    sDelete += m_sTable;
    sDelete += " WHERE RowId IN (SELECT RowId FROM ";
    sDelete += m_sTable;
    sDelete += " WHERE ";
    sDelete += sFieldUTF8;
    sDelete += " = ";
    sDelete += sValueUTF8;
    sDelete += " LIMIT 1)";

    const int nRetVal = m_pDB->ExecuteUTF8(sDelete.c_str());
    if ( nRetVal == -1 ) {
        OnErrorCode(nRetVal, _T("CSqLiteRecordsetImpl::DeleteByLongValue()"));
        return false;
    }
    return true;
}

bool CSqLiteRecordsetImpl::DeleteByStringValue(LPCTSTR sField, LPCTSTR sValue)
{
    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = ds_str_conv::ConvertToUTF8(sValue);

    // https://groups.google.com/forum/#!topic/android-developers/rrmbsKyKRCE
    // Unfortunately SQLite does not support the LIMIT clause in DELETE statements. But I think you could get around with something else:
    // DELETE FROM MyTable WHERE _id IN (SELECT _id FROM MyTable WHERE XYZ ORDER BY Col LIMIT 5);

    std::string sDelete;
    sDelete  = "DELETE FROM ";
    sDelete += m_sTable;
    sDelete += " WHERE RowId IN (SELECT RowId FROM ";
    sDelete += m_sTable;
    sDelete += " WHERE ";
    sDelete += sFieldUTF8;
    sDelete += " = '";
    sDelete += sValueUTF8;
    sDelete += "' LIMIT 1)";

    const int nRetVal = m_pDB->ExecuteUTF8(sDelete.c_str());
    if ( nRetVal == -1 ) {
        OnErrorCode(nRetVal, _T("CSqLiteRecordsetImpl::DeleteByStringValue()"));
        return false;
    }
    return true;
}
