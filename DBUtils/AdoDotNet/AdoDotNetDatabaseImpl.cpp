#include "stdafx.h"
#include "AdoDotNetDatabaseImpl.h"

#include "AdoDotNetImpl.h"
#include "AdoDotNetRecordsetImpl.h"

#include "../dsStrConv.h"
#include "../dsOpenParams.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

// String sample:
// Data Source=SQL-SERVER;Initial Catalog=catalog_name;Integrated Security=False;User ID=sa;Password=passsword
bool CAdoDotNetDatabaseImpl::IsMSSQLServerAdoDotNet(const wchar_t *sPath)
{
    std::wstring sPathLC(sPath);
    ds_str_conv::MakeLower(sPathLC);
    if (::wcsstr(sPathLC.c_str(), L"data source=")) {
        return true;
    }
    return false;
}

CAdoDotNetDatabaseImpl::CAdoDotNetDatabaseImpl() 
{
    m_pDatabase = CAdoDotNetUtils::CreateDatabase();
}

CAdoDotNetDatabaseImpl::~CAdoDotNetDatabaseImpl()
{
    if ( IsOpen() ) {
        Close();
    }
    CAdoDotNetUtils::DeleteDatabase(m_pDatabase);
}

bool CAdoDotNetDatabaseImpl::BeginTrans() 
{
    return true;
}

bool CAdoDotNetDatabaseImpl::CommitTrans() 
{
    return true;
}

bool CAdoDotNetDatabaseImpl::Rollback()   
{
    return true;
}

bool CAdoDotNetDatabaseImpl::Execute(const wchar_t *sSQL) 
{
    return m_pDatabase->Execute(sSQL);
}

bool CAdoDotNetDatabaseImpl::Execute(const char *sSQL)
{
    const std::wstring sSQLUTF16 = ds_str_conv::ConvertFromUTF8(sSQL);
    return Execute(sSQLUTF16.c_str());
}

void CAdoDotNetDatabaseImpl::Close() 
{
    m_pDatabase->Close();
    m_sConnString = L"";
}

bool CAdoDotNetDatabaseImpl::OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) 
{
    m_sConnString = sPath;
    m_bReadOnly = open_params.m_bReadOnly;
    const bool bRet = m_pDatabase->Open(sPath);
    return bRet;
}

dsDBType CAdoDotNetDatabaseImpl::GetType()
{
    return dsDBType::MsSQL;
}

bool CAdoDotNetDatabaseImpl::IsReadOnly() const
{
    return m_bReadOnly; 
}

bool CAdoDotNetDatabaseImpl::IsOpen() const
{
    return m_pDatabase->IsOpen();
}

std::wstring CAdoDotNetDatabaseImpl::GetName()
{
    return m_sConnString;
}

bool CAdoDotNetDatabaseImpl::DoesTableExistUTF8(const char *sTable) 
{
    const std::wstring sTableUTF16 = ds_str_conv::ConvertFromUTF8(sTable);
    return m_pDatabase->DoesTableExists(sTableUTF16.c_str());
}

bool CAdoDotNetDatabaseImpl::DoesViewExistUTF8(const char *sView)
{
    ASSERT(FALSE);
    return false;
}

bool CAdoDotNetDatabaseImpl::DoesTableExist(const wchar_t *sTable)
{
    return m_pDatabase->DoesTableExists(sTable);
}

CAbsRecordset *CAdoDotNetDatabaseImpl::CreateRecordset()
{
    return new CAdoDotNetRecordsetImpl(m_pDatabase);
}

CAdoDotNetDatabaseImpl::dbErrorHandler CAdoDotNetDatabaseImpl::SetErrorHandler(CAdoDotNetDatabaseImpl::dbErrorHandler newHandler)
{
    return m_pDatabase->SetErrorHandler(newHandler);
}

void CAdoDotNetDatabaseImpl::DeleteRelation(const wchar_t *sRelation)
{
    ASSERT(FALSE);
}

bool CAdoDotNetDatabaseImpl::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                            const wchar_t *sField, const wchar_t *sForeignField)
{
    ASSERT(FALSE);
    //Impl must be simillar to
    //std::wstring sCreateRelSQL;
    //sCreateRelSQL.Format(_T("ALTER TABLE %s ADD CONSTRAINT %s FOREIGN KEY (%s) REFERENCES %s(%s) ON DELETE CASCADE"), sTable1, sRelName, sField1, sTable2, sField2);
    //pDB->ExecuteSQL(sCreateRelSQL.c_str());
    return true;
}

bool CAdoDotNetDatabaseImpl::GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info)
{
    const std::wstring sTableUTF16 = ds_str_conv::ConvertFromUTF8(sTable);
    return m_pDatabase->GetTableFieldInfo(sTableUTF16.c_str(), info);
}

std::vector<std::string> CAdoDotNetDatabaseImpl::GetTableList()
{
    ASSERT(FALSE);
    std::vector<std::string> arrTables;
    return arrTables;
}

bool CAdoDotNetDatabaseImpl::DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    std::wstring sSQL = L"ALTER TABLE ";
    sSQL += sTableName;
    sSQL += L" DROP COLUMN ";
    sSQL += sColumnName;
    sSQL += L";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CAdoDotNetDatabaseImpl::DropColumn(const char *sTableName, const char *sColumnName)
{
    std::string sSQL = "ALTER TABLE ";
    sSQL += sTableName;
    sSQL += " DROP COLUMN ";
    sSQL += sColumnName;
    sSQL += ";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CAdoDotNetDatabaseImpl::DropForeignKeys(const wchar_t* sTableName)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::DropTable(const wchar_t *sTableName)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::DropTable(const char *sTableName)
{
    ASSERT(FALSE);
    return false;
}

bool CAdoDotNetDatabaseImpl::DropTrigger(const wchar_t *sTriggerName)
{
    ASSERT(FALSE);
    return true;
}

bool CAdoDotNetDatabaseImpl::DropIndex(const wchar_t *sIndexName)
{
    std::wstring sDropStatement = L"DROP INDEX ";
    sDropStatement += sIndexName;
    sDropStatement += L";";
    if (!Execute(sDropStatement.c_str())){
        return false;
    }

    return true;
}

bool CAdoDotNetDatabaseImpl::Backup(const char *sBackupFile)
{
    ASSERT(FALSE);
    return true;
}

bool CAdoDotNetDatabaseImpl::CreateTable(const char *sTableName, const dsTableFieldInfo &info)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::CreateDB(const wchar_t *sPath)
{
    ASSERT(false);
    return false;
}

bool CAdoDotNetDatabaseImpl::DoesIndexExistUTF8(const char *sIndex)
{
    ASSERT(false);
    return false;
}