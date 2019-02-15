#ifndef __DAO_DATABASE_IMPL_H__
#define __DAO_DATABASE_IMPL_H__
#pragma once

#ifndef __ABS_DATABASE_H__
    #include "../AbsDatabase.h"
#endif

class CDaoDatabase;
class CDaoErrorHandler;

class CDaoDatabaseImpl : public CAbsDatabase
{
// Construction/Destruction
public:
    CDaoDatabaseImpl();
    virtual ~CDaoDatabaseImpl();

// Static operations
public:
    static bool IsDaoDB(const wchar_t *sPath);

// Overrides
public:
    virtual bool BeginTrans() override; 
    virtual bool CommitTrans() override;
    virtual bool Rollback() override;

    virtual bool Execute(const wchar_t *lpszSQL) override; 
    virtual bool OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) override;

    virtual dsDBType GetType() override;

    virtual bool IsReadOnly() const override;
    virtual bool IsOpen() const override;

    virtual std::wstring GetName() override;
    
    virtual bool DoesTableExist(const wchar_t *sTable) override; 
    virtual bool DoesTableExistUTF8(const char *sTable) override;
    
    virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override;

    virtual bool CompactDatabase() override;

    virtual void DeleteRelation(const wchar_t *sRelation) override;
    virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                const wchar_t *sField, const wchar_t *sForeignField) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

    virtual bool GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info) override;

// Static operations
public:
    static bool CopyTableData(CDaoDatabaseImpl *pSrcDB, CDaoDatabaseImpl *pDstDB, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst);

public:
    CDaoDatabase *GetDaoDB() { return m_pDatabase; }

private:
    void Close();

// Attributes
private:
    CDaoDatabase *m_pDatabase;
    CDaoErrorHandler *m_pErrorHandler;
    bool m_bReadOnly;
};

#endif 
