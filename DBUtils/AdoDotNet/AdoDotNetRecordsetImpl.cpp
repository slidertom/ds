#include "stdafx.h"
#include "AdoDotNetRecordsetImpl.h"

#include "AdoDotNetImpl.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

CAdoDotNetRecordsetImpl::CAdoDotNetRecordsetImpl(CDotNetDatabaseAbs *pDatabase)
: m_pDB(pDatabase)
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

    std::wstring sFind = L"SELECT * FROM ";
    sFind += m_sTable;
    m_pSet->Open(sFind.c_str());
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
    if ( m_bSQLOpened ) {
        return true;
    }

    m_sTable = sTableName;
    return true;
}

bool CAdoDotNetRecordsetImpl::OpenSQL(const wchar_t *sSQL)
{
    if ( IsOpen() ) {
        m_pSet->Close();
    }

    if (!m_pSet->Open(sSQL)) {
        return false;
    }

    m_sTable = sSQL;
    m_bSQLOpened = true;
    return true;
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

void CAdoDotNetRecordsetImpl::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize)
{
    ASSERT(FALSE);
}

void CAdoDotNetRecordsetImpl::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize)
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

int CAdoDotNetRecordsetImpl::GetRecordCount() const
{
    CAdoDotNetRecordsetImpl loader(m_pDB);
    std::wstring sCountSQL;
    if (m_bSQLOpened) {
        sCountSQL = L"SELECT COUNT(*) AS RecordCount FROM (";
        sCountSQL += m_sTable;
        sCountSQL += L") AS TheMain";
    }
    else {
        sCountSQL = L"SELECT COUNT(*) AS RecordCount FROM ";
        sCountSQL += m_sTable;
    }

    if ( !loader.OpenSQL(sCountSQL.c_str()) ) {
        return -1;
    }

    if ( !loader.MoveFirst() ) {
        return false;
    }
    
    const int32_t nCount = loader.GetFieldInt32(L"RecordCount");
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

bool CAdoDotNetRecordsetImpl::SeekByString(const char *sIndex, const char *sValue)
{
    const std::wstring sIndexW = ds_str_conv::ConvertFromUTF8(sIndex);
    const std::wstring sValueW = ds_str_conv::ConvertFromUTF8(sValue);
    return this->SeekByString(sIndexW.c_str(), sValueW.c_str());
}

bool CAdoDotNetRecordsetImpl::SeekByString(const wchar_t *sIndex, const wchar_t *sValue)
{
    if ( _tcslen(sIndex) <= 0 ) {
        ASSERT(FALSE);
        return false;
    }

    if ( m_sTable.empty() ) {
        ASSERT(FALSE);
        return false;
    }

    if ( IsOpen() ) {
        m_pSet->Close();
    }

    std::wstring sFind;
    if (m_bSQLOpened) {
        sFind = L"SELECT * FROM (";
        sFind += m_sTable;
        sFind += L") AS TheMain WHERE ";
        sFind += sIndex;
        sFind += L" = ";
    }
    else {
        sFind = L"SELECT * FROM ";
        sFind += m_sTable;
        sFind += L" WHERE ";
        sFind += sIndex;
        sFind += L" = ";
    }

    if ( !m_pSet->SeekByString(sFind.c_str(), sValue) ) {
        return false;
    }

    return !m_pSet->IsEmpty();
}

bool CAdoDotNetRecordsetImpl::SeekByLong(const char *sIndex, int32_t nValue)      
{
    return SeekByLong(ds_str_conv::ConvertFromUTF8(sIndex).c_str(), nValue);
}

bool CAdoDotNetRecordsetImpl::SeekByLong(const wchar_t *sIndex, int32_t nValue)
{
    if ( _tcslen(sIndex) <= 0 ) {
        ASSERT(FALSE);
        return false;
    }

    if ( m_sTable.empty() ) {
        ASSERT(FALSE);
        return false;
    }

    if ( IsOpen() )
    {
        m_pSet->Close();
    }

    std::wstring sFind;
    if (m_bSQLOpened) {
        sFind = L"SELECT * FROM (";
        sFind += m_sTable;
        sFind += L") AS TheMain WHERE ";
        sFind += sIndex;
        sFind += L" = ";
        sFind += std::to_wstring(nValue);
    }
    else {
        sFind = L"SELECT * FROM ";
        sFind += m_sTable;
        sFind += L" WHERE ";
        sFind += sIndex;
        sFind += L" = ";
        sFind += std::to_wstring(nValue);
    }

    if ( !m_pSet->Open(sFind.c_str()) ) {
        return false;
    }

    return !m_pSet->IsEmpty();
}

std::wstring CAdoDotNetRecordsetImpl::GetFieldString(const wchar_t *sFieldName)
{
    return m_pSet->GetFieldString(sFieldName);
}

int32_t CAdoDotNetRecordsetImpl::GetFieldInt32(const char *sFieldName) 
{
    return GetFieldInt32(ds_str_conv::ConvertFromUTF8(sFieldName).c_str());
}

int32_t CAdoDotNetRecordsetImpl::GetFieldInt32(const wchar_t *sFieldName)
{
    return m_pSet->GetFieldLong(sFieldName);
}

void CAdoDotNetRecordsetImpl::SetFieldInt32(const char *sFieldName, int32_t lValue)
{
    SetFieldInt32(ds_str_conv::ConvertFromUTF8(sFieldName).c_str(), lValue);
}

void CAdoDotNetRecordsetImpl::SetFieldInt32(const wchar_t *sFieldName, int32_t lValue)
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

int CAdoDotNetRecordsetImpl::GetColumnCount() const 
{
    ASSERT(FALSE);
    return -1;
}

std::wstring CAdoDotNetRecordsetImpl::GetColumnName(int nCol) const 
{
    ASSERT(FALSE);
    return L"";
}

dsFieldType CAdoDotNetRecordsetImpl::GetColumnType(int nCol) const
{
    ASSERT(FALSE);
    return dsFieldType::Undefined;
}