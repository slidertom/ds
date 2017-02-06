#include "StdAfx.h"
#include "AdoDotNetRecordsetImpl.h"

#include "AdoDotNetImpl.h"

#include "../dsStrConv.h"

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

bool CAdoDotNetRecordsetImpl::Open(const wchar_t *sTableName)
{
	m_sTable = sTableName;
	return true;
}

bool CAdoDotNetRecordsetImpl::OpenSQL(const wchar_t *sSQL)
{
    if ( IsOpen() ) {
		m_pSet->Close();
	}

	return m_pSet->Open(sSQL);
}

bool CAdoDotNetRecordsetImpl::OpenView(const wchar_t *sViewName)
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

void CAdoDotNetRecordsetImpl::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, unsigned long nSize)
{
	ASSERT(FALSE);
}

void CAdoDotNetRecordsetImpl::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, unsigned long &nSize)
{
	ASSERT(FALSE);
}

void CAdoDotNetRecordsetImpl::FreeBinary(unsigned char *pData)
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

void CAdoDotNetRecordsetImpl::SetFieldValueNull(const wchar_t *lpszName)
{
	ASSERT(FALSE);
}

bool CAdoDotNetRecordsetImpl::DoesFieldExist(const wchar_t *sFieldName) 
{
	return m_pSet->DoesFieldExist(sFieldName);
}

bool CAdoDotNetRecordsetImpl::SeekByString(const wchar_t *sIndex, const wchar_t *sValue)
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
	sFind.Format(_T("SELECT * FROM %s WHERE %s = "), m_sTable, sIndex);

	if ( !m_pSet->SeekByString(sFind, sValue) ) {
		return false;
	}

	return !m_pSet->IsEmpty();
}

bool CAdoDotNetRecordsetImpl::SeekByLong(const wchar_t *sIndex, long nValue)
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

std::wstring CAdoDotNetRecordsetImpl::GetFieldString(const wchar_t *sFieldName)
{
	return (const wchar_t *)m_pSet->GetFieldString(sFieldName);
}

long CAdoDotNetRecordsetImpl::GetFieldLong(const wchar_t *sFieldName)
{
	return m_pSet->GetFieldLong(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldLong(const wchar_t *sFieldName, long lValue)
{
	m_pSet->SetFieldLong(sFieldName, lValue);
}

double CAdoDotNetRecordsetImpl::GetFieldDouble(const wchar_t *sFieldName) 
{
	return m_pSet->GetFieldDouble(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldDouble(const wchar_t *sFieldName, double dValue)
{
	m_pSet->SetFieldDouble(sFieldName, dValue);
}

time_t CAdoDotNetRecordsetImpl::GetFieldDateTime(const wchar_t *sFieldName)
{
	return m_pSet->GetFieldDateTime(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldDateTime(const wchar_t *sFieldName, const time_t &time)
{
	m_pSet->SetFieldDateTime(sFieldName, time);
}

bool CAdoDotNetRecordsetImpl::IsFieldValueNull(const wchar_t *sFieldName)
{
	return m_pSet->IsFieldValueNull(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue)
{
	m_pSet->SetFieldString(sFieldName, sValue);
}

std::string CAdoDotNetRecordsetImpl::GetFieldStringUTF8(const char *sFieldName)
{
    const std::wstring sValue = GetFieldString(ds_str_conv::ConvertFromUTF8(sFieldName).c_str());
    return ds_str_conv::ConvertToUTF8(sValue.c_str());
}

void CAdoDotNetRecordsetImpl::SetFieldStringUTF8(const char *sFieldName, const char *sValue)
{
    SetFieldString(ds_str_conv::ConvertFromUTF8(sFieldName).c_str(), ds_str_conv::ConvertFromUTF8(sValue).c_str());
}
