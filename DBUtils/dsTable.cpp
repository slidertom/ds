#include "StdAfx.h"
#include "dsTable.h"

#include "AbsDatabase.h"
#include "AbsRecordset.h"

#include "Collections/StdString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

dsTable::dsTable(dsDatabase *pDatabase, LPCTSTR sTableName)
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

void dsTable::OpenSQL(LPCTSTR sSQL)
{
	m_pSet->OpenSQL(sSQL);
}

bool dsTable::OpenView(LPCTSTR sViewName)
{
    m_sTableName = sViewName; // view and table name indicated as same
                              // related with the ::Open realization
                              // DAO provides different objects for the View and Table.
	return m_pSet->OpenView(sViewName);
}

void dsTable::Flush()
{
    if(!Open())
	{
		return;
	}

    m_pSet->Flush();
}

void dsTable::SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)
{
	m_pSet->SetFieldBinary(sFieldName, pData, nSize);
}

void dsTable::GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize) const
{
	m_pSet->GetFieldBinary(sFieldName, pData, nSize);
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

COLORREF dsTable::GetFieldRGB(LPCTSTR sFieldName) const
{
	const CStdString sRGB = GetFieldString(sFieldName);
	COLORREF color = _ttol(sRGB.c_str());
	return color;
}

void dsTable::SetFieldRGB(LPCTSTR sFieldName, COLORREF color)
{
	CStdString sRGB;
	sRGB.Format(_T("%d"), color);
	m_pSet->SetFieldString(sFieldName, sRGB.c_str());
}

bool dsTable::SeekIndex(LPCTSTR sIndex, LPCTSTR sValue)
{
	VERIFY(Open());
	return m_pSet->SeekByString(sIndex, sValue);
}

bool dsTable::SeekIndex(LPCTSTR sIndex, long nValue)
{
	VERIFY(Open());
	return m_pSet->SeekByLong(sIndex, nValue);
}

bool dsTable::IsFieldValueNull(LPCTSTR sFieldName) const
{
	return m_pSet->IsFieldValueNull(sFieldName);
}

CStdString dsTable::GetFieldString(LPCTSTR sFieldName) const 
{
	return m_pSet->GetFieldString(sFieldName);
}

void dsTable::SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue) 
{
	m_pSet->SetFieldString(sFieldName, sValue);
}

long dsTable::GetFieldLong(LPCTSTR sFieldName) const 
{
	return m_pSet->GetFieldLong(sFieldName);
}

void dsTable::SetFieldLong(LPCTSTR sFieldName, long nValue) 
{
	m_pSet->SetFieldLong(sFieldName, nValue);
}

double dsTable::GetFieldDouble(LPCTSTR sFieldName) const 
{
	return m_pSet->GetFieldDouble(sFieldName);
}

void dsTable::SetFieldDouble(LPCTSTR sFieldName, double dValue)
{
	m_pSet->SetFieldDouble(sFieldName, dValue);
}

bool dsTable::GetFieldBool(LPCTSTR sFieldName) const 
{
	return m_pSet->GetFieldLong(sFieldName) != 0;
}

void dsTable::SetFieldBool(LPCTSTR sFieldName, bool bValue) 
{
	m_pSet->SetFieldLong(sFieldName, bValue ? 1 : 0);
}

time_t dsTable::GetFieldDateTime(LPCTSTR sFieldName) const 
{
	return m_pSet->GetFieldDateTime(sFieldName);
}

void dsTable::SetFieldDateTime(LPCTSTR sFieldName, time_t nValue)
{
	m_pSet->SetFieldDateTime(sFieldName, nValue);
}

void dsTable::SetFieldNull(LPCTSTR sFieldName) 
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

long dsTable::GetRecordCount()
{ 
    VERIFY(Open());
    return m_pSet->GetRecordCount(); 
}

bool dsTable::DoesFieldExist(LPCTSTR sFieldName)
{
    VERIFY(Open());
	return m_pSet->DoesFieldExist(sFieldName);
}

CStdString dsTable::GetUniqueTextFieldValue(LPCTSTR sFieldName, LPCTSTR sPrefix, int width)
{
	VERIFY(Open());
	
    CStdString sTemp;
	int index = 1;

	CStdString sFormat;
	sFormat.Format(_T("%%s%%0%dd"), width);

	while (true)
	{
		sTemp.Format(sFormat, sPrefix, index);
		if ( !SeekIndex(sFieldName, sTemp) ) {
			return sTemp;
		}
		index++;
	}	

    ASSERT(FALSE);
    return _T("");
}

dsDatabase *dsTable::GetDatabase() const  
{
    return m_pDatabase; 
}

bool dsTable::DeleteAllByIndex(LPCTSTR sField, LPCTSTR sValue)
{
    VERIFY(Open());
    return m_pSet->DeleteAllByStringValue(sField, sValue);
}

bool dsTable::DeleteAllByIndex(LPCTSTR sField, long nValue)
{
    VERIFY(Open());
    return m_pSet->DeleteAllByLongValue(sField, nValue);
}

bool dsTable::DeleteByIndex(LPCTSTR sField, LPCTSTR sValue)
{
    VERIFY(Open());
    return m_pSet->DeleteByStringValue(sField, sValue);
}

bool dsTable::DeleteByIndex(LPCTSTR sField, long nValue)
{
    VERIFY(Open());
    return m_pSet->DeleteByLongValue(sField, nValue);
}

LPCTSTR dsTable::GetTableName() const
{
    return m_sTableName.c_str();
}