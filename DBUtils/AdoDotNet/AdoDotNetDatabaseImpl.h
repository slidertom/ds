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
	virtual bool BeginTrans();  
	virtual bool CommitTrans(); 
	virtual bool Rollback();    

	virtual bool Execute(LPCTSTR lpszSQL); 
	virtual void Close(); 
	virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw);

	virtual dsDBType GetType();

	virtual bool IsReadOnly() const;
	virtual bool IsOpen() const;

	virtual CStdString GetName();
	
	virtual bool DoesTableExist(LPCTSTR sTable);
	
	virtual CAbsRecordset *CreateRecordset();

    virtual void CommitDatabase() { }

    virtual bool CompactDatabase() { return true; }

	virtual bool CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst) { return false; };

	virtual void DeleteRelation(LPCTSTR sRelation);
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField);

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info);

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

// Attributes
private:
	bool m_bReadOnly;

	CDotNetDatabaseAbs *m_pDatabase;

    CStdString m_sConnString;
};

#endif 
