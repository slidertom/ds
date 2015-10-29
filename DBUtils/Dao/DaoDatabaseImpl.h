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
	virtual bool BeginTrans() override; 
	virtual bool CommitTrans() override;
	virtual bool Rollback() override;

	virtual bool Execute(LPCTSTR lpszSQL) override; 
	virtual void Close() override; 
	virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) override;

	virtual dsDBType GetType() override;

	virtual bool IsReadOnly() const override;
	virtual bool IsOpen() const override;

	virtual CStdString GetName() override;
	
	virtual bool DoesTableExist(LPCTSTR sTable) override; 
	
	virtual CAbsRecordset *CreateRecordset() override;

    virtual void CommitDatabase() override;

    virtual bool CompactDatabase() override;

	virtual bool CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst) override;

	virtual void DeleteRelation(LPCTSTR sRelation) override;
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField) override;

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) override;

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info) override;

public:
    CDaoDatabase *GetDaoDB() { return m_pDatabase; }

// Attributes
private:
    CDaoDatabase *m_pDatabase;
    CDaoErrorHandler *m_pErrorHandler;
	bool m_bReadOnly;
};

#endif 
