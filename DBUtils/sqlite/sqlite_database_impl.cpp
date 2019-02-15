#include "stdafx.h"
#include "sqlite_database_impl.h"

#include "sqlite_bind_util.h"
#include "sqlite_copy_table.h"
#include "sqlite_table_info.h"

#include "SqLiteErrorHandler.h"
#include "SqLiteRecordsetImpl.h"

#include "sqlite_include.h"

#include "../dsStrConv.h"
#include "../dsOpenParams.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#define SQL_BUSY_TIMEOUT  5000

CSqLiteDatabaseImpl::CSqLiteDatabaseImpl() 
{
    m_pErrorHandler = new CSqLiteErrorHandler;
}

CSqLiteDatabaseImpl::~CSqLiteDatabaseImpl()
{
    Close();
    delete m_pErrorHandler;
}

bool CSqLiteDatabaseImpl::IsSqLiteDB(const wchar_t *sPath)
{
    FILE *pFile = ::_wfopen(sPath, L"rb");
    if ( !pFile ) {
        return false;
    }

    bool bSqLite = false;
    char buffer[6];
    size_t nRead = ::fread(buffer, 1, 6, pFile);
    if ( nRead > 5 && 
         buffer[0] == 'S' && 
         buffer[1] == 'Q' && 
         buffer[2] == 'L' && 
         buffer[3] == 'i' && 
         buffer[4] == 't' && 
         buffer[5] == 'e' ) 
    {
        bSqLite = true;
    }

    ::fclose(pFile);

    return bSqLite;
}

bool CSqLiteDatabaseImpl::CompactDatabase()
{
    ExecuteUTF8("vacuum");
    return true;
}

bool CSqLiteDatabaseImpl::BeginTrans() 
{
    ASSERT(!m_bTransMode); // nested transactions are not supported 
    m_bTransMode = true;
    // m_bMultiUser controls if there can be database usage by multiple processes at one time
    if ( m_bMultiUser ) {
        VERIFY(ExecuteUTF8("begin immediate transaction"));
    }
    else {
        VERIFY(ExecuteUTF8("begin transaction"));
    }

    //VERIFY(ExecuteUTF8("begin exclusive transaction"));
    return true;
}

bool CSqLiteDatabaseImpl::CommitTrans() 
{
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
    VERIFY(ExecuteUTF8("commit transaction"));
    return true;
}

bool CSqLiteDatabaseImpl::Rollback()   
{
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
    ExecuteUTF8("rollback transaction");
    return true;
}

bool CSqLiteDatabaseImpl::Execute(const wchar_t *lpszSQL) 
{
    const std::string sql = ds_str_conv::ConvertToUTF8(lpszSQL);
    return ExecuteUTF8(sql.c_str());
}

void CSqLiteDatabaseImpl::Close() 
{
    if ( m_pDB ) {
        ::sqlite3_close(m_pDB);
    }
    m_pDB = nullptr;
    m_sFilePath = L"";

    for (auto &elem :m_table_field_info_map) {
        delete elem.second;
    }
    m_table_field_info_map.clear();
}

// Exec an sql statement in values[0] against
// the database in pData.
int process_ddl_row(void * pData, int nColumns, 
                    char **values, char **columns)
{
    if (nColumns != 1) {
        return 1; // Error
    }

    sqlite3 *db = (sqlite3*)pData;
    ::sqlite3_exec(db, values[0], NULL, NULL, NULL);

    return 0;
}

// Insert from a table named by backup.{values[0]}
// into main.{values[0]} in database pData.
int process_dml_row(void *pData, int nColumns, 
                    char **values, char **columns)
{
    if (nColumns != 1) {
        return 1; // Error
    }
        
    sqlite3* db = (sqlite3*)pData;

    char *stmt = sqlite3_mprintf("insert into main.%q "
                                    "select * from backup.%q", values[0], values[0]);
    sqlite3_exec(db, stmt, NULL, NULL, NULL);
    sqlite3_free(stmt);     

    return 0;
}

bool CSqLiteDatabaseImpl::OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) 
{
    m_bMemory    = open_params.m_bMemory;
    m_bMultiUser = open_params.m_bMultiUser;
    if ( open_params.m_bExclusive ) {
        m_bMultiUser = false;
    }
    // int nRetVal = sqlite3_config(SQLITE_CONFIG_SERIALIZED); // SQLITE_CONFIG_SERIALIZED SQLITE_CONFIG_MULTITHREAD open -> SQLITE_OPEN_FULLMUTEX
    // VERIFY(nRetVal == SQLITE_OK);
    // nRetVal;
    ASSERT(!m_pDB);
    // SQLite support three different threading modes:
    //      Single-thread. In this mode, all mutexes are disabled and SQLite is unsafe to use in more than a single thread at once.
    //      Multi-thread. In this mode, SQLite can be safely used by multiple threads provided that no single database connection is used simultaneously in two or more threads.
    //      Serialized. In serialized mode, SQLite can be safely used by multiple threads with no restriction.
    // The threading mode can be selected at compile-time (when the SQLite library is being compiled from source code) 
    // or at start-time (when the application that intends to use SQLite is initializing) or at run-time (when a new SQLite database connection is being created). 
    // Generally speaking, run-time overrides start-time and start-time overrides compile-time. Except, single-thread mode cannot be overridden once selected.
    // The default mode is serialized. 

    // http://www.mimec.org/node/297
    // database.exec("PRAGMA encoding = \"UTF-16\"");

    // For shared database usage. Also for multithreaded database usage.
    // participation in cache system is controled later via SQLITE_OPEN_SHAREDCACHE and SQLITE_OPEN_PRIVATECACHE flags.
    sqlite3_enable_shared_cache(1);

    // http://manski.net/2012/10/sqlite-performance/
    // Read:
    // * using a read-only connection doesn’t provide any performance benefit
    // * using a shared cache is never faster (but sometimes slower) than using a private cache
    // * using WAL is always faster than using the default journal mode (DELETE) 
    // Write:
    // * Using a shared cache doesn’t affect the performance.
    // * Using WAL improves write performance significantly.
    // * Using a shared connection is always faster than using multiple connections. 
    // WAL - http://www.sqlite.org/draft/wal.html
    // All processes using a database must be on the same host computer; WAL does not work over a network filesystem. 
    // http://utf8everywhere.org/
    m_sFilePath = sPath;
    // UTF8 path required
    const std::string localFileName = ds_str_conv::ConvertToUTF8(sPath);
    int nFlags = open_params.m_bReadOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
    
    // For shared database usage. Also for multithreaded database usage.
    // Serialized mode: Flag SQLITE OPEN FULLMUTEX sets serialized mode
    // In this mode no restriction is being put and Sqlite can be safely used for performing multiple threads. 
    // The default mode enabled in Sqlite is serialized.
    if ( m_bMultiUser ) {
        nFlags |= SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_FULLMUTEX;
    }
    else {
        nFlags |= SQLITE_OPEN_PRIVATECACHE;
    }

    int rc = sqlite3_open_v2(localFileName.c_str(), &m_pDB, nFlags, NULL);
    // sqlite3_open16 - UTF-16 does not allow to open in the read only mode
    // int rc = sqlite3_open16(sPath, &m_pDB);
    if (rc != SQLITE_OK)
    {
        const char *localError = "Out of memory";

        if ( m_pDB ) {
            localError = sqlite3_errmsg(m_pDB);
            Close();
        }
        m_pErrorHandler->OnErrorCode(rc, localError, "CSqLiteDatabaseImpl::OpenDB");
        return false;
    }

    rc = sqlite3_extended_result_codes(m_pDB, 1);
    if (rc != SQLITE_OK) {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::OpenDB");
        Close();
        return false;
    }

    rc = sqlite3_busy_timeout(m_pDB, SQL_BUSY_TIMEOUT);
    if (rc != SQLITE_OK) {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::OpenDB::sqlite3_busy_timeout");
        Close();
        return false;
    }
    
    //if ( m_bReadOnly ) {
    //    m_bMemory = true; 
    //}
    
    //m_bMemory = true; // 12 -> 2.5 s.
    // if slow network disk -> m_bMemory open can increase read performance x10
    
    if ( m_bMemory ) 
    {   // https://sqlite.org/backup.html
        sqlite3 *memorydb = nullptr;
        sqlite3_open(":memory:", &memorydb);

        sqlite3_backup *pBackup = sqlite3_backup_init(memorydb, "main", m_pDB, "main");
        if (pBackup) {
            ::sqlite3_backup_step(pBackup, -1);
            ::sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(memorydb);
        if (rc != SQLITE_OK) {
            m_pErrorHandler->OnErrorCode(rc, memorydb, "CSqLiteDatabaseImpl::OpenDB::sqlite3_open :memory");    
        }
        ::sqlite3_close(m_pDB);
        m_pDB = memorydb;

        // #Try to avoid hitting disk, trading safety for speed.
        // #http://stackoverflow.com/questions/304393
        ExecuteUTF8("PRAGMA temp_store=MEMORY;");
        ExecuteUTF8("PRAGMA journal_mode=MEMORY;");
    }

    // http://www.sqlite.org/foreignkeys.html#fk_enable
    // Foreign key constraints are disabled by default (for backwards compatibility), 
    // so must be enabled separately for each database connection. (Note, however, that future releases of SQLite might change so that foreign key 
    // constraints enabled by default. 
    ExecuteUTF8("PRAGMA foreign_keys = ON");
    ExecuteUTF8("PRAGMA cache_size = 20000"); // default 2000
    ExecuteUTF8("PRAGMA page_size = 65535");
    if ( open_params.m_bExclusive ) {
        ExecuteUTF8("PRAGMA locking_mode = EXCLUSIVE"); // 19 -> 13 the most sensitive for the network file
    }
    if ( m_bReadOnly )  {
        ExecuteUTF8("PRAGMA synchronous = OFF");    // network: 25 -> 19
        ExecuteUTF8("PRAGMA temp_store=MEMORY;");   // 13 -> 12
        ExecuteUTF8("PRAGMA journal_mode=MEMORY;"); // 13 -> 12
    }
    //ExecuteUTF8("PRAGMA main.journal_mode = OFF");
    // https://www.whoishostingthis.com/compare/sqlite/optimize/
    // If you’re already living dangerously with PRAGMA synchronous = OFF, 
    // and you’re trying to squeeze out all the extra milliseconds, you can also store the rollback 
    // journal in memory instead of saving it to disk. Combined with the previous optimization, this is a bit risky.
    //ExecuteUTF8("PRAGMA journal_mode = MEMORY");
    //ExecuteUTF8("PRAGMA busy_timeout=1000");
    
    return true;
}

dsDBType CSqLiteDatabaseImpl::GetType()
{
    return dsType_SqLite;
}

bool CSqLiteDatabaseImpl::IsReadOnly() const
{
    return m_bReadOnly; 
}

bool CSqLiteDatabaseImpl::IsOpen() const
{
    return (m_pDB) ? true : false;
}

std::wstring CSqLiteDatabaseImpl::GetName()
{
    return m_sFilePath;
}

bool CSqLiteDatabaseImpl::DoesTableExistUTF8(const char *sTable)
{
     std::string sSQL  = "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = '";
                sSQL += sTable;
                sSQL += "' COLLATE NOCASE";
    
    CSqLiteRecordsetImpl loader(this, m_pErrorHandler);
    if ( !loader.OpenSQLUTF8(sSQL.c_str()) ) {
        return false;
    }

    if ( !loader.MoveFirstImpl() ) {
        return false;
    }
    
    return true;
}

bool CSqLiteDatabaseImpl::DoesTableExist(const wchar_t *sTable)
{
    std::string sTableUTF8 = ds_str_conv::ConvertToUTF8(sTable);
    return DoesTableExistUTF8(sTableUTF8.c_str());
}

CAbsRecordset *CSqLiteDatabaseImpl::CreateRecordset()
{
    return new CSqLiteRecordsetImpl(this, m_pErrorHandler);
}

CSqLiteDatabaseImpl::dbErrorHandler CSqLiteDatabaseImpl::SetErrorHandler(CSqLiteDatabaseImpl::dbErrorHandler newHandler)
{
    ASSERT(m_pErrorHandler);
    return m_pErrorHandler->SetErrorHandler(newHandler);
}

void CSqLiteDatabaseImpl::DeleteRelation(const wchar_t *sRelation)
{
    //ASSERT(FALSE);
    //sqlite3_exec
}

bool CSqLiteDatabaseImpl::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                         const wchar_t *sField, const wchar_t *sForeignField)
{
    //ASSERT(FALSE);
    //Impl must be simillar to
    //sCreateRelSQL.Format(L"ALTER TABLE %s ADD CONSTRAINT %s FOREIGN KEY (%s) REFERENCES %s(%s) ON DELETE CASCADE", sTable1, sRelName, sField1, sTable2, sField2);
    //pDB->ExecuteSQL(sCreateRelSQL.c_str());
    return true;
}

#define SQLTM_COUNT 100 // -> SQLTM_COUNT*SQLTM_TIME = ms timeout 
#define SQLTM_TIME  50

bool CSqLiteDatabaseImpl::ExecuteUTF8(const char *sqlUTF8)
{
    char *localError = 0;
    int rc(0);
    int nRepeatCount(0);

    do
    {
        rc = sqlite3_exec(m_pDB, sqlUTF8, 0, 0, &localError);
        if ( rc == SQLITE_LOCKED || rc == SQLITE_LOCKED_SHAREDCACHE || rc == SQLITE_BUSY)
        {
            ++nRepeatCount;
            Sleep(SQLTM_TIME);
        }
        
    } while ( (nRepeatCount < SQLTM_COUNT) && ( rc == SQLITE_BUSY || rc == SQLITE_LOCKED || rc == SQLITE_LOCKED_SHAREDCACHE ) );

    if (rc == SQLITE_OK) {
        return true;
    }
    else
    {
        m_pErrorHandler->OnErrorCode(rc, localError, "CSqLiteDatabaseImpl::ExecuteUTF8");
        const std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(GetName().c_str());
        std::string sFuncDescr = " DB path: ";
                    sFuncDescr += sPathUTF8;
                    sFuncDescr += ".";
                    sFuncDescr += " CSqLiteDatabaseImpl::ExecuteUTF8";
        m_pErrorHandler->OnError(sqlUTF8, sFuncDescr.c_str());
        sqlite3_free(localError);
    }

    return false;
}

const sqlite_util::CFieldInfoMap *CSqLiteDatabaseImpl::GetTableFieldInfoImpl(const char *sTableNameUTF8)
{
    auto found = m_table_field_info_map.find(sTableNameUTF8);
    if ( found == m_table_field_info_map.end() )
    {
        sqlite_util::CFieldInfoMap *pFieldInfoMap = new sqlite_util::CFieldInfoMap;
        if ( !sqlite_util::sqlite_get_table_fields_info(this, sTableNameUTF8, m_pErrorHandler, *pFieldInfoMap) ) {
            delete pFieldInfoMap;
            return nullptr;
        }

        m_table_field_info_map[sTableNameUTF8] = pFieldInfoMap;
        return pFieldInfoMap;
    }
    
    return found->second;
}

void CSqLiteDatabaseImpl::OnError(const char *sError, const char *sFunctionName)
{
    m_pErrorHandler->OnError(sError, sFunctionName);
}

bool CSqLiteDatabaseImpl::GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info)
{
    std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTable);
    const sqlite_util::CFieldInfoMap *pFieldInfoMap = GetTableFieldInfoImpl(sTableNameUTF8.c_str());
    if ( !pFieldInfoMap ) {
        std::string sError = "GetTableFieldInfo failed. Table ";
                    sError += sTableNameUTF8;
        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::GetTableFieldInfo");
        return false;
    }

    auto end_it = pFieldInfoMap->end();
    for (auto it = pFieldInfoMap->begin(); it != end_it; ++it) 
    {
        dsFieldType field_type = it->second.GetFieldType();
        const std::wstring sColName = ds_str_conv::ConvertFromUTF8(it->first.c_str());
        info[sColName] = field_type;
    }

    return true;
}
// https://sqlite.org/backup.html
// This function is used to load the contents of a database file on disk 
// into the "main" database of open database connection pInMemory, or
// to save the current contents of the database opened by pInMemory into
// a database file on disk. pInMemory is probably an in-memory database, 
// but this function will also work fine if it is not.

// Parameter zFilename points to a nul-terminated string containing the
// name of the database file on disk to load from or save to. If parameter
// isSave is non-zero, then the contents of the file zFilename are 
// overwritten with the contents of the database opened by pInMemory. If
// parameter isSave is zero, then the contents of the database opened by
// pInMemory are replaced by data loaded from the file zFilename.
//
// If the operation is successful, SQLITE_OK is returned. Otherwise, if
// an error occurs, an SQLite error code is returned.
static int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave) 
{
    sqlite3 *pFile;           /* Database connection opened on zFilename */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */
    sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
    sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

    // Open the database file identified by zFilename. Exit early if this fails
    // for any reason. 
    int rc = sqlite3_open(zFilename, &pFile);
    if (rc == SQLITE_OK)
    {
        // If this is a 'load' operation (isSave==0), then data is copied
        // from the database file just opened to database pInMemory. 
        // Otherwise, if this is a 'save' operation (isSave==1), then data
        // is copied from pInMemory to pFile.  Set the variables pFrom and
        // pTo accordingly. 
        pFrom = (isSave ? pInMemory : pFile);
        pTo   = (isSave ? pFile     : pInMemory);

        // Set up the backup procedure to copy from the "main" database of 
        // connection pFile to the main database of connection pInMemory.
        // If something goes wrong, pBackup will be set to NULL and an error
        // code and message left in connection pTo.
        //
        // If the backup object is successfully created, call backup_step()
        // to copy data from pFile to pInMemory. Then call backup_finish()
        // to release resources associated with the pBackup object.  If an
        // error occurred, then an error code and message will be left in
        // connection pTo. If no error occurred, then the error code belonging
        // to pTo is set to SQLITE_OK.
        pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        if( pBackup ){
            ::sqlite3_backup_step(pBackup, -1);
            ::sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(pTo);
    }

    // Close the database connection opened on database file zFilename
    // and return the result of this function. 
    ::sqlite3_close(pFile);
    return rc;
}

void CSqLiteDatabaseImpl::CommitDatabase()
{
    if ( !m_bMemory ) {   
        return;
    }
        
    const std::string localFileName = ds_str_conv::ConvertToUTF8(m_sFilePath.c_str());
    const int rc = ::loadOrSaveDb(m_pDB, localFileName.c_str(), 1);
    if (rc != SQLITE_OK) {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::CommitDatabase::loadOrSaveDb");    
    }
}