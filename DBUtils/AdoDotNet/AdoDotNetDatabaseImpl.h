#ifndef __ADO_DOT_NET_DATABASE_IMPL_H__
#define __ADO_DOT_NET_DATABASE_IMPL_H__
#pragma once

#ifndef __ABS_DATABASE_H__
    #include "../AbsDatabase.h"
#endif

class CDotNetDatabaseAbs;

class CAdoDotNetDatabaseImpl : public CAbsDatabase
{
// Construction/Destruction
public:
    CAdoDotNetDatabaseImpl();
    virtual ~CAdoDotNetDatabaseImpl();

// static operations
public:
    static bool IsMSSQLServerAdoDotNet(const wchar_t *sPath);

// Overrides
public:
    virtual bool BeginTrans() override;  
    virtual bool CommitTrans() override; 
    virtual bool Rollback() override;    
    virtual bool Backup(const char *sBackupFile) override;

    virtual bool Execute(const wchar_t *lpszSQL) override;
    virtual bool OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) override;

    virtual dsDBType GetType() override;

    virtual bool IsReadOnly() const override;
    virtual bool IsOpen() const override;

    virtual std::wstring GetName() override;
    
    virtual bool DoesTableExist(const wchar_t *sTable) override;
    virtual bool DoesTableExistUTF8(const char *sTable) override;

    virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override { }

    virtual bool CompactDatabase() override { return true; }

    virtual void DeleteRelation(const wchar_t *sRelation) override;
    virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                const wchar_t *sField, const wchar_t *sForeignField) override;

    virtual bool GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

    virtual std::vector<std::string> GetTableList() override;

    virtual bool DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName) override;
    virtual bool DropTable(const wchar_t *sTableName) override;

    virtual bool CreateTable(const wchar_t *sTableName, const dsTableFieldInfo &info) override;

private:
    void Close(); 

// Attributes
private:
    bool m_bReadOnly {false};

    CDotNetDatabaseAbs *m_pDatabase;

    std::wstring m_sConnString;
};

#endif 
