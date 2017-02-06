#include "StdAfx.h"
#include "DaoRecordsetImpl.h"

#include "DaoDatabaseUtil.h"
#include "DaoDatabaseImpl.h"
#include "DaoBinaryFieldUtil.h"
#include "DaoErrorHandler.h"

#include "Collections/OleDateTimeUtil.h"

#include "../dsStrConv.h"

#include "afxdao.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
template <class TDaoDatabase>
inline void ChangeDBFieldName(TDaoDatabase *pDB, const wchar_t *sTableName, const wchar_t *oldFieldName, 
                                                 const wchar_t *newFieldName, const wchar_t *oldIndexName, const wchar_t *newIndexName)
{
	ASSERT(pDB);
	ASSERT(sTableName);
	ASSERT(oldFieldName);
	ASSERT(newFieldName);

	bool bIndexMode = (oldIndexName);
	CDaoTableDef td(pDB->Get());
	td.Open(sTableName);

	CDaoFieldInfo fieldInfo;
	CDaoIndexInfo indexInfo;
	td.GetFieldInfo(oldFieldName, fieldInfo, AFX_DAO_ALL_INFO);
	if (bIndexMode)
	{
		td.GetIndexInfo(oldIndexName, indexInfo, AFX_DAO_ALL_INFO);
	}

	fieldInfo.m_strName = newFieldName;
	if (bIndexMode)
	{
		indexInfo.m_pFieldInfos->m_strName = newFieldName;
		if ( newIndexName )
		{
			indexInfo.m_strName = newIndexName;
		}
	}

	td.CreateField(fieldInfo);
	if (bIndexMode)
	{
		td.DeleteIndex(oldIndexName);
		td.CreateIndex(indexInfo);
	}
	td.DeleteField(oldFieldName);
}
*/

CDaoRecordsetImpl::CDaoRecordsetImpl(CDaoDatabaseImpl *pDatabase, CDaoErrorHandler *pErrorHandler)
: m_pTableDef(nullptr), m_pErrorHandler(pErrorHandler), m_pSet(nullptr)
{
    m_pDatabase = pDatabase;
    ASSERT(m_pErrorHandler);
}

CDaoRecordsetImpl::~CDaoRecordsetImpl() 
{ 
	delete m_pTableDef;
	delete m_pSet;
}

void CDaoRecordsetImpl::SetIndex(const wchar_t *sIndex) 
{
	if ( m_sCurIndex == sIndex ) {
        return; // small optimization if index same
    }

    try {
	    m_pSet->SetCurrentIndex(sIndex);
    }
    catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SetIndex(index='";
		sMsg += sIndex;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
		return;
	}

	m_sCurIndex = sIndex;
}

bool CDaoRecordsetImpl::MoveNext() 
{
	try {
	    m_pSet->MoveNext();
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::MoveNext Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
		return false;
	}

	return true;
}

bool CDaoRecordsetImpl::MoveFirst() 
{
	ASSERT(m_pSet->IsOpen());
	try {
		m_pSet->MoveFirst();
	}
	catch (CDaoException *e) {
		e->Delete();
		return false; // we do return false if dataset is empty.
	}
	return true;
}

bool CDaoRecordsetImpl::Open(const wchar_t *sTableName)
{
	if ( IsOpen() ) {
		return true;
	}

	m_sCurIndex = _T("");

	try {
        delete m_pSet; // DAO can close m_pSet during database close, related with the 
        m_pSet = new CDaoRecordset(m_pDatabase->GetDaoDB());
		m_pSet->Open(dbOpenTable, sTableName);
	}
	catch (CDaoException *e) {
        std::wstring sDBName = m_pDatabase->GetName();
		std::wstring sMsg = L"CDaoRecordsetImpl::Open. Database: ";
		sMsg += sDBName.c_str();
		sMsg += L". Table='";
		sMsg += sTableName;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
		return false;
    }

	m_sCurTable = sTableName;

	return true;
}

bool CDaoRecordsetImpl::OpenSQL(const wchar_t *sSQL)
{
	ASSERT(!IsOpen());
	// OPEN TYPE       Snapshot  Dynaset  ForwardOnly  dbOpenTable
	// ADO Server        6.1      9.36      0.06          ----
	// DAO Network       0.09     0.05      ----          0.04
	// ADO Local         0.18     0.14      0.12          ----
	// DAO Local         0.08     0.03      ----          0.02  
	//                            ///\\\  
    
	try {
        delete m_pSet;
        m_pSet = new CDaoRecordset(m_pDatabase->GetDaoDB());
		m_pSet->Open(dbOpenDynaset, sSQL);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::OpenSQL(sql='";
		sMsg += sSQL;
		sMsg += L"')";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
		return false;
    }

	return true;
}

bool CDaoRecordsetImpl::OpenView(const wchar_t *sViewName)
{
	ASSERT(!IsOpen());
	delete m_pTableDef;
	m_pTableDef = nullptr;

	try {
        CDaoDatabase *pDB = m_pDatabase->GetDaoDB();
		m_pTableDef = new CDaoQueryDef(pDB);
		m_pTableDef->Open(sViewName);
        delete m_pSet;
        m_pSet = new CDaoRecordset(pDB);
		m_pSet->Open(m_pTableDef);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::OpenView(viewname='";
		sMsg += sViewName;
		sMsg += L"')";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
		return false;
	}

	return true;
}

bool CDaoRecordsetImpl::IsEOF()
{
	try {
        ASSERT(IsOpen());
		return m_pSet->IsEOF() != FALSE;
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::IsEOF Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return true;
}

bool CDaoRecordsetImpl::IsOpen() const
{
    if ( !m_pSet ) {
        return false;
    }

    // CDaoRecordset::IsOpen() does not throw any exception
	return m_pSet->IsOpen() != FALSE;
}

void CDaoRecordsetImpl::SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, unsigned long nSize)
{
	try {
		COleVariant var;
		VERIFY(dao_binary_field_util::PutBinaryIntoVariant(&var, pData, nSize));
		m_pSet->SetFieldValue(sFieldName, var);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldBinary(field='";
		const char *pDat = (char *)pData;
		sMsg += sFieldName;
		sMsg += L"', data=0x";
		sMsg += ds_str_conv::ConvertFromUTF8(pDat);
		sMsg += L", size=";
		sMsg += std::to_wstring(nSize);
		sMsg += L") Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
}

void CDaoRecordsetImpl::GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, unsigned long &nSize)
{
	try {
		*pData = NULL;
		nSize = 0;
	
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		if (var.vt == VT_NULL ) {
			return;
		}

		VERIFY(dao_binary_field_util::GetBinaryFromVariant(var, pData, &nSize));
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetFieldBinary(field='";
		sMsg += sFieldName;
		sMsg += L"', ...) Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

void CDaoRecordsetImpl::FreeBinary(unsigned char *pData)
{
    delete [] pData; //#24398 - originaly was free(pData), but it is created with new[]
}

bool CDaoRecordsetImpl::Delete()
{
	try {
		m_pSet->Delete();
	}
	catch(CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::Delete Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
		return false;
	}
	return true;
}

void CDaoRecordsetImpl::AddNew()
{
    try {
	    m_pSet->AddNew();
    }
    catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::AddNew Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
        e->Delete();
    }
}

void CDaoRecordsetImpl::Edit()
{
    try {
	    m_pSet->Edit();
    }
    catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::Edit Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
        e->Delete();
    }
}

bool CDaoRecordsetImpl::Update()
{
	try {
		m_pSet->Update();
	}
    catch (CDaoException *e) {
        std::wstring sMsg = L"CDaoRecordsetImpl::Update Table='";
                     sMsg += m_sCurTable;
                     sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
        e->Delete();
		return false;
    }

	return true;
}

long CDaoRecordsetImpl::GetRecordCount()
{
    try {
	    return m_pSet->GetRecordCount();
    }
    catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetRecordCount Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
		DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
    }
    return 0;
}

void CDaoRecordsetImpl::SetFieldValueNull(const wchar_t *sFieldName)
{
	try {
		m_pSet->SetFieldValueNull(sFieldName);
	}
	catch (CDaoException *e)
	{
		std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldValueNull(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

bool CDaoRecordsetImpl::DoesFieldExist(const wchar_t *sFieldName) 
{
    ASSERT(_tcslen(sFieldName) != 0);

    try {
		CDaoFieldInfo fieldinfo;
		m_pSet->GetFieldInfo(sFieldName, fieldinfo);
	}
	catch(CDaoException *e) {
		// Item not found in this collection
		e->Delete();
		return false;
	}

	return true;
}

// building variants
static inline COleVariant MakeVariant(const wchar_t *strSrc) {
    if (strSrc == NULL || strSrc[0] == _T('\0')) {
		return COleVariant();
    }
    else {
		return COleVariant(strSrc, VT_BSTRT);
    }
}
bool CDaoRecordsetImpl::SeekByString(const wchar_t *sIndex, const wchar_t *sValue)
{
	SetIndex(sIndex);

	try {
		return m_pSet->Seek(_T("="), &MakeVariant(sValue)) != FALSE;
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SeekByString(index='";
		sMsg += sIndex;
		sMsg += L"', value='";
		sMsg += sValue;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return false;
}

bool CDaoRecordsetImpl::SeekByLong(const wchar_t *sIndex, long nValue)
{
	SetIndex(sIndex);

	try {
		return m_pSet->Seek(_T("="), &COleVariant(nValue)) != FALSE;
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SeekByLong(index='";
		sMsg += sIndex;
		sMsg += L"', value='";
		sMsg += std::to_wstring(nValue);
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return false;
}

std::wstring CDaoRecordsetImpl::GetFieldString(const wchar_t *sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetString(var).c_str();
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetFieldString(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return std::wstring(_T(""));
}

void CDaoRecordsetImpl::SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, ::MakeVariant(sValue));
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldString(field='";
		sMsg += sFieldName;
		sMsg += L"', value='";
		sMsg += sValue;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
}

long CDaoRecordsetImpl::GetFieldLong(const wchar_t *sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetInt(var);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetFieldLong(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
	
	return 0;
}

void CDaoRecordsetImpl::SetFieldLong(const wchar_t *sFieldName, long lValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, lValue);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldLong(field='";
		sMsg += sFieldName;
		sMsg += L"', value='";
		sMsg += std::to_wstring(lValue);
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

double CDaoRecordsetImpl::GetFieldDouble(const wchar_t *sFieldName) 
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetDouble(var); 
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetFieldDouble(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return 0.0;
}

void CDaoRecordsetImpl::SetFieldDouble(const wchar_t *sFieldName, double dValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, dValue);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldDouble(field='";
		sMsg += sFieldName;
		sMsg += L"', value='";
		sMsg += std::to_wstring(dValue);
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

template <class TOleVariant>
static inline COleDateTime GetDateTime(const TOleVariant &varSrc) {
	LPCVARIANT pSrc = (LPCVARIANT)varSrc;
	switch (pSrc->vt)
	{
	case VT_DATE:
		return COleDateTime(varSrc);
	}
	COleDateTime dt;
	dt.SetStatus (COleDateTime::null);
	return dt;
}

time_t CDaoRecordsetImpl::GetFieldDateTime(const wchar_t *sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		COleDateTime time = ::GetDateTime(var);
		return ::VariantToTime(time);
	}
	catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::GetFieldDateTime(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return time_t(-1);
}

void CDaoRecordsetImpl::SetFieldDateTime(const wchar_t *sFieldName, const time_t &time)
{
	if (time != -1)
	{
		try {
			COleDateTime var(time);
			m_pSet->SetFieldValue(sFieldName, var);
		}
		catch (CDaoException *e) {
			std::wstring sMsg = L"CDaoRecordsetImpl::SetFieldDateTime(field='";
			sMsg += sFieldName;
			sMsg += L"', time=";
			sMsg += std::to_wstring(time);
			sMsg += L"') Table='";
			sMsg += m_sCurTable;
			sMsg += L"'";
            DoOnDaoException(e, sMsg.c_str());
			ASSERT(FALSE);
			e->Delete();
		}
	}
	else {
		SetFieldValueNull(sFieldName);
	}
}

bool CDaoRecordsetImpl::IsFieldValueNull(const wchar_t *sFieldName)
{
	ASSERT(_tcslen(sFieldName) != 0);
    try {
	    COleVariant var;
	    m_pSet->GetFieldValue(sFieldName, var);
	    return (var.vt == VT_NULL);
    }
    catch (CDaoException *e) {
		std::wstring sMsg = L"CDaoRecordsetImpl::IsFieldValueNull(field='";
		sMsg += sFieldName;
		sMsg += L"') Table='";
		sMsg += m_sCurTable;
		sMsg += L"'";
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
    }
    return true;
}

void CDaoRecordsetImpl::DoOnDaoException(CDaoException *e, const wchar_t *sFunction)
{
    ASSERT(m_pErrorHandler);
    m_pErrorHandler->OnDaoException(e, sFunction);
}

std::string CDaoRecordsetImpl::GetFieldStringUTF8(const char *sFieldName)
{
    const std::wstring sValue = GetFieldString(ds_str_conv::ConvertFromUTF8(sFieldName).c_str());
    return ds_str_conv::ConvertToUTF8(sValue.c_str());
}

void CDaoRecordsetImpl::SetFieldStringUTF8(const char *sFieldName, const char *sValue)
{
    SetFieldString(ds_str_conv::ConvertFromUTF8(sFieldName).c_str(), ds_str_conv::ConvertFromUTF8(sValue).c_str());
}
