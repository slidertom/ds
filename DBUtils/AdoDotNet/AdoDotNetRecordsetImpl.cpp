#include "StdAfx.h"
#include "AdoDotNetRecordsetImpl.h"

#include "AdoDotNetImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAdoDotNetRecordsetImpl::CAdoDotNetRecordsetImpl(CDotNetDatabaseAbs *pDatabase) 
{
	m_pSet = CAdoDotNetUtils::CreateRecordSet(pDatabase);
}

CAdoDotNetRecordsetImpl::~CAdoDotNetRecordsetImpl() 
{ 
	CAdoDotNetUtils::DeleteRecordSet(m_pSet);
}

void CAdoDotNetRecordsetImpl::OpenImpl()
{
	if ( m_pSet->IsOpen() ) {
		return;
	}

	CString sFind;
	sFind.Format(_T("SELECT * FROM %s"), m_sTable);
	m_pSet->Open(sFind);
}

bool CAdoDotNetRecordsetImpl::MoveFirst() 
{
	OpenImpl(); // we do not open on Open function
	return !m_pSet->IsEmpty();
}

bool CAdoDotNetRecordsetImpl::IsEOF() 
{
	return m_pSet->IsEOF();
}

bool CAdoDotNetRecordsetImpl::Open(LPCTSTR sTableName)
{
	m_sTable = sTableName;
	return true;
}

bool CAdoDotNetRecordsetImpl::OpenSQL(LPCTSTR sSQL)
{
    if ( IsOpen() )
	{
		m_pSet->Close();
	}

	return m_pSet->Open(sSQL);
}

bool CAdoDotNetRecordsetImpl::OpenView(LPCTSTR sViewName)
{
	ASSERT(FALSE);
	return false;
}

bool CAdoDotNetRecordsetImpl::MoveNext()
{
	return m_pSet->MoveNext();
}

bool CAdoDotNetRecordsetImpl::IsOpen() const
{
	return m_pSet->IsOpen();
}

void CAdoDotNetRecordsetImpl::SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)
{
	ASSERT(FALSE);
}

void CAdoDotNetRecordsetImpl::GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize)
{
	ASSERT(FALSE);
}

bool CAdoDotNetRecordsetImpl::Delete()
{
	ASSERT(FALSE);
	return true;
}

void CAdoDotNetRecordsetImpl::AddNew()
{
	ASSERT(FALSE);
}

void CAdoDotNetRecordsetImpl::Edit()
{
	ASSERT(FALSE);
}

bool CAdoDotNetRecordsetImpl::Update()
{
	ASSERT(FALSE);
	return false;
}

long CAdoDotNetRecordsetImpl::GetRecordCount()
{
	CString sCountSQL;
	sCountSQL.Format(_T("SELECT COUNT(*) AS RecordCount FROM %s"), m_sTable);
	m_pSet->Open(sCountSQL);

	const int nCount = GetFieldLong(_T("RecordCount"));

	m_pSet->Close();

	return nCount;
}

void CAdoDotNetRecordsetImpl::SetFieldValueNull(LPCTSTR lpszName)
{
	ASSERT(FALSE);
}

bool CAdoDotNetRecordsetImpl::DoesFieldExist(LPCTSTR sFieldName) 
{
	return m_pSet->DoesFieldExist(sFieldName);
}

bool CAdoDotNetRecordsetImpl::SeekByString(LPCTSTR sIndex, LPCTSTR sValue)
{
	if ( _tcslen(sIndex) <= 0 ) {
		ASSERT(FALSE);
		return false;
	}

	if ( m_sTable.IsEmpty() ) {
		ASSERT(FALSE);
		return false;
	}

	if ( IsOpen() ) {
		m_pSet->Close();
	}

	CString sFind;
	sFind.Format(_T("SELECT * FROM %s WHERE %s = '%s'"), m_sTable, sIndex, sValue);
	
	if ( !m_pSet->Open(sFind) ) {
		return false;
	}

	return !m_pSet->IsEmpty();
}

bool CAdoDotNetRecordsetImpl::SeekByLong(LPCTSTR sIndex, long nValue)
{
	if ( _tcslen(sIndex) <= 0 ) {
		ASSERT(FALSE);
		return false;
	}

	if ( m_sTable.IsEmpty() ) {
		ASSERT(FALSE);
		return false;
	}

	if ( IsOpen() )
	{
		m_pSet->Close();
	}

	CString sFind;
	sFind.Format(_T("SELECT * FROM %s WHERE %s = %d"), m_sTable, sIndex, nValue);
	
	if ( !m_pSet->Open(sFind) ) {
		return false;
	}

	return !m_pSet->IsEmpty();
}

CStdString CAdoDotNetRecordsetImpl::GetFieldString(LPCTSTR sFieldName)
{
	return (LPCTSTR)m_pSet->GetFieldString(sFieldName);
}

long CAdoDotNetRecordsetImpl::GetFieldLong(LPCTSTR sFieldName)
{
	return m_pSet->GetFieldLong(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldLong(LPCTSTR sFieldName, long lValue)
{
	m_pSet->SetFieldLong(sFieldName, lValue);
}

double CAdoDotNetRecordsetImpl::GetFieldDouble(LPCTSTR sFieldName) 
{
	return m_pSet->GetFieldDouble(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldDouble(LPCTSTR sFieldName, double dValue)
{
	m_pSet->SetFieldDouble(sFieldName, dValue);
}

time_t CAdoDotNetRecordsetImpl::GetFieldDateTime(LPCTSTR sFieldName)
{
	return m_pSet->GetFieldDateTime(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldDateTime(LPCTSTR sFieldName, const time_t &time)
{
	m_pSet->SetFieldDateTime(sFieldName, time);
}

bool CAdoDotNetRecordsetImpl::IsFieldValueNull(LPCTSTR sFieldName)
{
	return m_pSet->IsFieldValueNull(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue)
{
	m_pSet->SetFieldString(sFieldName, sValue);
}

#include "../sqlite/SqLiteUtil.h"

std::string CAdoDotNetRecordsetImpl::GetFieldStringUTF8(const char *sFieldName)
{
    const CStdString sValue = GetFieldString(sqlite_conv::ConvertFromUTF8(sFieldName).c_str());
    return sqlite_conv::ConvertToUTF8(sValue.c_str());
}

void CAdoDotNetRecordsetImpl::SetFieldStringUTF8(const char *sFieldName, const char *sValue)
{
    SetFieldString(sqlite_conv::ConvertFromUTF8(sFieldName).c_str(), sqlite_conv::ConvertFromUTF8(sValue).c_str());
}
