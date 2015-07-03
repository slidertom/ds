#include "StdAfx.h"
#include "SqLiteRecordsetImpl.h"

#include "SqLiteDatabaseImpl.h"
#include "SqLiteErrorHandler.h"
#include "SqLiteUtil.h"

#include "sqlite3.h"

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
//PRAGMA table_info(LocksHingesCombi);
/*
  int    PrepareSql(sqlite3 *db, sqlite3_stmt  **hs, char *SqlStr, int handle)
   {
       int        n, rc;

       n = 0;

       do
       {
           rc = sqlite3_prepare_v2(db, SqlStr, -1, hs, 0);

           if( (rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED) )
           {
               n++;
               Sleep(SQLTM_TIME);
           }
       }while( (n < SQLTM_COUNT) && ((rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED)));

       if( rc != SQLITE_OK)
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


static void OnColumnIndexFailed(CSqLiteErrorHandler *pErrorHandler, LPCTSTR sFieldName, LPCTSTR sFunctionName) 
{
    CStdString sError;
    sError.Format(_T("Unable to find field: %s."), sFieldName);
    pErrorHandler->OnError(sError.c_str(), sFunctionName);
}

CSqLiteRecordsetImpl::CSqLiteRecordsetImpl(CSqLiteDatabaseImpl *pDatabase, CSqLiteErrorHandler *pErrorHandler)
: m_pDB(pDatabase), m_pErrorHandler(pErrorHandler), m_bEOF(true), m_stmt(nullptr), m_nEditRowId(-1), 
  m_pSaveData(nullptr), m_pInfoData(nullptr), m_insert_stmt(nullptr)
{
    
}

CSqLiteRecordsetImpl::~CSqLiteRecordsetImpl() 
{ 
    delete m_pSaveData;
    delete m_pInfoData;
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
    m_sTable = sqlite_conv::ConvertToUTF8(sTableName);
    return true;
}

bool CSqLiteRecordsetImpl::OpenSQL(LPCTSTR sSQL)
{
    const std::string sSQLUTF8 = sqlite_conv::ConvertToUTF8(sSQL);
    return OpenSQLUTF8(sSQLUTF8.c_str());
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
	m_sTable = sqlite_conv::ConvertToUTF8(sViewName);
    return true;
}

void CSqLiteRecordsetImpl::SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)
{
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataBinary(pData, nSize);
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

void CSqLiteRecordsetImpl::GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize)
{
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldBinary"));
        return;
    }
	nSize = ::sqlite3_column_bytes(m_stmt, nColumnIndex);
    *pData = (unsigned char *)::sqlite3_column_blob(m_stmt, nColumnIndex);
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
        sError += sqlite_conv::ConvertFromUTF8(sql);
        pErrorHandler->OnError(sError.c_str(), _T("Prepare(sqlite3 *pDB....)"));
        ::sqlite3_finalize(stmt);
        return nullptr;
    }

    return stmt;
}

bool CSqLiteRecordsetImpl::Delete()
{
    const int nRowId = ::sqlite3_column_int(m_stmt, 0); // 0 column always holds row id

    CStdStringA sDelete;
	sDelete.Format("DELETE FROM %s WHERE ROWID = %d", m_sTable.c_str(), nRowId);

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    const char *pTail   = nullptr;
    sqlite3_stmt *pStmt = nullptr;
    int rc = ::sqlite3_prepare_v2(pDB, sDelete.c_str(), -1, &pStmt, &pTail);
    if (rc == SQLITE_OK) {
        rc = ::sqlite3_step(pStmt);            
    }
    else {
        OnErrorCode(rc, _T("CSqLiteRecordsetImpl::Delete()"));
    }
    ::sqlite3_finalize(pStmt);

    return rc == SQLITE_ROW || rc == SQLITE_DONE;
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
        if(f->primaryKey()) 
        {
            fields << f->name();

            if(!pk_value.isNull())
            {
                vals << pk_value;
            } else {
                if(f->notnull())
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

            if(f->isInteger())
                vals << "0";
            else
                vals << "''";
        } 
        else 
        {
            // don't insert into fields with a default value
            // or we will never see it.
            if(f->defaultValue().length() == 0)
            {
                fields << f->name();
                vals << "NULL";
            }
        }
    }

    if(!fields.empty())
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
void CSqLiteRecordsetImpl::AddNew()
{
    ASSERT(!m_pSaveData);
    //ASSERT(m_stmt); // open should be called
    ASSERT(m_nEditRowId == -1);
    m_pSaveData = new sqlite_util::CFieldDataMap;
  
    DoInsert(); // do insert default record if possible
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

void CSqLiteRecordsetImpl::DoInsert()
{
    ASSERT(m_sTable.length() > 0);

    if ( !m_pInfoData ) {
        m_pInfoData = new sqlite_util::CFieldInfoMap; // could be a cache on the database level
        sqlite_util::GetTableFieldsdInfo(m_pDB, m_sTable.c_str(), m_pErrorHandler, *m_pInfoData);
    }

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    int rc = SQLITE_OK;
    if ( !m_insert_stmt )
    {
        std::string sColumns;
        std::string sValues;

        // do generate empty statement like: DBBrowserDB::emptyInsertStmt
        auto beg_it = m_pInfoData->begin();
        auto end_it = m_pInfoData->end();
        for (auto it = beg_it; it != end_it; ++it) 
        {
            if ( it->second.m_bPrimary ) 
            {
                //  primary key value as paramater unsupported
                //fields << f->name();
                //if ( !pk_value.isNull() ) {
                //    vals << pk_value;
                //} else {
                if ( it->second.m_bNotNull )
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
           OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoInsert()(1)"));
           CStdString sError = _T("SQL statement:");
           sError += sqlite_conv::ConvertFromUTF8(sSql.c_str()).c_str();
           m_pErrorHandler->OnError(sError.c_str(), _T("CSqLiteRecordsetImpl::DoInsert()(1)"));
           return;
        }
    }

    ASSERT(rc == SQLITE_OK);
    
    sqlite_util::BindStatements(*m_pSaveData, m_insert_stmt);
    rc = ::sqlite3_step(m_insert_stmt);
    ::sqlite3_clear_bindings(m_insert_stmt);
    ::sqlite3_reset(m_insert_stmt);
    
    if ( rc != SQLITE_DONE ) {
        OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoInsert()(2)"));
    }

    // multi-threading unsuported
    m_nEditRowId = ::sqlite3_last_insert_rowid(pDB);
}

void CSqLiteRecordsetImpl::DoUpdate()
{
    ASSERT(m_pSaveData);
    std::string sValues;
    auto end_it = m_pSaveData->end();
    auto beg_it = m_pSaveData->begin();
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

    ASSERT(m_nEditRowId > 0);
    CStdStringA sSql;
    sSql.Format("UPDATE %s SET %s WHERE ROWID = %d", m_sTable.c_str(), sValues.c_str(), m_nEditRowId); 

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    const char* pTail = nullptr;
    sqlite3_stmt *pStmt = nullptr;
    int rc = ::sqlite3_prepare_v2(pDB, sSql.c_str(), -1, &pStmt, &pTail);
    if (rc == SQLITE_OK) 
    {
        sqlite_util::BindStatements(*m_pSaveData, pStmt);
        rc = ::sqlite3_step(pStmt);
    }
    else {
        if ( rc != SQLITE_DONE ) {
            OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoUpdate()(1)"));
        }
    }
    ::sqlite3_finalize(pStmt);

    if ( rc != SQLITE_DONE ) {
        OnErrorCode(rc, _T("CSqLiteRecordsetImpl::DoUpdate()(2)"));
    }
}

bool CSqLiteRecordsetImpl::Update()
{
    ASSERT(m_pSaveData);

    DoUpdate();

    delete m_pSaveData;
    m_pSaveData = nullptr;

    m_nEditRowId = -1;

	return true;
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

    if ( !loader.MoveFirst() ) {
        return false;
    }
    
    const long nCount = loader.GetFieldLong(_T("COUNT(*)"));
    return nCount;
}

bool CSqLiteRecordsetImpl::DoesFieldExist(LPCTSTR sFieldName) 
{
    sqlite_util::CFieldInfoMap field_info_map;
    if ( !sqlite_util::GetTableFieldsdInfo(m_pDB, m_sTable.c_str(), m_pErrorHandler, field_info_map) )
    {
        return false;
    }

    std::string sFieldNameUTF8 = sqlite_conv::ConvertToUTF8(sFieldName);
    if ( field_info_map.find(sFieldNameUTF8.c_str()) != field_info_map.end() ) {
        return true;
    }
	return false;
}

bool CSqLiteRecordsetImpl::SeekByString(LPCTSTR sIndex, LPCTSTR sValue)
{
    CloseStatement();

    const std::string sIndexUTF8 = sqlite_conv::ConvertToUTF8(sIndex);
    const std::string sValueUTF8 = sqlite_conv::ConvertToUTF8(sValue);
    // TODO:
    // http://zetcode.com/db/sqlitec/ -> could be done optimization -> 
    // do use prepared statements and bind operations
    // do map all prepared statements
    CStdStringA sFind;
	sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s'", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());
    // temporal workaround while mdb2sqlite conversion will be fixed
    //sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s' COLLATE NOCASE", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());

    if ( OpenImpl(sFind.c_str()) ) {
        if ( MoveFirst() ) {
            return true;
        }
    }

    return false;
}

bool CSqLiteRecordsetImpl::SeekByLong(LPCTSTR sIndex, long nValue)
{
    const std::string sIndexUTF8 = sqlite_conv::ConvertToUTF8(sIndex);
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
        if ( MoveFirst() ) {
            return true;
        }
    }

    return false;
}

std::string CSqLiteRecordsetImpl::GetFieldStringUTF8(LPCTSTR sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldStringUTF8"));
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
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldString"));
        return _T("");
    }
	const char *localValue = (const char*)sqlite3_column_text(m_stmt, nColumnIndex);
    return sqlite_conv::ConvertFromUTF8(localValue);
}

void CSqLiteRecordsetImpl::SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue)
{
    ASSERT(m_pSaveData);
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataText(sqlite_conv::ConvertToUTF8(sValue).c_str());
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

long CSqLiteRecordsetImpl::GetFieldLong(LPCTSTR sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldLong"));
        return 0;
    }
	return sqlite3_column_int(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldLong(LPCTSTR sFieldName, long lValue)
{
    ASSERT(m_pSaveData);
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataLong(lValue);
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

void CSqLiteRecordsetImpl::SetFieldValueNull(LPCTSTR sFieldName)
{
    ASSERT(m_pSaveData);
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataNull;
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

double CSqLiteRecordsetImpl::GetFieldDouble(LPCTSTR sFieldName) 
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldDouble"));
        return 0.;
    }
	return sqlite3_column_double(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDouble(LPCTSTR sFieldName, double dValue)
{
    ASSERT(m_pSaveData);
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDouble(dValue);
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

time_t CSqLiteRecordsetImpl::GetFieldDateTime(LPCTSTR sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::GetFieldDateTime"));
        return -1;
    }
    return sqlite3_column_int64(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDateTime(LPCTSTR sFieldName, const time_t &time)
{
    ASSERT(m_pSaveData);
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDateTime(time);
    (*m_pSaveData)[sqlite_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

bool CSqLiteRecordsetImpl::IsFieldValueNull(LPCTSTR sFieldName)
{
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        OnColumnIndexFailed(m_pErrorHandler, sFieldName, _T("CSqLiteRecordsetImpl::IsFieldValueNull"));
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

void CSqLiteRecordsetImpl::OpenImpl()
{
    if ( m_stmt ) {
        return; // otherwise: ASSERT(!m_sTable.empty()); in case of the direct OpenSQLUTF8 call.
    }

    ASSERT(!m_sTable.empty());
    std::string sSQL  = "SELECT ROWID,* FROM ";
                sSQL += m_sTable.c_str();
    OpenImpl(sSQL.c_str());
}

bool CSqLiteRecordsetImpl::MoveFirst() 
{
	OpenImpl(); // we do not open on Open function

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
    m_pErrorHandler->OnError(rc, localError, _T("CSqLiteRecordsetImpl::MoveFirst()"));

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
        CStdString sColName = sqlite_conv::ConvertFromUTF8(sName);
        sColName.MakeLower();
        m_name_to_index[sColName] = nIndex;
    } 

    return true;
}

void CSqLiteRecordsetImpl::Flush()
{
    std::string strSQL  = "DELETE FROM ";
                strSQL += m_sTable;
    m_pDB->ExecuteUTF8(strSQL.c_str());
}