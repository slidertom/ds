#include "StdAfx.h"
#include "dsTable.h"

#include "AbsDatabase.h"
#include "AbsRecordset.h"
#include "dsStrConv.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

dsTable::dsTable(dsDatabase *pDatabase, const wchar_t *sTableName)
:   m_pDatabase(pDatabase),
	m_sTableName(sTableName)
{
	ASSERT(m_pDatabase);
    ASSERT(m_pDatabase->m_pDatabase);
	m_pSet = m_pDatabase->m_pDatabase->CreateRecordset();
}

dsTable::~dsTable()
{
	delete m_pSet;
}

bool dsTable::Open()
{
	return m_pSet->Open(m_sTableName.c_str());
}

void dsTable::OpenSQL(const wchar_t *sSQL)
{
	m_pSet->OpenSQL(sSQL);
}

bool dsTable::OpenView(const wchar_t *sViewName)
{
    m_sTableName = sViewName; // view and table name indicated as same
                              // related with the ::Open realization
                              // DAO provides different objects for the View and Table.
	return m_pSet->OpenView(sViewName);
}

void dsTable::Flush()
{
    if ( !Open() ) {
		return;
	}

    m_pSet->Flush();
}

void dsTable::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, unsigned long nSize)
{
	m_pSet->SetFieldBinary(sFieldName, pData, nSize);
}

void dsTable::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, unsigned long &nSize) const
{
	m_pSet->GetFieldBinary(sFieldName, pData, nSize);
}

void dsTable::FreeBinary(unsigned char *pData)
{
	m_pSet->FreeBinary(pData);
}

bool dsTable::IsEOF() const	
{ 
	return m_pSet->IsEOF();
}

void dsTable::MoveNext()  
{ 
	m_pSet->MoveNext();                        
}

bool dsTable::MoveFirst() 
{ 
	VERIFY(Open());
	return m_pSet->MoveFirst(); 
}

COLORREF dsTable::GetFieldRGB(const wchar_t *sFieldName) const
{
	const std::wstring sRGB = GetFieldString(sFieldName);
	COLORREF color = _ttol(sRGB.c_str());
	return color;
}

void dsTable::SetFieldRGB(const wchar_t *sFieldName, COLORREF color)
{
	const std::wstring sRGB = std::to_wstring(color);
	m_pSet->SetFieldString(sFieldName, sRGB.c_str());
}

bool dsTable::SeekIndex(const wchar_t *sIndex, const wchar_t *sValue)
{
	VERIFY(Open());
	return m_pSet->SeekByString(sIndex, sValue);
}

bool dsTable::SeekIndex(const wchar_t *sIndex, int nValue)
{
	VERIFY(Open());
	return m_pSet->SeekByLong(sIndex, nValue);
}

bool dsTable::IsFieldValueNull(const wchar_t *sFieldName) const
{
	return m_pSet->IsFieldValueNull(sFieldName);
}

std::wstring dsTable::GetFieldString(const wchar_t *sFieldName) const 
{
	return m_pSet->GetFieldString(sFieldName);
}

void dsTable::SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) 
{
	m_pSet->SetFieldString(sFieldName, sValue);
}

int dsTable::GetFieldLong(const wchar_t *sFieldName) const 
{
	return m_pSet->GetFieldLong(sFieldName);
}

void dsTable::SetFieldLong(const wchar_t *sFieldName, int nValue) 
{
	m_pSet->SetFieldLong(sFieldName, nValue);
}

double dsTable::GetFieldDouble(const wchar_t *sFieldName) const 
{
	return m_pSet->GetFieldDouble(sFieldName);
}

void dsTable::SetFieldDouble(const wchar_t *sFieldName, double dValue)
{
	m_pSet->SetFieldDouble(sFieldName, dValue);
}

bool dsTable::GetFieldBool(const wchar_t *sFieldName) const 
{
	return m_pSet->GetFieldLong(sFieldName) != 0;
}

void dsTable::SetFieldBool(const wchar_t *sFieldName, bool bValue) 
{
	m_pSet->SetFieldLong(sFieldName, bValue ? 1 : 0);
}

time_t dsTable::GetFieldDateTime(const wchar_t *sFieldName) const 
{
	return m_pSet->GetFieldDateTime(sFieldName);
}

void dsTable::SetFieldDateTime(const wchar_t *sFieldName, time_t nValue)
{
	m_pSet->SetFieldDateTime(sFieldName, nValue);
}

void dsTable::SetFieldNull(const wchar_t *sFieldName) 
{
	m_pSet->SetFieldValueNull(sFieldName);
}

bool dsTable::Delete()	        
{ 
    return m_pSet->Delete();         
}

void dsTable::AddNew()
{ 
	VERIFY(Open());
	m_pSet->AddNew();
}

void dsTable::Edit()		    
{ 
    m_pSet->Edit();	               
}

bool dsTable::Update()	      
{ 
    return m_pSet->Update();         
}

int dsTable::GetColumnCount()
{
    VERIFY(Open());
    return m_pSet->GetColumnCount(); 
}

int dsTable::GetRecordCount()
{ 
    VERIFY(Open());
    return m_pSet->GetRecordCount(); 
}

std::wstring dsTable::GetColumnName(int nCol)
{
    VERIFY(Open());
    return m_pSet->GetColumnName(nCol); 
}

dsFieldType dsTable::GetColumnType(int nCol)
{
    VERIFY(Open());
    return m_pSet->GetColumnType(nCol); 
}

bool dsTable::DoesFieldExist(const wchar_t *sFieldName)
{
    VERIFY(Open());
	return m_pSet->DoesFieldExist(sFieldName);
}

std::wstring dsTable::GetUniqueTextFieldValue(const wchar_t *sFieldName, const wchar_t *sPrefix, int width)
{
	VERIFY(Open());
	
    std::wstring sFormat  = L"%s%0";
                 sFormat += std::to_wstring(width);
                 sFormat += L"d";
    
	int index = 1;
	while (true)
	{
        int sz = std::swprintf(nullptr, 0, sFormat.c_str(), sPrefix, index);
        std::vector<wchar_t> buf(sz + 1); // note +1 for null terminator
        std::swprintf(&buf[0], buf.size(), sFormat.c_str(), sPrefix, index);
        std::wstring sTemp(buf.begin(), buf.end());

		if ( !SeekIndex(sFieldName, sTemp.c_str()) ) {
			return sTemp;
		}
		++index;
	}	

    ASSERT(FALSE);
    return _T("");
}

dsDatabase *dsTable::GetDatabase() const  
{
    return m_pDatabase; 
}

bool dsTable::DeleteAllByIndex(const wchar_t *sField, const wchar_t *sValue)
{
    VERIFY(Open());
    return m_pSet->DeleteAllByStringValue(sField, sValue);
}

bool dsTable::DeleteAllByIndex(const wchar_t *sField, int nValue)
{
    VERIFY(Open());
    return m_pSet->DeleteAllByLongValue(sField, nValue);
}

bool dsTable::DeleteByIndex(const wchar_t *sField, const wchar_t *sValue)
{
    VERIFY(Open());
    return m_pSet->DeleteByStringValue(sField, sValue);
}

bool dsTable::DeleteByIndex(const wchar_t *sField, int nValue)
{
    VERIFY(Open());
    return m_pSet->DeleteByLongValue(sField, nValue);
}

const wchar_t *dsTable::GetTableName() const
{
    return m_sTableName.c_str();
}

std::string dsTable::GetFieldStringUTF8(const char *sFieldName) const
{
    return m_pSet->GetFieldStringUTF8(sFieldName);
}

void dsTable::SetFieldStringUTF8(const char *sFieldName, const char *sValue)
{
    return m_pSet->SetFieldStringUTF8(sFieldName, sValue);
}