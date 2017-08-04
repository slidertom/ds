#include "StdAfx.h"
#include "SqLiteDatabaseImpl.h"

#include "sqlite_bind_util.h"
#include "sqlite_copy_table.h"
#include "sqlite_table_info.h"

#include "SqLiteErrorHandler.h"
#include "SqLiteRecordsetImpl.h"

#include "sqlite_include.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SQL_BUSY_TIMEOUT  5000

CSqLiteDatabaseImpl::CSqLiteDatabaseImpl(bool bMultiUser) 
: m_bReadOnly(false), m_pDB(nullptr), m_bMultiUser(bMultiUser)
{
    m_bTransMode = false;
    m_pErrorHandler = new CSqLiteErrorHandler;
}

CSqLiteDatabaseImpl::~CSqLiteDatabaseImpl()
{
    Close();
    delete m_pErrorHandler;
}

bool CSqLiteDatabaseImpl::IsSqLiteDB(const wchar_t *sPath)
{
    FILE *pFile = _tfopen(sPath, _T("rb"));
    if ( !pFile ) {
		return false;
	}

    bool bSqLite = false;
    char buffer[6];
    size_t nRead = fread(buffer, 1, 6, pFile);
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

    fclose(pFile);

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

    //m_bMultiUser controls if there can be database usage by multiple processes at one time
	if ( m_bMultiUser )
	{
		VERIFY(ExecuteUTF8("begin immediate transaction"));
	}
	else
	{
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
    std::string sql = ds_str_conv::ConvertToUTF8(lpszSQL);
    return ExecuteUTF8(sql.c_str());
}

void CSqLiteDatabaseImpl::Close() 
{
    if ( m_pDB ) {
	    sqlite3_close(m_pDB);
    }
    m_pDB = nullptr;
    m_sFilePath = _T("");

    auto end_it = m_table_field_info_map.end();
    for (auto it = m_table_field_info_map.begin(); it != end_it; ++it) {
        delete it->second;
    }
    m_table_field_info_map.clear();
}

// Exec an sql statement in values[0] against
// the database in pData.
int process_ddl_row(void * pData, int nColumns, 
        char **values, char **columns)
{
        if (nColumns != 1)
                return 1; // Error

        sqlite3* db = (sqlite3*)pData;
        sqlite3_exec(db, values[0], NULL, NULL, NULL);

        return 0;
}

// Insert from a table named by backup.{values[0]}
// into main.{values[0]} in database pData.
int process_dml_row(void *pData, int nColumns, 
        char **values, char **columns)
{
        if (nColumns != 1)
                return 1; // Error
        
        sqlite3* db = (sqlite3*)pData;

        char *stmt = sqlite3_mprintf("insert into main.%q "
                                     "select * from backup.%q", values[0], values[0]);
        sqlite3_exec(db, stmt, NULL, NULL, NULL);
        sqlite3_free(stmt);     

        return 0;
}

bool CSqLiteDatabaseImpl::OpenDB(const wchar_t *sPath, bool bReadOnly, const wchar_t *szPsw) 
{
    //int nRetVal = sqlite3_config(SQLITE_CONFIG_SERIALIZED); // SQLITE_CONFIG_SERIALIZED SQLITE_CONFIG_MULTITHREAD open -> SQLITE_OPEN_FULLMUTEX
    //VERIFY(nRetVal == SQLITE_OK);
    //nRetVal;

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
    //database.exec( "PRAGMA encoding = \"UTF-16\"" );

	//For shared database usage. Also for multithreaded database usage.
	//participation in cache system is controled later via SQLITE_OPEN_SHAREDCACHE and SQLITE_OPEN_PRIVATECACHE flags.
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
    std::string localFileName = ds_str_conv::ConvertToUTF8(sPath);
	int nFlags = bReadOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
	
	//For shared database usage. Also for multithreaded database usage.
	if ( m_bMultiUser ) {
		nFlags |= SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_FULLMUTEX;
    }
	else {
		nFlags |= SQLITE_OPEN_PRIVATECACHE;
	}

    int rc = sqlite3_open_v2(localFileName.c_str(), &m_pDB, nFlags, NULL);
    // sqlite3_open16 - UTF-16 does not allow to open in the read only mode
    //int rc = sqlite3_open16(sPath, &m_pDB);

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
    if (rc != SQLITE_OK)
    {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::OpenDB");
        Close();
        return false;
    }

	rc = sqlite3_busy_timeout(m_pDB, SQL_BUSY_TIMEOUT);
    if (rc != SQLITE_OK)
    {
        m_pErrorHandler->OnErrorCode(rc, m_pDB, "CSqLiteDatabaseImpl::OpenDB::sqlite3_busy_timeout");
        Close();
        return false;
    }
    /*
    if ( bReadOnly )
    {   // https://sqlite.org/backup.html
        sqlite3* memorydb;
        sqlite3_open(":memory:", &memorydb);

        sqlite3_backup *pBackup = sqlite3_backup_init(memorydb, "main", m_pDB, "main");
        if( pBackup ){
            (void)sqlite3_backup_step(pBackup, -1);
            (void)sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(memorydb);
        // TODO: error output    
        (void)sqlite3_close(m_pDB);
        m_pDB = memorydb;
    }
    */

    // http://www.sqlite.org/foreignkeys.html#fk_enable
    // Foreign key constraints are disabled by default (for backwards compatibility), 
    // so must be enabled separately for each database connection. (Note, however, that future releases of SQLite might change so that foreign key 
    // constraints enabled by default. 
    ExecuteUTF8("PRAGMA foreign_keys = ON");
    ExecuteUTF8("PRAGMA cache_size = 20000"); // default 2000
    //ExecuteUTF8("PRAGMA main.journal_mode = OFF");
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

bool CSqLiteDatabaseImpl::DoesTableExist(const wchar_t *sTable)
{
    std::string sTableUTF8 = ds_str_conv::ConvertToUTF8(sTable);
    std::string sSQL  = "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = '";
                sSQL += sTableUTF8;
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

bool CSqLiteDatabaseImpl::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, long lAttr,
									     const wchar_t *sField, const wchar_t *sForeignField)
{
	//ASSERT(FALSE);
	//Impl must be simillar to
	//sCreateRelSQL.Format(_T("ALTER TABLE %s ADD CONSTRAINT %s FOREIGN KEY (%s) REFERENCES %s(%s) ON DELETE CASCADE"), sTable1, sRelName, sField1, sTable2, sField2);
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
            nRepeatCount++;
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