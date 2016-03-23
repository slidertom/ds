#include "StdAfx.h"
#include "dsDatabase.h"

#include "Dao/DaoDatabaseImpl.h"
#include "AdoDotNet/AdoDotNetDatabaseImpl.h"
#include "SqLite/SqLiteDatabaseImpl.h"
#include "LogImpl/LogImpl.h"

#include "dsTable.h"

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

CStdString dsDatabase::GetName() const
{
	ASSERT(m_pDatabase);
	return m_pDatabase->GetName();
}

bool dsDatabase::DoesTableExist(LPCTSTR sTable) const
{
	ASSERT(m_pDatabase);
	return m_pDatabase->DoesTableExist(sTable);
}

bool dsDatabase::OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR sKey)
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

	return m_pDatabase->OpenDB(sPath, bReadOnly, sKey);
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

bool dsDatabase::Execute(LPCTSTR lpszSQL)
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

bool dsDatabase::CopyTableData(dsDatabase *pDstDB, LPCTSTR sTableName)
{
	return CopyTableData(pDstDB, sTableName, sTableName);
}

#include "SqLite/SqLiteUtil.h"
#include "SqLite/SqLiteDatabaseImpl.h"

bool dsDatabase::CopyTableData(dsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst)
{
	ASSERT(m_pDatabase);
	ASSERT(pDstDB);
    if ( pDstDB->GetType() == this->GetType() )
    {   // do use specific database  operations
	    return m_pDatabase->CopyTableData(pDstDB->m_pDatabase, sTableNameSrc, sTableNameDst);
    }

    dsTableFieldInfo union_info;
    if ( !ds_table_field_info_util::fields_union(union_info, m_pDatabase, sTableNameSrc, pDstDB->m_pDatabase, sTableNameDst) ) {
        return false;
    }

    if ( pDstDB->GetType() == dsType_SqLite )
    {  // special case for the sqlite database 
       CSqLiteDatabaseImpl *pDstDBImpl = dynamic_cast<CSqLiteDatabaseImpl *>(pDstDB->m_pDatabase);
       return sqlite_util::ImportTableData(this, pDstDBImpl, sTableNameSrc, sTableNameDst, union_info);
    }

    dsTable src_table(this, sTableNameSrc);
    if ( !src_table.MoveFirst() ) {
        return true; // empty table
    }
   
    dsTable dst_table(pDstDB, sTableNameDst);
    while ( !src_table.IsEOF() )
    {
        dst_table.AddNew();
            auto end_it = union_info.end();
            for (auto it = union_info.begin(); it != end_it; ++it) 
            {
                const TCHAR *sFieldName = it->first.c_str();

                if ( src_table.IsFieldValueNull(sFieldName) )
                {
                    dst_table.SetFieldNull(sFieldName);
                    continue;
                }

                switch (it->second) 
                {
                case dsFieldType_Text:
                    {
                        const CStdString sValue = src_table.GetFieldString(sFieldName);
                        dst_table.SetFieldString(sFieldName, sValue.c_str());
                    }
                    break;
                case dsFieldType_Long:
                    {
                        const int nValue = src_table.GetFieldLong(sFieldName);
                        dst_table.SetFieldLong(sFieldName, nValue);
                    }
                    break;
                case dsFieldType_Double:
                    {
                        const double dValue = src_table.GetFieldDouble(sFieldName);
                        dst_table.SetFieldDouble(sFieldName, dValue);
                    }
                    break;
                case dsFieldType_DateTime:
                    {
                        const time_t nTime = src_table.GetFieldDateTime(sFieldName);
                        dst_table.SetFieldDateTime(sFieldName, nTime);
                    }
                    break;
                case dsFieldType_Binary:
                    {
                        unsigned char *pData = nullptr;
                        unsigned long nSize = 0;
                        src_table.GetFieldBinary(sFieldName, &pData, nSize);        
                        dst_table.SetFieldBinary(sFieldName, pData, nSize);
                    }
                    break;
                }
            }
        dst_table.Update();

        src_table.MoveNext();
    }

    return true;
}

bool dsDatabase::CompactDatabase()
{
    ASSERT(m_pDatabase);
    return m_pDatabase->CompactDatabase();
}

bool dsDatabase::CompactDatabase(LPCTSTR sPath)
{
    dsDatabase database;
    if ( !database.OpenDB(sPath, false) ) {
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

void dsDatabase::DeleteRelation(LPCTSTR sRelation)
{
	ASSERT(m_pDatabase);
    return m_pDatabase->DeleteRelation(sRelation);
}

bool dsDatabase::CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
						        LPCTSTR sField, LPCTSTR sForeignField)
{
	ASSERT(m_pDatabase);
	return m_pDatabase->CreateRelation(sName, sTable, sForeignTable, lAttr, sField, sForeignField);
}
