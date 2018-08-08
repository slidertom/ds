#include "stdafx.h"
#include "dsDatabase.h"

#ifndef __x86_64__ 
	#include "Dao/DaoDatabaseImpl.h"
#endif
#include "AdoDotNet/AdoDotNetDatabaseImpl.h"
#include "SqLite/SqLiteDatabaseImpl.h"
#include "LogImpl/LogImpl.h"

#include "dsCopyTableData.h"
#include "dsTable.h"
#include "dsStrConv.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

dsDatabase::dsDatabase() 
:  m_pDatabase(nullptr) 
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

bool dsDatabase::OpenDB(const wchar_t *sPath, const dsParams &params) noexcept
{
	Close(); // do auto close if opened
    
	ASSERT(!m_pDatabase);
	ASSERT(_tcslen(sPath) != 0);

	if ( CAdoDotNetDatabaseImpl::IsMSSQLServerAdoDotNet(sPath) ) {
		m_pDatabase = new CAdoDotNetDatabaseImpl;
	}
    else if ( CSqLiteDatabaseImpl::IsSqLiteDB(sPath) ) {
        m_pDatabase = new CSqLiteDatabaseImpl(params.m_bMultiUser);
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

	if ( !m_pDatabase->OpenDB(sPath, params.m_bReadOnly, params.m_sKey.c_str()) ) {
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

dsDatabase::dbErrorHandler dsDatabase::GetErrorHandler()
{
	return m_pErrorHandler;
}