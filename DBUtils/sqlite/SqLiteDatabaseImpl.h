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
    static bool IsSqLiteDB(LPCTSTR sPath);

// Overrides
public:
	virtual bool BeginTrans() override;  
	virtual bool CommitTrans() override; 
	virtual bool Rollback() override;    

	virtual bool Execute(LPCTSTR lpszSQL) override; 

    bool ExecuteUTF8(const char *sqlUTF8);

	virtual void Close() override; 
	virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) override;

	virtual dsDBType GetType() override;

	virtual bool IsReadOnly() const override;
	virtual bool IsOpen() const override;

	virtual CStdString GetName() override;
	
	virtual bool DoesTableExist(LPCTSTR sTable) override;
	
	virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override { }

    virtual bool CompactDatabase() override;

	virtual void DeleteRelation(LPCTSTR sRelation) override;
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField) override;

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

public:
    sqlite3 *GetSqLiteDB() { return m_pDB; }
    CSqLiteErrorHandler *GetErrorHandler() { return m_pErrorHandler; }

    const sqlite_util::CFieldInfoMap *GetTableFieldInfoImpl(const char *sTableNameUTF8);

// Attributes
private:
    CStdString m_sFilePath;
	bool m_bReadOnly;
    sqlite3 *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
    std::unordered_map<std::string, sqlite_util::CFieldInfoMap *>  m_table_field_info_map;

#ifdef _DEBUG
    bool m_bTransMode; // invariant
#endif
};

#endif 
