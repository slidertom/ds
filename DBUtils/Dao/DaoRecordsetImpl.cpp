#include "StdAfx.h"
#include "DaoRecordsetImpl.h"

#include "DaoDatabaseUtil.h"
#include "DaoDatabaseImpl.h"
#include "DaoBinaryFieldUtil.h"
#include "DaoErrorHandler.h"

#include "Collections/OleDateTimeUtil.h"

#include "afxdao.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
template <class TDaoDatabase>
inline void ChangeDBFieldName(TDaoDatabase *pDB, LPCTSTR sTableName, LPCTSTR oldFieldName, 
                                                 LPCTSTR newFieldName, LPCTSTR oldIndexName, LPCTSTR newIndexName)
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

void CDaoRecordsetImpl::SetIndex(LPCTSTR sIndex) 
{
	if ( m_sCurIndex == sIndex ) {
        return; // small optimization if index same
    }

    try {
	    m_pSet->SetCurrentIndex(sIndex);
    }
    catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetIndex(index='%s') Table='%s'"), sIndex, m_sCurTable);
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
		CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::MoveNext Table='%s'"), m_sCurTable);
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

bool CDaoRecordsetImpl::Open(LPCTSTR sTableName)
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
        CStdString sDBName = m_pDatabase->GetName();
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::Open(table='%s'). Database: %s. Table='%s'"), sDBName.c_str(), sTableName);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
		return false;
    }

	m_sCurTable = sTableName;

	return true;
}

bool CDaoRecordsetImpl::OpenSQL(LPCTSTR sSQL)
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::OpenSQL(sql='%s')"), sSQL);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
		return false;
    }

	return true;
}

bool CDaoRecordsetImpl::OpenView(LPCTSTR sViewName)
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::OpenView(viewname='%s')"), sViewName);
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::IsEOF Table='%s'"), m_sCurTable);
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

void CDaoRecordsetImpl::SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)
{
	try {
		COleVariant var;
		VERIFY(dao_binary_field_util::PutBinaryIntoVariant(&var, pData, nSize));
		m_pSet->SetFieldValue(sFieldName, var);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetFieldBinary(field='%s', data=0x%X, size=%d) Table='%s'"), sFieldName, pData, nSize, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
}

void CDaoRecordsetImpl::GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize)
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetFieldBinary(field='%s', ...) Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

bool CDaoRecordsetImpl::Delete()
{
	try {
		m_pSet->Delete();
	}
	catch(CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::Delete Table='%s'"), m_sCurTable);
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::AddNew Table='%s'"), m_sCurTable);
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::Edit Table='%s'"), m_sCurTable);
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::Update Table='%s'"), m_sCurTable);
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
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetRecordCount Table='%s'"), m_sCurTable);
		DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
    }
    return 0;
}

void CDaoRecordsetImpl::SetFieldValueNull(LPCTSTR sFieldName)
{
	try {
		m_pSet->SetFieldValueNull(sFieldName);
	}
	catch (CDaoException *e)
	{
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetFieldValueNull(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

bool CDaoRecordsetImpl::DoesFieldExist(LPCTSTR sFieldName) 
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
static inline COleVariant MakeVariant(LPCTSTR strSrc) {
    if (strSrc == NULL || strSrc[0] == _T('\0')) {
		return COleVariant();
    }
    else {
		return COleVariant(strSrc, VT_BSTRT);
    }
}
bool CDaoRecordsetImpl::SeekByString(LPCTSTR sIndex, LPCTSTR sValue)
{
	SetIndex(sIndex);

	try {
		return m_pSet->Seek(_T("="), &MakeVariant(sValue)) != FALSE;
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SeekByString(index='%s', value='%s') Table='%s'"), sIndex, sValue, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return false;
}

bool CDaoRecordsetImpl::SeekByLong(LPCTSTR sIndex, long nValue)
{
	SetIndex(sIndex);

	try {
		return m_pSet->Seek(_T("="), &COleVariant(nValue)) != FALSE;
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SeekByLong(index='%s', value=%d) Table='%s'"), sIndex, nValue, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return false;
}

CStdString CDaoRecordsetImpl::GetFieldString(LPCTSTR sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetString(var);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetFieldString(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return CStdString(_T(""));
}

void CDaoRecordsetImpl::SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, ::MakeVariant(sValue));
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetFieldString(field='%s', value='%s') Table='%s'"), sFieldName, sValue, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
}

long CDaoRecordsetImpl::GetFieldLong(LPCTSTR sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetInt(var);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetFieldLong(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
		e->Delete();
	}
	
	return 0;
}

void CDaoRecordsetImpl::SetFieldLong(LPCTSTR sFieldName, long lValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, lValue);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetFieldLong(field='%s', value=%d) Table='%s'"), sFieldName, lValue, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}
}

double CDaoRecordsetImpl::GetFieldDouble(LPCTSTR sFieldName) 
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		return dao_database_util::GetDouble(var); 
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetFieldDouble(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return 0.0;
}

void CDaoRecordsetImpl::SetFieldDouble(LPCTSTR sFieldName, double dValue)
{
	try {
		m_pSet->SetFieldValue(sFieldName, dValue);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::SetFieldDouble(field='%s', value=%f) Table='%s'"), sFieldName, dValue, m_sCurTable);
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

time_t CDaoRecordsetImpl::GetFieldDateTime(LPCTSTR sFieldName)
{
	try {
		COleVariant var;
		m_pSet->GetFieldValue(sFieldName, var);
		COleDateTime time = ::GetDateTime(var);
		return ::VariantToTime(time);
	}
	catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::GetFieldDateTime(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
		ASSERT(FALSE);
		e->Delete();
	}

	return time_t(-1);
}

void CDaoRecordsetImpl::SetFieldDateTime(LPCTSTR sFieldName, const time_t &time)
{
	if (time != -1)
	{
		try {
			COleDateTime var(time);
			m_pSet->SetFieldValue(sFieldName, var);
		}
		catch (CDaoException *e) {
            CStdString sMsg;
            sMsg.Format(_T("CDaoRecordsetImpl::SetFieldDateTime(field='%s', time=%d) Table='%s'"), sFieldName, time, m_sCurTable);
            DoOnDaoException(e, sMsg.c_str());
			ASSERT(FALSE);
			e->Delete();
		}
	}
	else {
		SetFieldValueNull(sFieldName);
	}
}

bool CDaoRecordsetImpl::IsFieldValueNull(LPCTSTR sFieldName)
{
	ASSERT(_tcslen(sFieldName) != 0);
    try {
	    COleVariant var;
	    m_pSet->GetFieldValue(sFieldName, var);
	    return (var.vt == VT_NULL);
    }
    catch (CDaoException *e) {
        CStdString sMsg;
        sMsg.Format(_T("CDaoRecordsetImpl::IsFieldValueNull(field='%s') Table='%s'"), sFieldName, m_sCurTable);
        DoOnDaoException(e, sMsg.c_str());
        ASSERT(FALSE);
        e->Delete();
    }
    return true;
}

void CDaoRecordsetImpl::DoOnDaoException(CDaoException *e, LPCTSTR sFunction)
{
    ASSERT(m_pErrorHandler);
    m_pErrorHandler->OnDaoException(e, sFunction);
}
