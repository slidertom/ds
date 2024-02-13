#include "stdafx.h"
#include "dsDatabase.h"

#ifndef __x86_64__ 
    #include "Dao/DaoDatabaseImpl.h"
#endif
#include "Dao/DaoFileUtil.h"
#include "AdoDotNet/AdoDotNetDatabaseImpl.h"
#include "sqlite/sqlite_database_impl.h"
#include "LogImpl/LogImpl.h"

#include "dsCopyTableData.h"
#include "dsTable.h"
#include "dsStrConv.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

dsDatabase::dsDatabase() 
{ 
    m_pErrorHandler = CLogImpl::Log; // default implementation
}

dsDatabase::~dsDatabase() 
{ 
    DeleteAllRoles(); 
    delete m_pDatabase;
}

void dsDatabase::RegisterListener(dsDatabaseListener *pListener) noexcept
{
    ASSERT(std::find(m_listeners.begin(), m_listeners.end(), pListener) == m_listeners.end());
    m_listeners.push_back(pListener);
}

void dsDatabase::UnregisterListener(dsDatabaseListener *pListener) noexcept
{
    auto found = std::find(m_listeners.begin(), m_listeners.end(), pListener);
    ASSERT(m_listeners.end() != found);
    m_listeners.erase(found);
}

void dsDatabase::Refresh() noexcept
{
    for (auto *pListener : m_listeners) {
        pListener->OnDatabaseClose();
    }
}

bool dsDatabase::IsReadOnly() const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->IsReadOnly();
}

bool dsDatabase::IsOpen() const noexcept
{
    if ( !m_pDatabase ) {
        return false;
    }
    return m_pDatabase->IsOpen();
}

std::wstring dsDatabase::GetName() const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->GetName();
}

bool dsDatabase::DoesTableExist(const char *sTable) const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->DoesTableExistUTF8(sTable);
}

bool dsDatabase::DoesTableExist(const wchar_t *sTable) const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->DoesTableExist(sTable);
}

bool dsDatabase::DoesViewExist(const char *sView) const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->DoesViewExistUTF8(sView);
}

bool dsDatabase::IsSqLiteDB(const wchar_t *sPath) noexcept
{
    return CSqLiteDatabaseImpl::IsSqLiteDB(sPath);
}

bool dsDatabase::IsDaoDB(const wchar_t *sPath) noexcept 
{
    return CDaoFileUtil::IsDaoDB(sPath);
}

bool dsDatabase::IsMSSQLServerAdoDotNet(const wchar_t *sPath) noexcept
{
    return CAdoDotNetDatabaseImpl::IsMSSQLServerAdoDotNet(sPath);
}

bool dsDatabase::OpenDB(const wchar_t *sPath, const dsOpenParams &params) noexcept
{
    Close(); // do auto close if opened

    ASSERT(!m_pDatabase);
    ASSERT(::wcslen(sPath) != 0);

    if ( CAdoDotNetDatabaseImpl::IsMSSQLServerAdoDotNet(sPath) ) {
        m_pDatabase = new CAdoDotNetDatabaseImpl;
    }
    else if ( CSqLiteDatabaseImpl::IsSqLiteDB(sPath) ) {
        m_pDatabase = new CSqLiteDatabaseImpl;
    }    
#ifndef __x86_64__ 
    else if ( CDaoDatabaseImpl::IsDaoDB(sPath) ) {
        m_pDatabase = new CDaoDatabaseImpl;
    } 
#endif
    else {
        return false;
    }

    m_pDatabase->SetErrorHandler(m_pErrorHandler);

    if ( !m_pDatabase->OpenDB(sPath, params) ) {
        Close();
        return false;
    }

    return true;
}

void dsDatabase::BeginTrans() noexcept
{
    ASSERT(m_pDatabase);
    m_pDatabase->BeginTrans();
}

void dsDatabase::CommitTrans() noexcept
{
    ASSERT(m_pDatabase);
    m_pDatabase->CommitTrans();

    if (m_funcPostCommitTrans) {
        m_funcPostCommitTrans();
    }
}

void dsDatabase::RollbackTrans() noexcept
{
    ASSERT(m_pDatabase);
    m_pDatabase->Rollback();
}

bool dsDatabase::Execute(const wchar_t *sSQL) noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->Execute(sSQL);
}

dsDBType dsDatabase::GetType() noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->GetType();
}

void dsDatabase::Close() noexcept
{
    for (dsDatabaseListener *pListener : m_listeners) {
        pListener->OnDatabaseClose();
    }

    if ( m_pDatabase )  {
        delete m_pDatabase;
        m_pDatabase = nullptr;
    }
}

void dsDatabase::CommitDatabase() noexcept
{
   ASSERT(m_pDatabase);
   m_pDatabase->CommitDatabase();
}

bool dsDatabase::CompactDatabase() noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->CompactDatabase();
}

bool dsDatabase::CompactDatabase(const wchar_t *sPath) noexcept
{
    dsDatabase database;
    if ( !database.OpenDB(sPath) ) {
        return false;
    }
    return database.CompactDatabase();
}

bool dsDatabase::CompactDatabase(const wchar_t *sPath, dbErrorHandler newHandler) noexcept
{
    dsDatabase database;
    database.SetErrorHandler(newHandler);
    if ( !database.OpenDB(sPath) ) {
        return false;
    }
    return database.CompactDatabase();
}

dsDatabase::dbErrorHandler dsDatabase::SetErrorHandler(dsDatabase::dbErrorHandler newHandler) noexcept
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    if ( m_pDatabase ) {
        m_pDatabase->SetErrorHandler(m_pErrorHandler);
    }
    return prevHandler;
}

void dsDatabase::DeleteRelation(const wchar_t *sRelation) noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->DeleteRelation(sRelation);
}

bool dsDatabase::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                const wchar_t *sField, const wchar_t *sForeignField) noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->CreateRelation(sName, sTable, sForeignTable, lAttr, sField, sForeignField);
}

void dsDatabase::SetLogPath(const wchar_t *sLogPath) noexcept
{
    CLogImpl::SetLogPath(sLogPath);
}

dsDatabase::dbErrorHandler dsDatabase::GetErrorHandler() noexcept
{
    return m_pErrorHandler;
}

bool dsDatabase::DropIndex(const wchar_t *sIndexName) noexcept
{
    return m_pDatabase->DropIndex(sIndexName);
}

std::wstring dsDatabase::AddUniqueIndexNoCase(const wchar_t *sTableName, const wchar_t *sFieldName) noexcept
{
    std::wstring sIndexName = sTableName;
    sIndexName += L"_";
    sIndexName += sFieldName;

    std::wstring sCreateStatement = L"CREATE UNIQUE INDEX `";
    sCreateStatement += sIndexName;
    sCreateStatement += L"` ON `";
    sCreateStatement += sTableName;
    sCreateStatement += L"` (`";
    sCreateStatement += sFieldName;
    sCreateStatement += L"` COLLATE NOCASE);";

    if (!m_pDatabase->Execute(sCreateStatement.c_str())) {
        return L"";
    }

    return sIndexName;
}

void dsDatabase::SetPostCommitHandler(const std::function<void()> &func) noexcept
{
    m_funcPostCommitTrans = func;
}

std::vector<std::string> dsDatabase::GetTableList() noexcept
{
    return m_pDatabase->GetTableList();
}

bool dsDatabase::DropForeignKeys(const wchar_t* sTableName) noexcept
{
    return m_pDatabase->DropForeignKeys(sTableName);
}

bool dsDatabase::DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName) noexcept
{
    return m_pDatabase->DropColumn(sTableName, sColumnName);
}

bool dsDatabase::DropColumn(const char *sTableName, const char *sColumnName) noexcept
{
    return m_pDatabase->DropColumn(sTableName, sColumnName);
}

bool dsDatabase::RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName) noexcept
{
    return m_pDatabase->RemoveColumnCollateNoCase(sTableName, sColumnName);
}

bool dsDatabase::DropTable(const wchar_t *sTableName) noexcept
{
    return m_pDatabase->DropTable(sTableName);
}

bool dsDatabase::DropTable(const char *sTableName) noexcept
{
    return m_pDatabase->DropTable(sTableName);
}

bool dsDatabase::DropTrigger(const wchar_t *sTriggerName) noexcept
{
    return m_pDatabase->DropTrigger(sTriggerName);
}

bool dsDatabase::Backup(const char *sBackupFile) noexcept
{
    return m_pDatabase->Backup(sBackupFile);
}

bool dsDatabase::GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info) noexcept
{
    if (!m_pDatabase->GetTableFieldInfo(sTable, info)) {
        return false;
    }
    return true;
}

bool dsDatabase::CreateTable(const char *sTableName, const dsTableFieldInfo &info) noexcept
{
    if (!m_pDatabase->CreateTable(sTableName, info)) {
        return false;
    }
    return true;
}

bool dsDatabase::CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info)  noexcept
{
    if (!m_pDatabase->CreateTables(tables_info)) {
        return false;
    }
    return true;
}

bool dsDatabase::CreateDB(const wchar_t *sPath, dsDBType eType) noexcept
{
    Close(); // do auto close if opened

    ASSERT(!m_pDatabase);
    ASSERT(::wcslen(sPath) != 0);

    if ( dsDBType::MsSQL == eType ) {
        m_pDatabase = new CAdoDotNetDatabaseImpl;
    }
    if ( dsDBType::SqLite == eType ) {
        m_pDatabase = new CSqLiteDatabaseImpl;
    }    
#ifndef __x86_64__ 
    else if ( dsDBType::Dao == eType ) {
        m_pDatabase = new CDaoDatabaseImpl;
    } 
#endif
    else {
        return false;
    }

    m_pDatabase->SetErrorHandler(m_pErrorHandler);

    if ( !m_pDatabase->CreateDB(sPath) ) {
        Close();
        return false;
    }

    return true;
}

bool dsDatabase::CopyData(dsDatabase &dbSource, dsDatabase &dbTarget) noexcept
{
    dsCopyTableData copy_table_data(&dbSource, &dbTarget);
    const std::vector<std::string> arrTargetTables = dbTarget.GetTableList();
    copy_table_data.BeginCopy();
    for (const std::string &sTable : arrTargetTables) {
        if (dbTarget.GetType() == dsDBType::SqLite && ::strcmp(sTable.c_str(), "sqlite_sequence") == 0) {
            continue;
        }

        dsTable table(&dbTarget, sTable.c_str());
        table.Flush();

        const std::wstring sTable_ = ds_str_conv::ConvertFromUTF8(sTable.c_str());
        if (!copy_table_data.CopyTableData(sTable_.c_str())) {
            return false;
        }
    }

    if (!copy_table_data.EndCopy()) {
        return false;
    }

    return true;
}

bool dsDatabase::CloneTo(const wchar_t *sPath, dsDBType eType) noexcept
{
    // Same type databases - use Backup functionality.
    if (GetType() == eType) {
        ASSERT(false);
    }

    dsDatabase dbTarget;
    if (!dbTarget.OpenDB(sPath)) {
        if (!dbTarget.CreateDB(sPath, dsDBType::Dao)) {
            return false;
        }

        std::vector<std::pair<std::string, dsTableFieldInfo>> tables_info;
        const std::vector<std::string> arrSourceTables = GetTableList();
        for (const std::string &sTable : arrSourceTables) {
            if ( GetType() == dsDBType::SqLite && ::strcmp(sTable.c_str(), "sqlite_sequence") == 0 ) {
                continue;
            }

            dsTableFieldInfo info;
            if (!GetTableFieldInfo(sTable.c_str(), info)) { 
                return false;
            }

            tables_info.push_back(std::make_pair(sTable, info));
        }

        if (!dbTarget.CreateTables(tables_info)) {
            return false;
        }
    } 

    if (!dsDatabase::CopyData(*this, dbTarget)) {
        return false;
    }

    return true;
}

bool dsDatabase::DoesIndexExist(const char *sIndex) const noexcept
{
    ASSERT(m_pDatabase);
    return m_pDatabase->DoesIndexExistUTF8(sIndex);
}