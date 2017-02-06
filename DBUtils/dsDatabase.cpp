#include "StdAfx.h"
#include "dsDatabase.h"

#include "Dao/DaoDatabaseImpl.h"
#include "AdoDotNet/AdoDotNetDatabaseImpl.h"
#include "SqLite/SqLiteDatabaseImpl.h"
#include "LogImpl/LogImpl.h"

#include "dsCopyTableData.h"
#include "dsTable.h"
#include "dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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

void dsDatabase::RegisterListener(dsDatabaseListener *pListner) 
{
    ASSERT(std::find(m_listners.begin(), m_listners.end(), pListner) == m_listners.end());
    m_listners.push_back(pListner);
}

void dsDatabase::UnregisterListener(dsDatabaseListener *pListner) 
{
    auto found = std::find(m_listners.begin(), m_listners.end(), pListner);
    ASSERT(m_listners.end() != found);
    m_listners.erase(found);
}

void dsDatabase::Refresh()
{
	auto end_it = m_listners.end();
	for (auto it = m_listners.begin(); it != end_it; ++it) {
		(*it)->OnDatabaseClose();
	}
}

bool dsDatabase::IsReadOnly() const
{
	ASSERT(m_pDatabase);
	return m_pDatabase->IsReadOnly();
}

bool dsDatabase::IsOpen() const
{
	if ( !m_pDatabase ) {
		return false;
	}
	return m_pDatabase->IsOpen();
}

std::wstring dsDatabase::GetName() const
{
	ASSERT(m_pDatabase);
	return m_pDatabase->GetName();
}

bool dsDatabase::DoesTableExist(const char *sTable) const
{
    ASSERT(m_pDatabase);
    std::wstring sTableUTF16 = ds_str_conv::ConvertFromUTF8(sTable);
	return m_pDatabase->DoesTableExist(sTableUTF16.c_str());
}

bool dsDatabase::DoesTableExist(const wchar_t *sTable) const
{
	ASSERT(m_pDatabase);
	return m_pDatabase->DoesTableExist(sTable);
}

bool dsDatabase::OpenDB(const wchar_t *sPath, const dsParams &params)
{
	Close(); // do auto close if opened
    
	ASSERT(!m_pDatabase);
	ASSERT(_tcslen(sPath) != 0);

	if ( CAdoDotNetDatabaseImpl::IsMSSQLServerAdoDotNet(sPath) ) {
		m_pDatabase = new CAdoDotNetDatabaseImpl;
	}
    else if ( CSqLiteDatabaseImpl::IsSqLiteDB(sPath) ) {
        m_pDatabase = new CSqLiteDatabaseImpl;
    }
    else if ( CDaoDatabaseImpl::IsDaoDB(sPath) ) {
		m_pDatabase = new CDaoDatabaseImpl;
	} 
    else {
        return false;
    }

    m_pDatabase->SetErrorHandler(m_pErrorHandler);

	if ( !m_pDatabase->OpenDB(sPath, params.m_bReadOnly, params.m_sKey.c_str(), params.m_bMultiUser) ) {
        Close();
        return false;
    }

    return true;
}

void dsDatabase::BeginTrans()
{
	ASSERT(m_pDatabase);
	m_pDatabase->BeginTrans();
}

void dsDatabase::CommitTrans()
{
	ASSERT(m_pDatabase);
	m_pDatabase->CommitTrans();
}

void dsDatabase::RollbackTrans()
{
	ASSERT(m_pDatabase);
	m_pDatabase->Rollback();
}

bool dsDatabase::Execute(const wchar_t *lpszSQL)
{
	ASSERT(m_pDatabase);
	return m_pDatabase->Execute(lpszSQL);
}

dsDBType dsDatabase::GetType()
{
	ASSERT(m_pDatabase);
	return m_pDatabase->GetType();
}

void dsDatabase::Close() 
{
	auto end_it = m_listners.end();
	for (auto it = m_listners.begin(); it != end_it; ++it) {
		(*it)->OnDatabaseClose();
	}

	if ( m_pDatabase ) 
    {
		if ( m_pDatabase->IsOpen() ) {
			m_pDatabase->Close(); 
		}

		delete m_pDatabase;
		m_pDatabase = nullptr;
	}
}

void dsDatabase::CommitDatabase()
{
   ASSERT(m_pDatabase);
   m_pDatabase->CommitDatabase();
}

bool dsDatabase::CompactDatabase()
{
    ASSERT(m_pDatabase);
    return m_pDatabase->CompactDatabase();
}

bool dsDatabase::CompactDatabase(const wchar_t *sPath)
{
    dsDatabase database;
    if ( !database.OpenDB(sPath) ) {
        return false;
    }
    return database.CompactDatabase();
}

dsDatabase::dbErrorHandler dsDatabase::SetErrorHandler(dsDatabase::dbErrorHandler newHandler)
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    if ( m_pDatabase ) {
        m_pDatabase->SetErrorHandler(m_pErrorHandler);
    }
    return prevHandler;
}

void dsDatabase::DeleteRelation(const wchar_t *sRelation)
{
	ASSERT(m_pDatabase);
    return m_pDatabase->DeleteRelation(sRelation);
}

bool dsDatabase::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, long lAttr,
						        const wchar_t *sField, const wchar_t *sForeignField)
{
	ASSERT(m_pDatabase);
	return m_pDatabase->CreateRelation(sName, sTable, sForeignTable, lAttr, sField, sForeignField);
}
