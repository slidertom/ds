#ifndef __SQ_LITE_DATABASE_IMPL_H__
#define __SQ_LITE_DATABASE_IMPL_H__
#pragma once

#ifndef __ABS_DATABASE_H__
    #include "../AbsDatabase.h"
#endif

struct sqlite3;
class CSqLiteErrorHandler;
namespace sqlite_util { class CFieldInfoMap; };

class CSqLiteDatabaseImpl : public CAbsDatabase
{
// Construction/Destruction
public:
    CSqLiteDatabaseImpl();
    virtual ~CSqLiteDatabaseImpl();

// Static operations
public:
    static bool IsSqLiteDB(const wchar_t *sPath);

// Overrides
public:
    virtual bool BeginTrans() override;  
    virtual bool CommitTrans() override; 
    virtual bool Rollback() override;    
    virtual bool Backup(const char *sBackupFile) override;

    virtual bool Execute(const wchar_t *lpszSQL) override; 
    virtual bool Execute(const char *sSQL) override;

    virtual bool OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) override;

    virtual dsDBType GetType() override;

    virtual bool IsReadOnly() const override;
    virtual bool IsOpen() const override;

    virtual std::wstring GetName() override;
    
    virtual bool DoesTableExist(const wchar_t *sTable) override;
    virtual bool DoesTableExistUTF8(const char *sTable) override;
    virtual bool DoesViewExistUTF8(const char *sView) override;
    virtual bool DoesIndexExistUTF8(const char *sIndex) override;

    virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override;

    virtual bool CompactDatabase() override;

    virtual void DeleteRelation(const wchar_t *sRelation) override;
    virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                const wchar_t *sField, const wchar_t *sForeignField) override;

    virtual bool GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

    virtual std::vector<std::string> GetTableList() override;

    virtual bool DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName) override;
    virtual bool DropColumn(const char *sTableName, const char *sColumnName) override;
    virtual bool RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName) override;
    virtual bool DropTable(const wchar_t *sTableName) override;
    virtual bool DropTable(const char *sTableName)    override;
    virtual bool DropTrigger(const wchar_t *sTriggerName) override;
    virtual bool DropIndex(const wchar_t *sIndexName) override;
    virtual bool DropForeignKeys(const wchar_t* sTableName) override;

    virtual bool CreateTable(const char *sTableName, const dsTableFieldInfo &info) override;
    virtual bool CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info) override;

    virtual bool CreateDB(const wchar_t *sPath) override;

public:
    sqlite3 *GetSqLiteDB() { return m_pDB; }
    CSqLiteErrorHandler *GetErrorHandler() { return m_pErrorHandler; }
    void OnError(const char *sError, const char *sFunctionName);
    const sqlite_util::CFieldInfoMap *GetTableFieldInfoImpl(const char *sTableNameUTF8);
    bool IsTransMode() const { return m_bTransMode; }

private:
    void Close();
    void ClearFieldsInfo();

    bool RecreateTable(const char *sTableName, const char *sColumnName, bool bDelete, bool bRemoveCollateNoCase);

// Attributes
private:
    std::wstring m_sFilePath;
    bool m_bReadOnly  {false};
    bool m_bMemory    {false};
    bool m_bMultiUser {false};
    bool m_bTransMode {false}; // invariant
    sqlite3 *m_pDB    {nullptr};
    CSqLiteErrorHandler *m_pErrorHandler;
    std::unordered_map<std::string, sqlite_util::CFieldInfoMap *>  m_table_field_info_map;

#ifdef _DEBUG
    friend class CSqLiteRecordsetImpl;
#endif
};

#endif 