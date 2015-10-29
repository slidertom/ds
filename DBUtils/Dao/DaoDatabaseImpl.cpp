#include "StdAfx.h"
#include "DaoDatabaseImpl.h"

#include "DaoRecordsetImpl.h"
#include "DAOExtensions.h"
#include "DaoErrorHandler.h"

#include "afxdao.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace inernal_dao_file_utils
{
    inline bool DoesFileExist(LPCTSTR fileName) {
	    return (GetFileAttributes(fileName) != 0xffffffff);
    }

    inline void RemoveFile(LPCTSTR fileName) {
        ::_tremove(fileName);
    }

    inline bool CopyFile(LPCTSTR sSrcFile, LPCTSTR sDstFile) {
        return ::CopyFile(sSrcFile, sDstFile, FALSE) != FALSE;
    }
};

CDaoDatabaseImpl::CDaoDatabaseImpl() 
: m_bReadOnly(false)
{
    m_pDatabase = new CDaoDatabase;
    m_pErrorHandler = new CDaoErrorHandler;
}

CDaoDatabaseImpl::~CDaoDatabaseImpl()
{
    delete m_pDatabase;
    delete m_pErrorHandler;
}

bool CDaoDatabaseImpl::IsDaoDB(LPCTSTR sPath)
{
    FILE *pFile = _tfopen(sPath, _T("rb"));
    if ( !pFile ) {
		return false;
	}
 //  Standard Jet DB
    
    bool bDao = false;
    char buffer[16];
    int nRead = fread(buffer, 1, 16, pFile);
    if ( nRead > 15 && 
         buffer[4]  == 'S' && 
         buffer[5]  == 't' && 
         buffer[6]  == 'a' && 
         buffer[7]  == 'n' && 
         buffer[8]  == 'd' && 
         buffer[9]  == 'a' &&
         buffer[10] == 'r' && 
         buffer[11] == 'd' && 
         buffer[13] == 'J' && 
         buffer[14] == 'e' &&
         buffer[15] == 't' ) 
    {
        bDao = true;
    }

    fclose(pFile);

    return bDao;
}

bool CDaoDatabaseImpl::BeginTrans() 
{
	try {
		m_pDatabase->m_pWorkspace->BeginTrans();
	}
	catch (CDaoException *e) {
		ASSERT(FALSE); // should not happen
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::BeginTrans"));
		e->Delete();
		return false;
	}

	return true;
}

bool CDaoDatabaseImpl::CommitTrans() 
{
	try {
		m_pDatabase->m_pWorkspace->CommitTrans();
	}
	catch (CDaoException *e) {
		ASSERT(FALSE); // should not happen
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::CommitTrans"));
		e->Delete();
		return false;
	}

	return true;
}

bool CDaoDatabaseImpl::Rollback()   
{
	try {
		m_pDatabase->m_pWorkspace->Rollback();
	}
	catch (CDaoException *e) {
		ASSERT(FALSE); // should not happen
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::Rollback"));
		e->Delete();
		return false;
	}

	return true;
}

bool CDaoDatabaseImpl::Execute(LPCTSTR lpszSQL) 
{
    try {
	    m_pDatabase->Execute(lpszSQL);
    } 
    catch (CDaoException *e) {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::Execute"));
        e->Delete();
        return false;
    }
    return true;
}

void CDaoDatabaseImpl::Close() 
{
	try {
		m_pDatabase->Close();
	}
	catch (CDaoException *e) {   
		ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::Close"));
		e->Delete();
	}
}

bool CDaoDatabaseImpl::OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) 
{
	m_bReadOnly = bReadOnly;

	CStdString sConnect;
	if ( _tcslen(szPsw) > 0 ) {
		sConnect.Format(_T(";PWD=%s;"), szPsw);
	}

	try {
		m_pDatabase->Open(sPath, FALSE, bReadOnly, sConnect.c_str());
	}
	catch (CDaoException *e)
    {
        // TODO: do check is it dao database before open
        //ASSERT(FALSE);
        //m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::OpenDB"));
		e->Delete();
		return false;
	}
	
	return true;
}

dsDBType CDaoDatabaseImpl::GetType()
{
	return dsType_Dao;
}

bool CDaoDatabaseImpl::IsReadOnly() const
{
	return m_bReadOnly; 
}

bool CDaoDatabaseImpl::IsOpen() const
{
	return m_pDatabase->IsOpen() != FALSE;
}

CStdString CDaoDatabaseImpl::GetName()
{
    CString sName = m_pDatabase->GetName();
	const CStdString str = sName;
    TRACE(str.c_str());TRACE(_T("\n"));
    return str;
}

bool CDaoDatabaseImpl::DoesTableExist(LPCTSTR sTable)
{
	bool bTableExists = false;
	try
	{
		CDaoTableDefInfo tabInfo;
		int nTableCount = m_pDatabase->GetTableDefCount();
		for (int i1 = 0; i1 < nTableCount; ++i1)
		{
			m_pDatabase->GetTableDefInfo(i1, tabInfo);
			if (tabInfo.m_strName.CompareNoCase(sTable) == 0)
			{
				bTableExists = true;
				break;
			}
		}
	}
	catch (CDaoException *e)
	{
		ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::DoesTableExist"));
		e->Delete();
	}

	return bTableExists;
}

CAbsRecordset *CDaoDatabaseImpl::CreateRecordset()
{
	return new CDaoRecordsetImpl(this, m_pErrorHandler);
}

void CDaoDatabaseImpl::CommitDatabase()
{
    ASSERT(IsOpen());
    ASSERT(m_pDatabase);

    // #21469
    // Howto test:
    // 1. do open project
    // 2. do swicth GI
    // 3. do assign pane content
    // 4. control+S 
    // 5. do close app -> save -> NO
    // do open app, do open project -> pane content is not stored if database was not been closed.

    // NOTE: we do not call dsDatabase::Close(), to avoid m_listners call -> close.
    
    const bool bReadOnly   = this->IsReadOnly();
    const CStdString sName = m_pDatabase->GetName();
    
    m_pDatabase->Close(); // we do not close locker
    delete m_pDatabase;
    m_pDatabase = new CDaoDatabase;
	this->OpenDB(sName.c_str(), bReadOnly, _T(""));
}

bool CDaoDatabaseImpl::CompactDatabase()
{
    const bool bReadOnly   = this->IsReadOnly();
    const CStdString sName = this->GetName();
    TRACE(sName.c_str());TRACE(_T("\n"));

    try
    {
        // Make sure you're passing the address of a path to a different target file than the one you're compacting. 
        CStdString sTemp = sName.c_str();
		sTemp += _T("-temp");
        if ( inernal_dao_file_utils::DoesFileExist(sTemp.c_str()) ) {
		    inernal_dao_file_utils::RemoveFile(sTemp.c_str()); // just in case remove any garbage
        }

        m_pDatabase->Close(); 
        delete m_pDatabase;

        CDaoWorkspace daoWorkspace;
        daoWorkspace.CompactDatabase(sName.c_str(), sTemp.c_str(), dbLangGeneral);

        if ( !inernal_dao_file_utils::CopyFile(sTemp.c_str(), sName.c_str()) ) {
			ASSERT(FALSE);
            CStdString sError;
            sError.Format(_T("inernal_dao_file_utils::CopyFile failed. From: %s to %s."), sTemp.c_str(), sName.c_str());
            m_pErrorHandler->OnError(sError.c_str(), _T("CDaoDatabaseImpl::CompactDatabase()"));
			return false;
		}
	
        inernal_dao_file_utils::RemoveFile(sTemp.c_str());

        m_pDatabase = new CDaoDatabase;
	    this->OpenDB(sName.c_str(), bReadOnly, _T(""));
    }
    catch (CDaoException *e)
    {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::CompactDatabase"));
        e->Delete();    
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst)
{
	ASSERT(pDstDB);
    CDaoDatabaseImpl *pDBDestImpl = dynamic_cast<CDaoDatabaseImpl *>(pDstDB);
    ASSERT(pDBDestImpl);

	CDaoDatabase *pDstDBDao = pDBDestImpl->m_pDatabase;

	ASSERT(m_pDatabase);
	ASSERT(pDstDBDao);

	try {
		dao_extensions::CopyTableData(m_pDatabase, pDstDBDao, sTableNameSrc, sTableNameDst, m_pErrorHandler);
	}
    catch (CDaoException *e) {
        ASSERT(FALSE);
		m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::CopyTableData"));
        CStdString sMsg;
        sMsg.Format(_T("CopyTableData From: %s To %s."), sTableNameSrc, sTableNameDst);
        m_pErrorHandler->OnError(sMsg.c_str(), _T("CDaoDatabaseImpl::CopyTableData"));
		e->Delete();
		return false;
    }

	return true;
}

CDaoDatabaseImpl::dbErrorHandler CDaoDatabaseImpl::SetErrorHandler(CDaoDatabaseImpl::dbErrorHandler newHandler)
{
    ASSERT(m_pErrorHandler);
    return m_pErrorHandler->SetErrorHandler(newHandler);
}

void CDaoDatabaseImpl::DeleteRelation(LPCTSTR sRelation)
{
	try
	{
		m_pDatabase->DeleteRelation(sRelation);
	}
	catch (CDaoException *e) {
		e->Delete();// No info for user
	}
}

bool CDaoDatabaseImpl::CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
									  LPCTSTR sField, LPCTSTR sForeignField)
{
	ASSERT(m_pDatabase);

	try
	{
		m_pDatabase->CreateRelation(sName, sTable, sForeignTable, lAttr, sField, sForeignField);
	}
    catch (CDaoException *e) {
        ASSERT(FALSE);
		m_pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::CreateRelation"));
		e->Delete();
		return false;
    }

	return true;
}

bool CDaoDatabaseImpl::GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info)
{
    ASSERT(m_pDatabase);
    CDaoTableDef tableInfo(m_pDatabase);
    tableInfo.Open(sTable);

    if ( !tableInfo.IsOpen() ) 
    {
        CStdString sError;
        sError.Format(_T("GetTableFieldInfo failed. Table %s."), sTable);
        m_pErrorHandler->OnError(sError.c_str(), _T("CDaoDatabaseImpl::GetTableFieldInfo"));
        return false;
    }

    CDaoFieldInfo fieldInfo;
    
    short nFields = tableInfo.GetFieldCount();
    for(short i = 0; i < nFields; ++i) 
    {
        tableInfo.GetFieldInfo(i, fieldInfo);
        const short nType = fieldInfo.m_nType;
        dsFieldType field_type = dsFieldType_Undefined;

        switch (nType) 
        {
        case dbBoolean:
            field_type = dsFieldType_Long;
            break;
        case dbByte:
            field_type = dsFieldType_Long;
            break;
        case dbInteger:
            field_type = dsFieldType_Long;
            break;
        case dbLong:
            field_type = dsFieldType_Long;
            break;
        case dbCurrency:
            field_type = dsFieldType_Double;
            break;
        case dbSingle:
            field_type = dsFieldType_Long;
            break;
        case dbDouble:
            field_type = dsFieldType_Double;
            break;
        case dbDate:
            field_type = dsFieldType_DateTime;
            break;
        case dbText:
            field_type = dsFieldType_Text;
            break;
        case dbLongBinary:
            field_type = dsFieldType_Binary;
            break;
        case dbMemo:
            field_type = dsFieldType_Text;
            break;
        case dbGUID:
            field_type = dsFieldType_Text;
            break;
        default:
            {
                CStdString sError;
                sError.Format(_T("Field type: %d is undefined."), nType);
                m_pErrorHandler->OnError(sError.c_str(), _T("CDaoDatabaseImpl::GetTableFieldInfo()"));
                ASSERT(FALSE);
            }
            break;
        }

        CStdString sName = fieldInfo.m_strName;
        info[sName] = field_type;
    }

    tableInfo.Close();

    return true;
}