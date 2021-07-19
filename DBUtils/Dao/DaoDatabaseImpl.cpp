#include "stdafx.h"
#include "DaoDatabaseImpl.h"

#include "DaoRecordsetImpl.h"
#include "DAOExtensions.h"
#include "DaoErrorHandler.h"
#include "DaoFileUtil.h"

#include "../dsStrConv.h"
#include "../dsOpenParams.h"
#include "afxdao.h"

#include "string"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace inernal_dao_file_utils
{
    static inline bool DoesFileExist(const wchar_t *fileName) {
        return (::GetFileAttributes(fileName) != 0xffffffff);
    }

    static inline void RemoveFile(const wchar_t *fileName) {
        ::_wremove(fileName);
    }

    static inline bool CopyFile(const wchar_t *sSrcFile, const wchar_t *sDstFile) {
        return ::CopyFile(sSrcFile, sDstFile, FALSE) != FALSE;
    }
};

CDaoDatabaseImpl::CDaoDatabaseImpl() 
: m_bReadOnly(false)
{
    m_pDatabase     = new CDaoDatabase;
    m_pErrorHandler = new CDaoErrorHandler;
}

CDaoDatabaseImpl::~CDaoDatabaseImpl()
{
    // CDaoDatabase::~CDaoDatabase()
    // does this:
    // if (IsOpen())
    //        Close();
    // custom implementation to avoid exceptions:
    if ( IsOpen() ) {
        Close();
    }

    delete m_pDatabase;
    delete m_pErrorHandler;
}

bool CDaoDatabaseImpl::IsDaoDB(const wchar_t *sPath)
{
    return CDaoFileUtil::IsDaoDB(sPath);
}

bool CDaoDatabaseImpl::BeginTrans() 
{
    try {
        m_pDatabase->m_pWorkspace->BeginTrans();
    }
    catch (CDaoException *e) {
        ASSERT(FALSE); // should not happen
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::BeginTrans");
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
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::CommitTrans");
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
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::Rollback");
        e->Delete();
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::Execute(const char *sSQL) {
    const std::wstring sSQLUTF16 = ds_str_conv::ConvertFromUTF8(sSQL);
    return Execute(sSQLUTF16.c_str());
}

bool CDaoDatabaseImpl::Execute(const wchar_t *lpszSQL) 
{
    try {
        m_pDatabase->Execute(lpszSQL);
    } 
    catch (CDaoException *e) {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::Execute");
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
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::Close");
        e->Delete();
    }
}

bool CDaoDatabaseImpl::OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) 
{
    m_bReadOnly = open_params.m_bReadOnly;

    std::wstring sConnect;
    if ( ::wcslen(open_params.m_sKey.c_str()) > 0 ) {
        sConnect = L";PWD=";
        sConnect += open_params.m_sKey;
        sConnect += L";";
    }

    try {
        m_pDatabase->Open(sPath, FALSE, open_params.m_bReadOnly, sConnect.c_str());
    }
    catch (CDaoException *e)
    {
        // TODO: do check is it dao database before open
        //ASSERT(FALSE);
        //m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::OpenDB");
        e->Delete();
        return false;
    }
    
    return true;
}

dsDBType CDaoDatabaseImpl::GetType()
{
    return dsDBType::Dao;
}

bool CDaoDatabaseImpl::IsReadOnly() const
{
    return m_bReadOnly; 
}

bool CDaoDatabaseImpl::IsOpen() const
{
    return m_pDatabase->IsOpen() != FALSE;
}

std::wstring CDaoDatabaseImpl::GetName()
{
    const std::wstring sName = m_pDatabase->GetName();
    return sName;
}

bool CDaoDatabaseImpl::DoesTableExistUTF8(const char *sTable)
{
    std::wstring sTableUTF16 = ds_str_conv::ConvertFromUTF8(sTable);
    return DoesTableExist(sTableUTF16.c_str());
}

bool CDaoDatabaseImpl::DoesTableExist(const wchar_t *sTable)
{
    bool bTableExists = false;
    try {
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
    catch (CDaoException *e) {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::DoesTableExist");
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
    
    dsOpenParams open_params;
    open_params.m_bReadOnly = m_bReadOnly;
    
    const std::wstring sName = m_pDatabase->GetName();
    
    m_pDatabase->Close(); // we do not close locker
    delete m_pDatabase;
    m_pDatabase = new CDaoDatabase;
    this->OpenDB(sName.c_str(), open_params);
}

bool CDaoDatabaseImpl::CompactDatabase()
{
    const std::wstring sName = this->GetName();

    try 
    {
        // Make sure you're passing the address of a path to a different target file than the one you're compacting. 
        std::wstring sTemp = sName.c_str();
        sTemp += L"-temp";
        if ( inernal_dao_file_utils::DoesFileExist(sTemp.c_str()) ) {
            inernal_dao_file_utils::RemoveFile(sTemp.c_str()); // just in case remove any garbage
        }

        m_pDatabase->Close(); 
        delete m_pDatabase;

        CDaoWorkspace daoWorkspace;
        daoWorkspace.CompactDatabase(sName.c_str(), sTemp.c_str(), dbLangGeneral);

        if ( !inernal_dao_file_utils::CopyFile(sTemp.c_str(), sName.c_str()) ) {
            ASSERT(FALSE);
            std::wstring sError  = L"inernal_dao_file_utils::CopyFile failed. From: ";
                         sError += sTemp.c_str();
                         sError += L" to ";
                         sError += sName.c_str();
            m_pErrorHandler->OnError(sError.c_str(), L"CDaoDatabaseImpl::CompactDatabase()");
            return false;
        }
    
        inernal_dao_file_utils::RemoveFile(sTemp.c_str());

        dsOpenParams open_params;
        open_params.m_bReadOnly = m_bReadOnly;
        m_pDatabase = new CDaoDatabase;
        this->OpenDB(sName.c_str(), open_params);
    }
    catch (CDaoException *e)
    {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::CompactDatabase");
        e->Delete();    
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::CopyTableData(CDaoDatabaseImpl *pSrcDB, CDaoDatabaseImpl *pDstDB, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst)
{
    ASSERT(pSrcDB);
    ASSERT(pDstDB);
    return dao_extensions::CopyTableDataImpl(pSrcDB->m_pDatabase, pDstDB->m_pDatabase, sTableNameSrc, sTableNameDst, pDstDB->m_pErrorHandler);
}

CDaoDatabaseImpl::dbErrorHandler CDaoDatabaseImpl::SetErrorHandler(CDaoDatabaseImpl::dbErrorHandler newHandler)
{
    ASSERT(m_pErrorHandler);
    return m_pErrorHandler->SetErrorHandler(newHandler);
}

void CDaoDatabaseImpl::DeleteRelation(const wchar_t *sRelation)
{
    try {
        m_pDatabase->DeleteRelation(sRelation);
    }
    catch (CDaoException *e) {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::DeleteRelation"); 
        e->Delete();
    }
}

bool CDaoDatabaseImpl::CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                      const wchar_t *sField, const wchar_t *sForeignField)
{
    ASSERT(m_pDatabase);

    try {
        m_pDatabase->CreateRelation(sName, sTable, sForeignTable, lAttr, sField, sForeignField);
    }
    catch (CDaoException *e) {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::CreateRelation");
        e->Delete();
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info)
{
    ASSERT(m_pDatabase);
    CDaoTableDef tableInfo(m_pDatabase);
    const std::wstring sTableUTF16 = ds_str_conv::ConvertFromUTF8(sTable);
    tableInfo.Open(sTableUTF16.c_str());

    if ( !tableInfo.IsOpen() ) 
    {
        std::wstring sError = L"GetTableFieldInfo failed. Table ";
        sError += sTableUTF16;
        sError += L".";
        m_pErrorHandler->OnError(sError.c_str(), L"CDaoDatabaseImpl::GetTableFieldInfo");
        return false;
    }

    CDaoFieldInfo fieldInfo;
    
    short nFields = tableInfo.GetFieldCount();
    for (short i = 0; i < nFields; ++i) 
    {
        tableInfo.GetFieldInfo(i, fieldInfo);
        const short nType = fieldInfo.m_nType;
        const dsFieldType field_type = CDaoRecordsetImpl::DaoTypeToDs(nType);

        if ( field_type == dsFieldType::Undefined )
        {
            std::wstring sError  = L"Field type: ";
                            sError += std::to_wstring(nType);
                            sError += L" is undefined.";
            m_pErrorHandler->OnError(sError.c_str(), L"CDaoDatabaseImpl::GetTableFieldInfo()");
            ASSERT(FALSE);
        }
         
        std::wstring sName = fieldInfo.m_strName;
        info[sName] = field_type;
    }

    tableInfo.Close();

    return true;
}

std::vector<std::string> CDaoDatabaseImpl::GetTableList()
{
    std::vector<std::string> arrTables;
    const int32_t nTableCount = m_pDatabase->GetTableDefCount();

    for (int32_t i = 0; i < nTableCount; ++i) {
		CDaoTableDefInfo tabledefinfo;                           
		m_pDatabase->GetTableDefInfo(i, tabledefinfo);                      
		if (tabledefinfo.m_lAttributes == 0) {  
            const std::string sTableName(ds_str_conv::ConvertToUTF8(tabledefinfo.m_strName.GetString()));
			arrTables.push_back(sTableName);
		}
	}

    return arrTables;
}

bool CDaoDatabaseImpl::DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    std::wstring sSQL = L"ALTER TABLE ";
    sSQL += sTableName;
    sSQL += L" DROP COLUMN ";
    sSQL += sColumnName;
    sSQL += L";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName)
{
    ASSERT(false);
    return false;
}

bool CDaoDatabaseImpl::DropTable(const wchar_t *sTableName)
{
    std::wstring sSQL = L"DROP TABLE ";
    sSQL += sTableName;
    sSQL += L";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::DropTrigger(const wchar_t *sTriggerName)
{
    std::wstring sSQL = L"DROP TRIGGER ";
    sSQL += sTriggerName;
    sSQL += L";";

    if (!Execute(sSQL.c_str())) {
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::DropIndex(const wchar_t *sIndexName)
{
    std::wstring sDropStatement = L"DROP INDEX ";
    sDropStatement += sIndexName;
    sDropStatement += L";";
    if (!Execute(sDropStatement.c_str())){
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::Backup(const char *sBackupFile)
{
    ASSERT(FALSE);
    return true;
}

bool CDaoDatabaseImpl::CreateTable(const char *sTableName, const dsTableFieldInfo &info)
{
    std::wstring sField;
    dsFieldType nFieldType;
    try {
        CDaoTableDef TableDef(m_pDatabase);
        const std::wstring sTableNameUTF16 = ds_str_conv::ConvertFromUTF8(sTableName);
        TableDef.Create(sTableNameUTF16.c_str());
                        
        for (const auto it : info) {
            sField = it.first;
            nFieldType = it.second;

            short nType = -1;
            switch (nFieldType) {
            case dsFieldType::Text:
            {
                nType = dbMemo; // Just to be sure, that JSON field will fit in this field
            }
            break;
            case dsFieldType::Integer:
            {
                nType = dbLong;
            }
            break;
            case dsFieldType::Double:
            {
                nType = dbDouble;
            }
            break;
            case dsFieldType::Blob:
            {
                nType = dbLongBinary;
            }
            break;
            case dsFieldType::DateTime:
            {
                nType = dbDate;
            }
            break;
            default:
                ASSERT(false);
                break;
            }

            // SQLite does not support date type.
            // For such reason - date type field decision is executed on such conditions:
            // if (FieldTypeIsInteger && FieldNameContains("date")) => field type must be DateTime
            if (nType == dbLong) {
                std::wstring sFieldUpper = sField;
                ds_str_conv::MakeUpper(sFieldUpper);
                const size_t nPos = sFieldUpper.find(L"DATE");
                if (nPos != std::wstring::npos) {
                    nType = dbDate;
                }
            }

            TableDef.CreateField(sField.c_str(), nType, 0);
        }

        TableDef.Append();
    }
    catch (CDaoException *e)
    {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::CreateTable");
        e->Delete();
        return false;
    }

    return true;
}

bool CDaoDatabaseImpl::CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info)
{
    for (const std::pair<std::string, dsTableFieldInfo> &info : tables_info) {
        if (!CreateTable(info.first.c_str(), info.second)) {
            return false;
        }
    }

    return true;
}

bool CDaoDatabaseImpl::CreateDB(const wchar_t *sPath)
{
    try {
        m_pDatabase->Create(sPath);
    }
    catch (CDaoException *e)
    {
        ASSERT(FALSE);
        m_pErrorHandler->OnDaoException(e, L"CDaoDatabaseImpl::CreateDB");
        e->Delete();
        return false;
    }

    return true;
}