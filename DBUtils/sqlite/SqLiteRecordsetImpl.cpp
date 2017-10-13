#include "stdafx.h"
#include "SqLiteRecordsetImpl.h"

#include "sqlite_copy_table.h"
#include "sqlite_bind_util.h"
#include "sqlite_table_info.h"
#include "sqlite_include.h"

#include "SqLiteDatabaseImpl.h"
#include "SqLiteErrorHandler.h"

#include "mutex"
#include "condition_variable"

#include "../dsStrConv.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
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

/*
** A pointer to an instance of this structure is passed as the user-context
** pointer when registering for an unlock-notify callback.
*/
typedef struct UnlockNotification UnlockNotification;
struct UnlockNotification {
  bool bFired;                    /* True after unlock event has occurred */
  std::condition_variable cond; /* Condition variable to wait on */
  std::mutex mutex;             /* Mutex to protect structure */
};

/*
** This function is an unlock-notify callback registered with SQLite.
*/
static void unlock_notify_cb(void **apArg, int nArg)
{
	for ( int i = 0; i < nArg; i++ )
	{
		UnlockNotification *p = (UnlockNotification *)apArg[i];
		p->mutex.lock();
		p->bFired = true;
		p->cond.notify_all();
		p->mutex.unlock();
	}
}

/*
** This function assumes that an SQLite API call (either sqlite3_prepare_v2() 
** or sqlite3_step()) has just returned SQLITE_LOCKED. The argument is the
** associated database connection.
**
** This function calls sqlite3_unlock_notify() to register for an 
** unlock-notify callback, then blocks until that callback is delivered 
** and returns SQLITE_OK. The caller should then retry the failed operation.
**
** Or, if sqlite3_unlock_notify() indicates that to block would deadlock 
** the system, then this function returns SQLITE_LOCKED immediately. In 
** this case the caller should not retry the operation and should roll 
** back the current transaction (if any).
*/
static int wait_for_unlock_notify(sqlite3 *db)
{
	int rc;
	UnlockNotification un;

	/* Initialize the UnlockNotification structure. */
	un.bFired = false;

	/* Register for an unlock-notify callback. */
	rc = sqlite3_unlock_notify(db, unlock_notify_cb, (void *)&un);
	ASSERT( rc==SQLITE_LOCKED || rc==SQLITE_OK );

	/* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
	** or SQLITE_OK. 
	**
	** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
	** case this function needs to return SQLITE_LOCKED to the caller so 
	** that the current transaction can be rolled back. Otherwise, block
	** until the unlock-notify callback is invoked, then return SQLITE_OK.
	*/
	if( rc==SQLITE_OK ){
		std::unique_lock<std::mutex> lck(un.mutex);
		if( !un.bFired ){
			un.cond.wait(lck);
		}
	}
	else
	{
		//DeadLock
		//To prevent - use "begin exclusive transaction"
	}

	return rc;
}

/*
** This function is a wrapper around the SQLite function sqlite3_step().
** It functions in the same way as step(), except that if a required
** shared-cache lock cannot be obtained, this function may block waiting for
** the lock to become available. In this scenario the normal API step()
** function always returns SQLITE_LOCKED.
**
** If this function returns SQLITE_LOCKED, the caller should rollback
** the current transaction (if any) and try again later. Otherwise, the
** system may become deadlocked.
*/
int sqlite3_blocking_step(sqlite3_stmt *pStmt)
{
	int rc;
	while( SQLITE_LOCKED_SHAREDCACHE==(rc = sqlite3_step(pStmt)) ){
		rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
		if( rc != SQLITE_OK )
			break;
		sqlite3_reset(pStmt);
	}
	return rc;
}

/*
** This function is a wrapper around the SQLite function sqlite3_prepare_v2().
** It functions in the same way as prepare_v2(), except that if a required
** shared-cache lock cannot be obtained, this function may block waiting for
** the lock to become available. In this scenario the normal API prepare_v2()
** function always returns SQLITE_LOCKED.
**
** If this function returns SQLITE_LOCKED, the caller should rollback
** the current transaction (if any) and try again later. Otherwise, the
** system may become deadlocked.
*/
int sqlite3_blocking_prepare_v2(
  sqlite3 *db,              /* Database handle. */
  const char *zSql,         /* UTF-8 encoded SQL statement. */
  int nSql,                 /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,    /* OUT: A pointer to the prepared statement */
  const char **pz           /* OUT: End of parsed string */
)
{
	int rc;
	while( SQLITE_LOCKED_SHAREDCACHE==(rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)) ){
		rc = wait_for_unlock_notify(db);
		if( rc!=SQLITE_OK ) break;
	}

	return rc;
}

static void OnColumnIndexFailed(CSqLiteErrorHandler *pErrorHandler, 
                                const char *sFieldName, 
                                const char *sFunctionName, 
                                const char *sTableName,
                                const char *sDatabasePath) 
{
    std::string sError   = "Unable to find field: ";
                sError += sFieldName;
                sError += ".";
                sError += " Table: ";
                sError += sTableName;
                sError += ".";
                sError += " DB path: ";
                sError += sDatabasePath;
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

bool CSqLiteRecordsetImpl::Open(const wchar_t *sTableName)
{
    if ( m_bSQLOpened ) {
        return true;
    }
	m_sTable = ds_str_conv::ConvertToUTF8(sTableName);
    return true;
}

bool CSqLiteRecordsetImpl::OpenSQL(const wchar_t *sSQL)
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
        m_sTable = sSQL;
        return true;
    }
    return false;
}

bool CSqLiteRecordsetImpl::OpenView(const wchar_t *sViewName)
{
    // NOTE: OpenImpl() is not completely correct for the VIEW
    // VIEW does not contain ROWID
	m_sTable = ds_str_conv::ConvertToUTF8(sViewName);
    return true;
}

void CSqLiteRecordsetImpl::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize)
{
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataBinary(pData, nSize);
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

void CSqLiteRecordsetImpl::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize)
{
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        ::OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldBinary", m_sTable.c_str(), sPathUTF8.c_str());
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
    const int rc = ::sqlite3_blocking_prepare_v2(pDB, sql, -1, &stmt, &tail);

    if (rc != SQLITE_OK)  {
        const char* localError = sqlite3_errmsg(pDB);
        pErrorHandler->OnErrorCode(rc, localError, "Prepare(sqlite3 *pDB....)");
        std::string sError = "SQL statement: ";
                    sError += sql;
        pErrorHandler->OnError(sError.c_str(), "Prepare(sqlite3 *pDB....)");
        ::sqlite3_finalize(stmt);
        return nullptr;
    }

    return stmt;
}

bool CSqLiteRecordsetImpl::Delete()
{
    const int nRowId = ::sqlite3_column_int(m_stmt, 0); // 0 column always holds row id

    const std::string sRowId = std::to_string(nRowId);
    
    std::string sDelete  = "DELETE FROM ";
                sDelete += m_sTable;
                sDelete += " WHERE ROWID = ";
                sDelete += sRowId;
	//sDelete.Format("DELETE FROM %s WHERE ROWID = %d", m_sTable.c_str(), nRowId);
    const int nRetVal = m_pDB->ExecuteUTF8(sDelete.c_str());
    if ( nRetVal == -1 ) {
        OnErrorCode(nRetVal, "CSqLiteRecordsetImpl::Delete()");
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
    
        std::string sSql = "INSERT INTO `";
        sSql += m_sTable.c_str();
        sSql += "` (";
        sSql += sColumns.c_str();
        sSql += ")";
        sSql += " VALUES (";
        sSql += sValues.c_str();
        sSql += ")";
   
        const char* pTail = nullptr;
        rc = ::sqlite3_blocking_prepare_v2(pDB, sSql.c_str(), -1, &m_insert_stmt, &pTail);

        if (rc != SQLITE_OK) 
        {
           OnErrorCode(rc, "CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_prepare_v2)");
           std::string sError = "SQL statement:";
                       sError += sSql.c_str();
           m_pErrorHandler->OnError(sError.c_str(), "CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_prepare_v2)");
           return;
        }
    }

    ASSERT(rc == SQLITE_OK);
    
    sqlite_util::sqlite_bind_statements(*m_pSaveData, m_insert_stmt);
    rc = ::sqlite3_blocking_step(m_insert_stmt);
    if ( rc != SQLITE_DONE ) {
        const char *sql = sqlite3_sql(m_insert_stmt);
        std::string sError = "SQL statement: ";
        sError += sql;
        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_step)");
        OnErrorCode(rc, "CSqLiteRecordsetImpl::DoInsertDefault()(sqlite3_step)");
    }
    ::sqlite3_clear_bindings(m_insert_stmt);
    ::sqlite3_reset(m_insert_stmt);
    
    // multi-threading unsuported
    m_nEditRowId = ::sqlite3_last_insert_rowid(pDB);
}

bool CSqLiteRecordsetImpl::DoUpdate()
{
    //sqlite3_finalize(m_update_stmt);
    //m_update_stmt = nullptr;

    const std::string sValues = sqlite_util::save_data_to_update_values_string(m_pSaveData);
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
        std::string sSql  = "UPDATE `";
                    sSql += m_sTable.c_str();
                    sSql += "` SET ";
                    sSql += sValues.c_str();
                    sSql += " WHERE ROWID = ?";
        rc = ::sqlite3_blocking_prepare_v2(pDB, sSql.c_str(), -1, &m_update_stmt, &pTail);
        if (rc != SQLITE_OK) 
        {
            OnErrorCode(rc, "CSqLiteRecordsetImpl::DoUpdate()(sqlite3_prepare_v2)");
            sqlite3_finalize(m_update_stmt);
            m_update_stmt = nullptr;
        }
    }

    if (m_update_stmt) 
    {
        const int nIndex = sqlite_util::sqlite_bind_statements(*m_pSaveData, m_update_stmt);
        ::sqlite3_bind_int64(m_update_stmt, nIndex, m_nEditRowId);                            
        rc = ::sqlite3_blocking_step(m_update_stmt);

        ::sqlite3_clear_bindings(m_update_stmt);
        ::sqlite3_reset(m_update_stmt);
    }
   
    if ( rc != SQLITE_DONE ) 
    {
        const std::string sErrorValues = sqlite_util::save_data_to_error_values_string(m_pSaveData);
        std::string sError  = "SQL statement: UPDATE ";
                    sError += m_sTable.c_str();
                    sError += " SET ";
                    sError += sErrorValues.c_str();
                    sError += " WHERE ROWID = ";
                    sError += std::to_string(m_nEditRowId);
        m_pErrorHandler->OnErrorCode(rc, sError.c_str(), "CSqLiteRecordsetImpl::DoUpdate()(sqlite3_finalize)");
        return false;
    }

    return true;
}

bool CSqLiteRecordsetImpl::Update()
{
    ASSERT(m_pSaveData);
    ASSERT(m_pDB->m_bTransMode);

    const bool bRetVal = DoUpdate();

    delete m_pSaveData;
    m_pSaveData = nullptr;

    m_nEditRowId = -1;

	return bRetVal;
}

void CSqLiteRecordsetImpl::CommitInsert()
{
    ASSERT(m_pSaveData);
    // TODO: save/update/insert/add_new operations should be extracted from CSqLiteRecordsetImpl
    // CSqLiteRecordsetImpl should delegate only calls for the extracted operations

    const std::string sColumns = sqlite_util::save_data_to_insert_columns_string(m_pSaveData);
    const std::string sValues  = sqlite_util::save_data_to_insert_values_string(m_pSaveData);

    std::string sSql  = "INSERT INTO ";
                sSql += m_sTable;
                sSql += " (";
                sSql += sColumns;
                sSql += ")";
                sSql += " VALUES (";
                sSql += sValues;
                sSql += ")";

    sqlite3 *pDB = m_pDB->GetSqLiteDB();

    const char* pTail = nullptr;
    sqlite3_stmt *pStmt = nullptr;
    int rc = ::sqlite3_blocking_prepare_v2(pDB, sSql.c_str(), -1, &pStmt, &pTail);
    if (rc == SQLITE_OK) {
        sqlite_util::sqlite_bind_statements(*m_pSaveData, pStmt);
        rc = ::sqlite3_blocking_step(pStmt);
    }
    else {
        if ( rc != SQLITE_DONE ) {
            m_pErrorHandler->OnError(sSql.c_str(), "CSqLiteRecordsetImpl::CommitInsert()(1)");
            OnErrorCode(rc, "CSqLiteRecordsetImpl::CommitInsert()(1)");
        }
    }
    ::sqlite3_finalize(pStmt);

    if ( rc != SQLITE_DONE ) {
        m_pErrorHandler->OnError(sSql.c_str(), "CSqLiteRecordsetImpl::CommitInsert()(2)");
        OnErrorCode(rc, "CSqLiteRecordsetImpl::CommitInsert()(2)");
    }

    delete m_pSaveData;
    m_pSaveData = nullptr;
}

void CSqLiteRecordsetImpl::OnErrorCode(int rc, const char *sFunctionName)
{
    sqlite3 *pDB = m_pDB->GetSqLiteDB();
    m_pErrorHandler->OnErrorCode(rc, pDB, sFunctionName);
}

int CSqLiteRecordsetImpl::GetRecordCount() const
{
    std::string sSQL;
    if ( m_bSQLOpened )
    {
        sSQL  = "SELECT COUNT(*) FROM (";
        sSQL += m_sTable.c_str();
        sSQL += ")";
    }
    else
    {
        sSQL  = "SELECT COUNT(*) FROM ";
        sSQL += m_sTable.c_str();
    }

    CSqLiteRecordsetImpl loader(m_pDB, m_pErrorHandler);
    if ( !loader.OpenSQLUTF8(sSQL.c_str()) ) {
        return -1;
    }

    if ( !loader.MoveFirstImpl() ) {
        return false;
    }
    
    const int nCount = loader.GetFieldLong(_T("COUNT(*)"));
    return nCount;
}

bool CSqLiteRecordsetImpl::DoesFieldExist(const wchar_t *sFieldName) 
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

bool CSqLiteRecordsetImpl::SeekByString(const wchar_t *sIndex, const wchar_t *sValue)
{
    CloseStatement();

    const std::string sIndexUTF8 = ds_str_conv::ConvertToUTF8(sIndex);
    const std::string sValueUTF8 = ds_str_conv::ConvertToUTF8(sValue);
    // TODO:
    // http://zetcode.com/db/sqlitec/ -> could be done optimization -> 
    // do use prepared statements and bind operations
    // do map all prepared statements
    std::string sFind  = "SELECT ROWID,* FROM ";
                sFind += m_sTable;
                sFind += " WHERE ";
                sFind += sIndexUTF8;
                sFind += " = '";
                sFind += sValueUTF8;
                sFind += "' COLLATE NOCASE";
	//sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s'", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());
    // NOCASE as "DB BROWSER" does not care about NOCASE attributes inside CREATE statement
    //sFind.Format("SELECT ROWID,* FROM %s WHERE %s = '%s' COLLATE NOCASE", m_sTable.c_str(), sIndexUTF8.c_str(), sValueUTF8.c_str());

    if ( OpenImpl(sFind.c_str()) ) {
        if ( MoveFirstImpl() ) {
            return true;
        }
    }

    return false;
}

bool CSqLiteRecordsetImpl::SeekByLong(const wchar_t *sIndex, int nValue)
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
    std::string sFind = "SELECT ROWID,* FROM `";
                sFind += m_sTable.c_str();
                sFind += "` WHERE ";
                sFind += sIndexUTF8;
                sFind += " = '";
                sFind += std::to_string(nValue);
                sFind += "'";

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
    ASSERT(m_pSaveData->find(sFieldName) == m_pSaveData->end()); // should be called only once for the one sFieldName 

    if ( strlen(sValue) <= 0 ) {
        // This is default realization, if it's not required please do add compile define
        // DAO: inline COleVariant MakeVariant(const wchar_t *strSrc) does the same
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
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        ::OnColumnIndexFailed(m_pErrorHandler, sFieldName, "CSqLiteRecordsetImpl::GetFieldStringUTF8", m_sTable.c_str(), sPathUTF8.c_str());
        return "";
    }
	const char *sValue = (const char *)sqlite3_column_text(m_stmt, nColumnIndex);
    if ( !sValue ) {
        return "";
    }
    return sValue;
}

std::wstring CSqLiteRecordsetImpl::GetFieldString(const wchar_t *sFieldName)
{
    //int nRowId = sqlite3_column_int(m_stmt, 0);
    //nRowId;
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        ::OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldString", m_sTable.c_str(), sPathUTF8.c_str());
        return _T("");
    }
	const char *localValue = (const char*)sqlite3_column_text(m_stmt, nColumnIndex);
    return ds_str_conv::ConvertFromUTF8(localValue);
}

void CSqLiteRecordsetImpl::SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue)
{
    SetFieldStringUTF8(ds_str_conv::ConvertToUTF8(sFieldName).c_str(), ds_str_conv::ConvertToUTF8(sValue).c_str());
}

int64_t CSqLiteRecordsetImpl::GetFieldInt64(const wchar_t *sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldLong", m_sTable.c_str(), sPathUTF8.c_str());
        return 0;
    }
	return sqlite3_column_int64(m_stmt, nColumnIndex);
}

int CSqLiteRecordsetImpl::GetFieldLong(const wchar_t *sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldLong", m_sTable.c_str(), sPathUTF8.c_str());
        return 0;
    }
	return sqlite3_column_int(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldLong(const wchar_t *sFieldName, int lValue)
{
    ASSERT(m_pSaveData);
    const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
    ASSERT(m_pSaveData->find(sFieldNameUTF8) == m_pSaveData->end()); // should be called only once for the one sFieldName 

    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataLong(lValue);
    (*m_pSaveData)[sFieldNameUTF8] = pFieldData;
}

void CSqLiteRecordsetImpl::SetFieldInt64(const wchar_t *sFieldName, int64_t lValue)
{
    ASSERT(m_pSaveData);
    const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
    ASSERT(m_pSaveData->find(sFieldNameUTF8) == m_pSaveData->end()); // should be called only once for the one sFieldName 

    sqlite_util::CFieldDataInt64 *pFieldData = new sqlite_util::CFieldDataInt64(lValue);
    (*m_pSaveData)[sFieldNameUTF8] = pFieldData;
}

void CSqLiteRecordsetImpl::SetFieldValueNull(const wchar_t *sFieldName)
{
    ASSERT(m_pSaveData);
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataNull;
    (*m_pSaveData)[ds_str_conv::ConvertToUTF8(sFieldName)] = pFieldData;
}

double CSqLiteRecordsetImpl::GetFieldDouble(const wchar_t *sFieldName) 
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldDouble", m_sTable.c_str(), sPathUTF8.c_str());
        return 0.;
    }
	return sqlite3_column_double(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDouble(const wchar_t *sFieldName, double dValue)
{
    ASSERT(m_pSaveData);
    const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
    ASSERT(m_pSaveData->find(sFieldNameUTF8.c_str()) == m_pSaveData->end()); // should be called only once for the one sFieldName 
    sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDouble(dValue);
    (*m_pSaveData)[sFieldNameUTF8] = pFieldData;
}

time_t CSqLiteRecordsetImpl::GetFieldDateTime(const wchar_t *sFieldName)
{
    ASSERT(m_stmt);
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::GetFieldDateTime", m_sTable.c_str(), sPathUTF8.c_str());
        return -1;
    }
    return sqlite3_column_int64(m_stmt, nColumnIndex);
}

void CSqLiteRecordsetImpl::SetFieldDateTime(const wchar_t *sFieldName, const time_t &time)
{
    ASSERT(m_pSaveData);
    const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
    ASSERT(m_pSaveData->find(sFieldNameUTF8) == m_pSaveData->end()); // should be called only once for the one sFieldName 
	sqlite_util::CFieldData *pFieldData = new sqlite_util::CFieldDataDateTime(time);
    (*m_pSaveData)[sFieldNameUTF8] = pFieldData;
}

bool CSqLiteRecordsetImpl::IsFieldValueNull(const wchar_t *sFieldName)
{
	// The sqlite3_stricmp() and sqlite3_strnicmp() APIs allow applications and extensions to compare the contents of two buffers containing UTF-8 strings in a case-independent fashion, using the same definition of "case independence" that SQLite uses internally when comparing identifiers. 
    const int nColumnIndex = FindColumnIndex(sFieldName);
    if ( nColumnIndex == -1 ) {
        const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(m_pDB->GetName().c_str());
        OnColumnIndexFailed(m_pErrorHandler, sFieldNameUTF8.c_str(), "CSqLiteRecordsetImpl::IsFieldValueNull", m_sTable.c_str(), sPathUTF8.c_str());
        return false;
    }
    return sqlite3_column_type(m_stmt, nColumnIndex) == SQLITE_NULL;
}

int CSqLiteRecordsetImpl::FindColumnIndex(const wchar_t *sFieldName)
{
    std::wstring sName = sFieldName;
    ds_str_conv::MakeLower(sName);
    
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
	const int rc = sqlite3_blocking_step(m_stmt);
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
    OnErrorCode(rc, "CSqLiteRecordsetImpl::MoveNext()");

    return false;
}

bool CSqLiteRecordsetImpl::MoveFirst() 
{
	if ( !m_bSQLOpened )//already opened by OpenSQL
	{
		CloseStatement(); // sqlite3_reset would be enough if m_stmt and m_stmt.sql == sSQL

		ASSERT(!m_sTable.empty());
		std::string sSQL  = "SELECT ROWID,* FROM ";
					sSQL += m_sTable;
		if ( !OpenImpl(sSQL.c_str()) ) {
            return false;
        }
	}
    
    return MoveFirstImpl();
}

bool CSqLiteRecordsetImpl::MoveFirstImpl()
{
	ASSERT(m_stmt);
    const int rc = sqlite3_blocking_step(m_stmt);
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
    OnErrorCode(rc, "CSqLiteRecordsetImpl::MoveFirstImpl()");

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
        std::wstring sColName = ds_str_conv::ConvertFromUTF8(sName);
        ds_str_conv::MakeLower(sColName);
        m_name_to_index[sColName] = nIndex;
    } 

    return true;
}

bool CSqLiteRecordsetImpl::DeleteAllByStringValue(const wchar_t *sField, const wchar_t *sValue)
{
    ASSERT(!m_sTable.empty());

    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = ds_str_conv::ConvertToUTF8(sValue);

    std::string sSQL  = "DELETE FROM ";
                sSQL += m_sTable;
                sSQL += " WHERE ";
                sSQL += sFieldUTF8;
                sSQL += " = '";
                sSQL += sValueUTF8;
                sSQL += "'";

    if ( m_pDB->ExecuteUTF8(sSQL.c_str()) ) {
        return true;
    }

    return false;
}

bool CSqLiteRecordsetImpl::DeleteAllByLongValue(const wchar_t *sField, int nValue)
{
    ASSERT(!m_sTable.empty());

    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = std::to_string(nValue);

    std::string sSQL  = "DELETE FROM ";
                sSQL += m_sTable;
                sSQL += " WHERE ";
                sSQL += sFieldUTF8;
                sSQL += " = ";
                sSQL += sValueUTF8;

    if ( m_pDB->ExecuteUTF8(sSQL.c_str()) ) {
        return true;
    }

    return false;
}

void CSqLiteRecordsetImpl::Flush()
{
    ASSERT(!m_sTable.empty());

    std::string sSQL  = "DELETE FROM ";
                sSQL += m_sTable;
    m_pDB->ExecuteUTF8(sSQL.c_str());
}

bool CSqLiteRecordsetImpl::DeleteByLongValue(const wchar_t *sField, int nValue)
{   
    const std::string sFieldUTF8 = ds_str_conv::ConvertToUTF8(sField);
    const std::string sValueUTF8 = std::to_string(nValue);

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
        OnErrorCode(nRetVal, "CSqLiteRecordsetImpl::DeleteByLongValue()");
        return false;
    }
    return true;
}

bool CSqLiteRecordsetImpl::DeleteByStringValue(const wchar_t *sField, const wchar_t *sValue)
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
        OnErrorCode(nRetVal, "CSqLiteRecordsetImpl::DeleteByStringValue()");
        return false;
    }
    return true;
}

int CSqLiteRecordsetImpl::GetColumnCount() const
{
    ASSERT(m_stmt);
    return sqlite3_column_count(m_stmt);
}

std::wstring CSqLiteRecordsetImpl::GetColumnName(int nCol) const
{
    ASSERT(m_stmt);
    const std::string sColName = sqlite3_column_name(m_stmt, nCol);
    // we do out own conversion: (sqlite3_column_name16 unused)
    return ds_str_conv::ConvertFromUTF8(sColName.c_str());
}

dsFieldType CSqLiteRecordsetImpl::GetColumnType(int nCol) const
{
    // https://sqlite.org/c3ref/column_blob.html
    // The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. 
    // The value returned by sqlite3_column_type() is only meaningful if no type conversions have occurred as described below. 
    // After a type conversion, the value returned by sqlite3_column_type() is undefined. 
    // Future versions of SQLite may change the behavior of sqlite3_column_type() following a type conversion.

    ASSERT(m_stmt);
    int nType = sqlite3_column_type(m_stmt, nCol);
    switch (nType)
    {
    case SQLITE_INTEGER:
        return dsFieldType_Integer;
        break;
    case SQLITE_FLOAT:
        return dsFieldType_Double;
        break;
    case SQLITE_TEXT:
        return dsFieldType_Text;
        break;
    case SQLITE_BLOB:
        return dsFieldType_Blob;
        break;
    };

    return dsFieldType_Undefined;
}