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
    Execute("vacuum");
    return true;
}

bool CSqLiteDatabaseImpl::BeginTrans() 
{
    ASSERT(!m_bTransMode); // nested transactions are not supported 
    m_bTransMode = true;
    // m_bMultiUser controls if there can be database usage by multiple processes at one time
    if ( m_bMultiUser ) {
        VERIFY(Execute("begin immediate transaction"));
    }
    else {
        VERIFY(Execute("begin transaction"));
    }

    //VERIFY(ExecuteUTF8("begin exclusive transaction"));
    return true;
}

bool CSqLiteDatabaseImpl::CommitTrans() 
{
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
    VERIFY(Execute("commit transaction"));
    return true;
}

bool CSqLiteDatabaseImpl::Rollback()   
{
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
    Execute("rollback transaction");
    return true;
}

bool CSqLiteDatabaseImpl::Execute(const wchar_t *lpszSQL) 
{
    const std::string sql = ds_str_conv::ConvertToUTF8(lpszSQL);
    return Execute(sql.c_str());
}

void CSqLiteDatabaseImpl::Close() 
{
    if ( m_pDB ) {
        ::sqlite3_close(m_pDB);
    }
    m_pDB = nullptr;
    m_sFilePath = L"";
    m_pErrorHandler->SetDatabaseLocation("");

    ClearFieldsInfo();
}

void CSqLiteDatabaseImpl::ClearFieldsInfo() 
{
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
    m_pErrorHandler->SetDatabaseLocation(localFileName.c_str());

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
    
    // if slow network disk -> m_bMemory open can increase read performance x10
    bool journal_mode_memory = false;

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

        // https://www.sqlite.org/pragma.html#pragma_journal_mode
        // Note that the journal_mode for an in-memory database is either MEMORY or OFF 
        // and can not be changed to a different value. An attempt to change the journal_mode 
        // of an in-memory database to any setting other than MEMORY or OFF is ignored. 
        journal_mode_memory = true;
    }

    // http://www.sqlite.org/foreignkeys.html#fk_enable
    // Foreign key constraints are disabled by default (for backwards compatibility), 
    // so must be enabled separately for each database connection. 
    // (Note, however, that future releases of SQLite might change so that foreign key 
    // constraints enabled by default. 
    Execute("PRAGMA foreign_keys = ON");
    Execute("PRAGMA cache_size = 20000"); // default 2000
    Execute("PRAGMA page_size = 65536");  // Query or set the page size of the database. The page size must be a power of two between 512 and 65536 inclusive. 
                                          // the default page size increased to 4096. The default page size is recommended for most applications. 
    Execute("PRAGMA temp_store=MEMORY;"); // PRAGMA temp_store = 0 | DEFAULT | 1 | FILE | 2 | MEMORY

    if ( open_params.m_bExclusive ) {
        Execute("PRAGMA locking_mode = EXCLUSIVE"); // 19 -> 13 the most sensitive for the network file
    }

    if ( m_bReadOnly )  {
        Execute("PRAGMA synchronous = OFF");    // network: 25 -> 19
        journal_mode_memory = true;
    }

    if ( journal_mode_memory ) {
        Execute("PRAGMA journal_mode=MEMORY;"); // 13 -> 12
    }
    else {
        Execute("PRAGMA journal_mode=WALL;");
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
    return dsDBType::SqLite;
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

bool CSqLiteDatabaseImpl::DoesViewExistUTF8(const char *sView)
{
    std::string sSQL = "SELECT 1 FROM sqlite_master WHERE type = 'view' AND name = '";
    sSQL += sView;
    sSQL += "' COLLATE NOCASE";

    CSqLiteRecordsetImpl loader(this, m_pErrorHandler);
    if (!loader.OpenSQLUTF8(sSQL.c_str())) {
        return false;
    }

    if (!loader.MoveFirstImpl()) {
        return false;
    }

    return true;
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

bool CSqLiteDatabaseImpl::Execute(const char *sqlUTF8)
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
                    sFuncDescr += " SQL: ";
                    sFuncDescr += sqlUTF8;
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

bool CSqLiteDatabaseImpl::GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info)
{
    const sqlite_util::CFieldInfoMap *pFieldInfoMap = GetTableFieldInfoImpl(sTable);
    if ( !pFieldInfoMap ) {
        std::string sError = "GetTableFieldInfo failed. Table ";
                    sError += sTable;
        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::GetTableFieldInfo");
        return false;
    }

    auto end_it = pFieldInfoMap->end();
    for (auto it = pFieldInfoMap->begin(); it != end_it; ++it) 
    {
        dsFieldType field_type = it->second.GetFieldType();
        info[it->first] = field_type;
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
        char *localError = 0;
        // we do copy from the memory to local file 
        // or from file to memory 
        // journal can be ignored 
        sqlite3_exec(pFile, "PRAGMA journal_mode=OFF", 0, 0, &localError); 
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

bool CSqLiteDatabaseImpl::Backup(const char *sBackupFile)
{
    const int32_t rc = ::loadOrSaveDb(m_pDB, sBackupFile, 1);
    const bool bRet  = (rc == SQLITE_OK);

    if ( !bRet ) {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::Backup::loadOrSaveDb");
    }

    return bRet;
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

std::vector<std::string> CSqLiteDatabaseImpl::GetTableList()
{
    std::vector<std::string> arrTables;
    
    CSqLiteRecordsetImpl loader(this, m_pErrorHandler);
    if (!loader.OpenSQLUTF8("SELECT name FROM sqlite_master WHERE type='table'")) {
        return arrTables;
    }

    if (!loader.MoveFirstImpl()) {
        return arrTables;
    }

    while (!loader.IsEOF()) {
        arrTables.push_back(loader.GetFieldStringUTF8("name"));
        loader.MoveNext();
    }

    return arrTables;
}

class CTransRestore 
{
public:
    CTransRestore(CSqLiteDatabaseImpl *pDB, bool bTransMode) : m_pDB(pDB) {
        if (bTransMode) {
            m_pDB->CommitTrans();
            m_bStartTrans = true;
        }
    };
    ~CTransRestore() {
        if (m_bStartTrans) {
            m_pDB->BeginTrans();
        }
    };

private:
    CSqLiteDatabaseImpl *m_pDB {nullptr};
    bool m_bStartTrans {false};
};

bool CSqLiteDatabaseImpl::RecreateTable(const char *sTableName, const char *sColumnName, bool bDelete, bool bRemoveCollateNoCase)
{
    // CTransRestore has been implemented in order to have correct sequence of drop collumn functionality
    // which involves the usage of BEGIN TRANSACTION command.
    // Information about correct order of such operations can be found in https://sqlite.org/lang_altertable.html section "5. Making Other Kinds Of Table Schema Changes".
    // In short:
    // 1. Create new table
    // 2. Copy data
    // 3. Drop old table
    // 4. Rename new into old 

    // Transaction must be controlled inside RecreateTable
    CTransRestore transRestore(this, m_bTransMode);
    // SQLite drop column implementation is based on https://www.sqlite.org/faq.html#q11
    ClearFieldsInfo(); // To always have newest fields information before collecting all the information.
    const sqlite_util::CFieldInfoMap *pFieldInfoMap = GetTableFieldInfoImpl(sTableName);
    if ( !pFieldInfoMap ) {
        std::string sError = "RecreateTable failed. Table ";
                    sError += sTableName;
                    sError += " does not exist.";
        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::RecreateTable");
        return false;
    }

    bool bColumnExist(false);
    for (const auto &it : *pFieldInfoMap) {
        if (it.first == sColumnName) {
            bColumnExist = true;
            break;
        }
    }

    if (!bColumnExist && (bDelete || bRemoveCollateNoCase)) {
        std::string sError = "ModifyColumn failed. No column ";
                    sError += sColumnName;            
                    sError += " defined in table ";
                    sError += sTableName;
                    sError += ".";

        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::ModifyColumn");
        return false;
    }

    // Get fields list with UNIQUE flag and create statements for indexes
    std::vector<std::string> sUniqueFields;
    std::unordered_map<std::string, std::string> mapIndexSQLs;
    sqlite_util::sqlite_get_table_index_info(this, sTableName, m_pErrorHandler, sUniqueFields, mapIndexSQLs);

    std::string sMainSQL;
    sMainSQL += "PRAGMA foreign_keys=off;\n";
    sMainSQL += "BEGIN TRANSACTION;\n";
  
    // Constructing CREATE statement
    std::string sPrimary;
    std::string sFieldsList;
    sMainSQL += "CREATE TABLE \"";
    sMainSQL += sTableName;
    sMainSQL += "_temporal\" (";
    for (const auto &it : *pFieldInfoMap) {
        if (bDelete && it.first == sColumnName) {
            continue;
        }

        sFieldsList += it.first + ", ";

        sMainSQL += "\"" + it.first + "\" " + it.second.m_sType;

        if (it.second.m_bPrimary) {
            sPrimary = it.first;
        }

        if (it.second.m_bNotNull) {
            sMainSQL += " NOT NULL";
        }

        const auto it_find_unique = std::find(sUniqueFields.begin(), sUniqueFields.end(), it.first);
        if (sUniqueFields.end() != it_find_unique) {
            sMainSQL += " UNIQUE";
        }

        if (!it.second.m_sDefault.empty()) {
            sMainSQL += " DEFAULT ";
            sMainSQL += it.second.m_sDefault;
        }

		// Collate settings
        const char* pCollSeq;
		const int ret = sqlite3_table_column_metadata(m_pDB, nullptr, sTableName, it.first.c_str(), nullptr, &pCollSeq, nullptr, nullptr, nullptr);
        if (SQLITE_OK == ret) {
            const std::string sCollate = pCollSeq;
            if ("BINARY" != sCollate && !bRemoveCollateNoCase) { // Default is Binary
                sMainSQL += " COLLATE ";
                sMainSQL += sCollate;
            }
        }

        sMainSQL += ",";
    }

    if (!sPrimary.empty()) {
        sMainSQL +=  " PRIMARY KEY(\"" + sPrimary + "\")";
    }
    else {
        sMainSQL.pop_back(); // To remove last comma.
    }

    sMainSQL += ");\n";

    // Remowing last comma and space
    sFieldsList.pop_back();
    sFieldsList.pop_back();

    // Creating INSERT statement
    sMainSQL += "INSERT INTO ";
    sMainSQL += sTableName;
    sMainSQL += "_temporal";
    sMainSQL += " (";
    sMainSQL += sFieldsList;
    sMainSQL += ") SELECT ";
    sMainSQL += sFieldsList;
    sMainSQL += " FROM ";
    sMainSQL += sTableName;
    sMainSQL += ";\n";

    sMainSQL += "DROP TABLE ";
    sMainSQL += sTableName;
    sMainSQL += ";\n";

    sMainSQL += "ALTER TABLE ";
    sMainSQL += sTableName;
    sMainSQL += "_temporal";
    sMainSQL += " RENAME TO ";
    sMainSQL += sTableName;
    sMainSQL += ";\n";

    // Restore Indexes
    // TODO: restore Triggers and Views
    for (const auto &it : mapIndexSQLs) {
        if (it.first == sColumnName) {
            continue;
        }
        sMainSQL += it.second + ";\n";
    }

    sMainSQL += "COMMIT;\n";
    sMainSQL += "PRAGMA foreign_keys=on;\n";
     
    if (!Execute(sMainSQL.c_str())) {
        Execute("ROLLBACK;");
        std::string sError = "RecreateTable failed. SQL error: ";
                    sError += sMainSQL;            
                    sError += ".";

        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::RecreateTable");
        return false;
    }

    if (bDelete) {
        auto found = m_table_field_info_map.find(sTableName);
        if (found != m_table_field_info_map.end()) {
            found->second->erase(sColumnName);
        }
    }

    return true;
}

bool CSqLiteDatabaseImpl::DropForeignKeys(const wchar_t* sTableName)
{
    const std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTableName);
    // Re-create table without foreign keys
    if (!RecreateTable(sTableNameUTF8.c_str(), "", false, false)) {
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    const std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTableName);
    const std::string sColumnNameUTF8 = ds_str_conv::ConvertToUTF8(sColumnName);
    if (!RecreateTable(sTableNameUTF8.c_str(), sColumnNameUTF8.c_str(), true, false)) {
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::DropColumn(const char *sTableName, const char *sColumnName)
{
    if (!RecreateTable(sTableName, sColumnName, true, false)) {
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    const std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTableName);
    const std::string sColumnNameUTF8 = ds_str_conv::ConvertToUTF8(sColumnName);
    if (!RecreateTable(sTableNameUTF8.c_str(), sColumnNameUTF8.c_str(), false, true)) {
        return false;
    }

    return true;
}
bool CSqLiteDatabaseImpl::DropTable(const wchar_t *sTableName)
{
    const std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTableName);
    return this->DropTable(sTableNameUTF8.c_str());
}

bool CSqLiteDatabaseImpl::DropTable(const char *sTableName)
{
    std::string sTriggerList;
    sTriggerList = "SELECT * FROM sqlite_master WHERE type='trigger' AND tbl_name='"; 
    sTriggerList += sTableName;
    sTriggerList += "';";

    CSqLiteRecordsetImpl loaderTriggerList(this, m_pErrorHandler);
    if (loaderTriggerList.OpenSQLUTF8(sTriggerList.c_str())) {
        if (loaderTriggerList.MoveFirstImpl()) {
            while ( !loaderTriggerList.IsEOF()) {
                const std::string sTriggerName = loaderTriggerList.GetFieldStringUTF8("name");
                std::string sSQL = "DROP TRIGGER ";
                sSQL += sTriggerName;
                sSQL += ";";

                if (!this->Execute(sSQL.c_str())) {
                    std::string sError = "DropTable failed. DROP TRIGGER SQL error: ";
                    sError += sSQL;            
                    sError += ".";

                    m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::DropTable");
                }

                loaderTriggerList.MoveNext();
            }
        }
    }

    std::string sSQL = "DROP TABLE ";
    sSQL += sTableName;
    sSQL += ";";

    if (!Execute(sSQL.c_str())) {
        std::string sError = "DropTable failed. DROP TABLE SQL error: ";
        sError += sSQL;            
        sError += ".";

        m_pErrorHandler->OnError(sError.c_str(), "CSqLiteDatabaseImpl::DropTable");
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::DropTrigger(const wchar_t *sTriggerName)
{
    // To prevent from "Table is locked error"
    {
        const std::string sTriggerNameUTF8 = ds_str_conv::ConvertToUTF8(sTriggerName);
        std::string sTriggerList;
        sTriggerList = "SELECT * FROM sqlite_master WHERE type='trigger' AND name='"; 
        sTriggerList += sTriggerNameUTF8;
        sTriggerList += "';";

        CSqLiteRecordsetImpl loaderTriggerList(this, m_pErrorHandler);
        if (!loaderTriggerList.OpenSQLUTF8(sTriggerList.c_str())) {
            return false;
        }

        if (!loaderTriggerList.MoveFirstImpl()) {
            return true;
        }
    }

    std::wstring sSQL = L"DROP TRIGGER ";
    sSQL += sTriggerName;
    sSQL += L";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::DropIndex(const wchar_t *sIndexName)
{
    // To prevent from "Table is locked error"
    {
        const std::string sIndexNameUTF8 = ds_str_conv::ConvertToUTF8(sIndexName);
        std::string sIndexList;
        sIndexList = "SELECT * FROM sqlite_master WHERE type='index' AND name='";
        sIndexList += sIndexNameUTF8;
        sIndexList += "';";

        CSqLiteRecordsetImpl loaderIndexList(this, m_pErrorHandler);
        if (!loaderIndexList.OpenSQLUTF8(sIndexList.c_str())) {
            return false;
        }

        if (!loaderIndexList.MoveFirstImpl()) {
            return true;
        }
    }

    std::wstring sDropStatement = L"DROP INDEX ";
    sDropStatement += sIndexName;
    sDropStatement += L";";
    if (!Execute(sDropStatement.c_str())){
        return false;
    }

    return true;
}

bool CSqLiteDatabaseImpl::CreateTable(const char *sTableName, const dsTableFieldInfo &info)
{
    ASSERT(false);
    return false;
}

bool CSqLiteDatabaseImpl::CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info)
{
    ASSERT(false);
    return false;
}

bool CSqLiteDatabaseImpl::CreateDB(const wchar_t *sPath)
{
    ASSERT(false);
    return false;
}

bool CSqLiteDatabaseImpl::DoesIndexExistUTF8(const char *sIndex)
{
    std::string sSQL = "SELECT 1 FROM sqlite_master WHERE type = 'index' AND name = '";
    sSQL += sIndex;
    sSQL += "' COLLATE NOCASE";

    CSqLiteRecordsetImpl loader(this, m_pErrorHandler);
    if (!loader.OpenSQLUTF8(sSQL.c_str())) {
        return false;
    }

    if (!loader.MoveFirstImpl()) {
        return false;
    }

    return true;
}