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
    static bool IsMSSQLServerAdoDotNet(LPCTSTR sPath);

// Overrides
public:
	virtual bool BeginTrans() override;  
	virtual bool CommitTrans() override; 
	virtual bool Rollback() override;    

	virtual bool Execute(LPCTSTR lpszSQL) override; 
	virtual void Close() override; 
	virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) override;

	virtual dsDBType GetType() override;

	virtual bool IsReadOnly() const override;
	virtual bool IsOpen() const override;

	virtual std::wstring GetName() override;
	
	virtual bool DoesTableExist(LPCTSTR sTable) override;
	
	virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override { }

    virtual bool CompactDatabase() override { return true; }

	virtual void DeleteRelation(LPCTSTR sRelation) override;
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField) override;

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

// Attributes
private:
	bool m_bReadOnly;

	CDotNetDatabaseAbs *m_pDatabase;

    std::wstring m_sConnString;
};

#endif 
