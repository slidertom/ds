#include "StdAfx.h"
#include "SqLiteDatabaseImpl.h"

#include "SqLiteUtil.h"
#include "SqLiteErrorHandler.h"
#include "SqLiteRecordsetImpl.h"

#include "sqlite_include.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSqLiteDatabaseImpl::CSqLiteDatabaseImpl() 
: m_bReadOnly(false), m_pDB(nullptr)
{
#ifdef _DEBUG
    m_bTransMode = false;
#endif
    m_pErrorHandler = new CSqLiteErrorHandler;
}

CSqLiteDatabaseImpl::~CSqLiteDatabaseImpl()
{
    Close();
    delete m_pErrorHandler;
}

bool CSqLiteDatabaseImpl::IsSqLiteDB(LPCTSTR sPath)
{
    FILE *pFile = _tfopen(sPath, _T("rb"));
    if ( !pFile ) {
		return false;
	}

    bool bSqLite = false;
    char buffer[6];
    int nRead = fread(buffer, 1, 6, pFile);
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
    const bool bRetVal = ExecuteUTF8("vacuum");
    return bRetVal;
}

bool CSqLiteDatabaseImpl::BeginTrans() 
{
#ifdef _DEBUG
    ASSERT(!m_bTransMode); // nested transactions are not supported 
    m_bTransMode = true;
#endif
    // "begin deferred transaction"
    // "begin immediate transaction"
    // "begin exclusive transaction"
    ExecuteUTF8("begin transaction");
	return true;
}

bool CSqLiteDatabaseImpl::CommitTrans() 
{
#ifdef _DEBUG
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
#endif
    ExecuteUTF8("commit transaction");
	return true;
}

bool CSqLiteDatabaseImpl::Rollback()   
{
#ifdef _DEBUG
    ASSERT(m_bTransMode); // transaction should be started
    m_bTransMode = false;
#endif
    ExecuteUTF8("rollback transaction");
	return true;
}

bool CSqLiteDatabaseImpl::Execute(LPCTSTR lpszSQL) 
{
    std::string sql = ds_str_conv::ConvertToUTF8(lpszSQL);
    const bool bRetVal = ExecuteUTF8(sql.c_str());
    return bRetVal;
}

void CSqLiteDatabaseImpl::Close() 
{
    if ( m_pDB ) {
	    sqlite3_close(m_pDB);
    }
    m_pDB = nullptr;
    m_sFilePath = _T("");
}

bool CSqLiteDatabaseImpl::OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) 
{
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

    // http://utf8everywhere.org/
    m_sFilePath = sPath;
    // UTF8 path required
    std::string localFileName = ds_str_conv::ConvertToUTF8(sPath);
    int rc = sqlite3_open_v2(localFileName.c_str(), &m_pDB, bReadOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE, NULL);
    // sqlite3_open16 - UTF-16 does not allow to open in the read only mode
    //int rc = sqlite3_open16(sPath, &m_pDB);

    if (rc != SQLITE_OK)
    {
        const char *localError = "Out of memory";

        if ( m_pDB ) {
            localError = sqlite3_errmsg(m_pDB);
            Close();
        }
        m_pErrorHandler->OnError(rc, localError, _T("CSqLiteDatabaseImpl::OpenDB"));
        return false;
    }

    rc = sqlite3_extended_result_codes(m_pDB, 1);
    if (rc != SQLITE_OK)
    {
        const char *localError = sqlite3_errmsg(m_pDB);
        m_pErrorHandler->OnError(rc, localError, _T("CSqLiteDatabaseImpl::OpenDB"));
        Close();
        return false;
    }

    // http://www.sqlite.org/foreignkeys.html#fk_enable
    // Foreign key constraints are disabled by default (for backwards compatibility), 
    // so must be enabled separately for each database connection. (Note, however, that future releases of SQLite might change so that foreign key 
    // constraints enabled by default. 
    ExecuteUTF8("PRAGMA foreign_keys = ON");

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

CStdString CSqLiteDatabaseImpl::GetName()
{
	return m_sFilePath;
}

bool CSqLiteDatabaseImpl::DoesTableExist(LPCTSTR sTable)
{
    std::string sTableUTF8 = ds_str_conv::ConvertToUTF8(sTable);
    std::string sSQL  = "PRAGMA table_info(";
                sSQL += sTableUTF8;
                sSQL += ")";
    
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

void CSqLiteDatabaseImpl::DeleteRelation(LPCTSTR sRelation)
{
	//ASSERT(FALSE);
    //sqlite3_exec
}

bool CSqLiteDatabaseImpl::CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
									     LPCTSTR sField, LPCTSTR sForeignField)
{
	//ASSERT(FALSE);
	//Impl must be simillar to
	//CStdString sCreateRelSQL;
	//sCreateRelSQL.Format(_T("ALTER TABLE %s ADD CONSTRAINT %s FOREIGN KEY (%s) REFERENCES %s(%s) ON DELETE CASCADE"), sTable1, sRelName, sField1, sTable2, sField2);
	//pDB->ExecuteSQL(sCreateRelSQL.c_str());
	return true;
}

bool CSqLiteDatabaseImpl::ExecuteUTF8(const char *sqlUTF8)
{
    char *localError = 0;
    const int rc = sqlite3_exec(m_pDB, sqlUTF8, 0, 0, &localError);

    if (rc == SQLITE_OK) {
        return true;
    }
    else
    {
        m_pErrorHandler->OnError(rc, localError, _T("CSqLiteDatabaseImpl::ExecuteUTF8"));
        const CStdString sSQL = ds_str_conv::ConvertFromUTF8(sqlUTF8);
        m_pErrorHandler->OnError(sSQL.c_str(), _T("CSqLiteDatabaseImpl::ExecuteUTF8"));
        sqlite3_free(localError);
    }

    return false;
}

bool CSqLiteDatabaseImpl::GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info)
{
    std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTable);
    sqlite_util::CFieldInfoMap field_info_map;
    if ( !sqlite_util::GetTableFieldsdInfo(this, sTableNameUTF8.c_str(), m_pErrorHandler, field_info_map) )
    {
        CStdString sError;
        sError.Format(_T("GetTableFieldInfo failed. Table %s."), sTable);
        m_pErrorHandler->OnError(sError.c_str(), _T("CSqLiteDatabaseImpl::GetTableFieldInfo"));
        return false;
    }

    auto end_it = field_info_map.end();
    for (auto it = field_info_map.begin(); it != end_it; ++it) 
    {
        dsFieldType field_type = dsFieldType_Undefined;
        const sqlite_util::eFieldType type = it->second.GetFieldType();   
        switch (type)
        {
        case sqlite_util::eFieldType_Text:
            field_type = dsFieldType_Text;
            break;
        case sqlite_util::eFieldType_Long:
            field_type = dsFieldType_Long;
            break;
        case sqlite_util::eFieldType_Double:
            field_type = dsFieldType_Double;
            break;
        case sqlite_util::eFieldType_Binary:
            field_type = dsFieldType_Binary;
            break;
        }
    
        const CStdString sColName = ds_str_conv::ConvertFromUTF8(it->first.c_str());
        info[sColName] = field_type;
    }

    return true;
}

bool CSqLiteDatabaseImpl::CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst)
{
    ASSERT(FALSE); // UNTESTED
    // You'll have to attach Database X with Database Y using the Attach command, then run the appropriate Insert Into commands for the tables you want to transfer.
    //INSERT INTO X.TABLE(Id, Value) SELECT * FROM Y.TABLE;
    
    //ATTACH DATABASE "myother.db" AS aDB;
    // INSERT INTO newtable 
    //SELECT * FROM aDB.oldTableInMyOtherDB;
    // sqlite3_exec(db, "ATTACH 'C:/tmp/tmp.sqlite' as mytmp");
    ASSERT(pDstDB->GetType() == dsType_SqLite);
    const CStdString sPath = pDstDB->GetName();
    const std::string localFileName = ds_str_conv::ConvertToUTF8(sPath);
    std::string sSQL = "ATTACH '";
    sSQL += localFileName;
    sSQL += "' as DestDB";
    if ( !ExecuteUTF8(sSQL.c_str()) ) {
        return false;
    }
    std::string sTableNameSrcUTF8 = ds_str_conv::ConvertToUTF8(sTableNameSrc);
    std::string sTableNameDstUTF8 = ds_str_conv::ConvertToUTF8(sTableNameDst);
    //INSERT INTO newtable SELECT * FROM aDB.oldTableInMyOtherDB;
    sSQL = "INSERT INTO DestDB.";
    sSQL += sTableNameDstUTF8;
    sSQL += "SELECT * FROM ";
    sSQL += sTableNameSrcUTF8;
    if ( !ExecuteUTF8(sSQL.c_str()) ) {
        return false;
    }   
   
    return true;
}