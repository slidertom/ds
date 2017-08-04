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
	CSqLiteDatabaseImpl(bool bMultiUser);
	virtual ~CSqLiteDatabaseImpl();

// Static operations
public:
    static bool IsSqLiteDB(const wchar_t *sPath);

// Overrides
public:
	virtual bool BeginTrans() override;  
	virtual bool CommitTrans() override; 
	virtual bool Rollback() override;    

	virtual bool Execute(const wchar_t *lpszSQL) override; 
    bool ExecuteUTF8(const char *sqlUTF8);

	virtual bool OpenDB(const wchar_t *sPath, bool bReadOnly, const wchar_t *szPsw) override;

	virtual dsDBType GetType() override;

	virtual bool IsReadOnly() const override;
	virtual bool IsOpen() const override;

	virtual std::wstring GetName() override;
	
	virtual bool DoesTableExist(const wchar_t *sTable) override;
	
	virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override { }

    virtual bool CompactDatabase() override;

	virtual void DeleteRelation(const wchar_t *sRelation) override;
	virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, long lAttr,
								const wchar_t *sField, const wchar_t *sForeignField) override;

    virtual bool GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

public:
    sqlite3 *GetSqLiteDB() { return m_pDB; }
    CSqLiteErrorHandler *GetErrorHandler() { return m_pErrorHandler; }
    void OnError(const char *sError, const char *sFunctionName);
    const sqlite_util::CFieldInfoMap *GetTableFieldInfoImpl(const char *sTableNameUTF8);
    bool IsTransMode() const { return m_bTransMode; }

private:
    void Close(); 

// Attributes
private:
    std::wstring m_sFilePath;
	bool m_bReadOnly;
    sqlite3 *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
    std::unordered_map<std::string, sqlite_util::CFieldInfoMap *>  m_table_field_info_map;

    bool m_bTransMode; // invariant

	bool m_bMultiUser;

#ifdef _DEBUG
    friend class CSqLiteRecordsetImpl;
#endif
};

#endif 
