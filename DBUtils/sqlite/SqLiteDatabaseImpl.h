#ifndef __SQ_LITE_DATABASE_IMPL_H__
#define __SQ_LITE_DATABASE_IMPL_H__
#pragma once

#ifndef __ABS_DATABASE_H__
	#include "../AbsDatabase.h"
#endif

struct sqlite3;
class CSqLiteErrorHandler;

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
	virtual bool BeginTrans();  
	virtual bool CommitTrans(); 
	virtual bool Rollback();    

	virtual bool Execute(LPCTSTR lpszSQL); 
    // return the number of database rows that were changed (or inserted or deleted)
    int ExecuteUTF8(const char *sqlUTF8);

	virtual void Close(); 
	virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw);

	virtual dsDBType GetType();

	virtual bool IsReadOnly() const;
	virtual bool IsOpen() const;

	virtual CStdString GetName();
	
	virtual bool DoesTableExist(LPCTSTR sTable);
	
	virtual CAbsRecordset *CreateRecordset();

    virtual void CommitDatabase() { }

    virtual bool CompactDatabase();

	virtual bool CopyTableData(CAbsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst);

	virtual void DeleteRelation(LPCTSTR sRelation);
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField);

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info);

    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

public:
    sqlite3 *GetSqLiteDB() { return m_pDB; }
    CSqLiteErrorHandler *GetErrorHandler() { return m_pErrorHandler; }

// Attributes
private:
    CStdString m_sFilePath;
	bool m_bReadOnly;
    sqlite3 *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
};

#endif 
