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
    static bool IsDaoDB(LPCTSTR sPath);

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

    virtual void CommitDatabase();

    virtual bool CompactDatabase();

	virtual bool CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst);

	virtual void DeleteRelation(LPCTSTR sRelation);
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField);

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info);

public:
    CDaoDatabase *GetDaoDB() { return m_pDatabase; }

// Attributes
private:
    CDaoDatabase *m_pDatabase;
    CDaoErrorHandler *m_pErrorHandler;
	bool m_bReadOnly;
};

#endif 
