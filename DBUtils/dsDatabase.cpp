#include "stdafx.h"
#include "dsDatabase.h"

#ifndef __x86_64__ 
    #include "Dao/DaoDatabaseImpl.h"
#endif
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

bool dsDatabase::IsSqLiteDB(const wchar_t *sPath) noexcept
{
    return CSqLiteDatabaseImpl::IsSqLiteDB(sPath);
}

bool dsDatabase::IsDaoDB(const wchar_t *sPath) noexcept 
{
    #ifndef __x86_64__ 
        return CDaoDatabaseImpl::IsDaoDB(sPath);
    #endif
    return false; // x64 build dao database is not supported
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
        m_pDatabase = new CSqLiteDatabaseImpl();
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

    if (m_postCommitTrans) {
        m_postCommitTrans();
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
    std::wstring sDropStatement = L"DROP INDEX ";
    sDropStatement += sIndexName;
    sDropStatement += L";";
    if (!m_pDatabase->Execute(sDropStatement.c_str())){
        return false;
    }

    return true;
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

void dsDatabase::SetPostCommitHandler(const FuncPostCommitTrans &func) noexcept 
{
    m_postCommitTrans = func;
}

bool dsDatabase::DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName) noexcept
{
    return m_pDatabase->DropColumn(sTableName, sColumnName);
}