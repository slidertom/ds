#ifndef __SQ_LITE_RECORD_SET_IMPL_H__
#define __SQ_LITE_RECORD_SET_IMPL_H__
#pragma once

#ifndef __ABS_RECORD_SET_H__
	#include "../AbsRecordset.h"
#endif

#include "unordered_map"

// Notes:
// Columns and tables names are case insensitive

class CSqLiteErrorHandler;
class CSqLiteDatabaseImpl;
struct sqlite3_stmt;
struct sqlite3;
namespace sqlite_util { class CFieldDataMap; class CFieldInfoMap; }

class CSqLiteRecordsetImpl : public CAbsRecordset
{
// Construction/Destruction
public:
    CSqLiteRecordsetImpl(CSqLiteDatabaseImpl *pDatabase, CSqLiteErrorHandler *pErrorHandler);
	virtual ~CSqLiteRecordsetImpl();

// Overrides/Operations
public:
	virtual bool Open(LPCTSTR sTableName);
	virtual bool OpenSQL(LPCTSTR sSQL);
    bool OpenSQLUTF8(const char *sSQL);
	virtual bool OpenView(LPCTSTR sViewName);

    virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue);
	virtual bool SeekByLong(LPCTSTR sIndex, long nValue);
    bool SeekByLongUTF8(const char *sIndexUTF8, long nValue);

	virtual bool MoveNext();
	virtual bool MoveFirst();

	virtual long GetRecordCount();

	virtual bool Delete();
	virtual void AddNew();
	virtual void Edit();
	virtual bool Update();

	virtual bool IsEOF();

	virtual void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize);
	virtual void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize);

	virtual void SetFieldValueNull(LPCTSTR lpszName);

	virtual CStdString GetFieldString(LPCTSTR sFieldName);
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue);

    virtual std::string GetFieldStringUTF8(const char *sFieldName);
	virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue);

	virtual long GetFieldLong(LPCTSTR sFieldName);
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue);

	virtual double GetFieldDouble(LPCTSTR sFieldName);
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue);

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName);
	virtual void SetFieldDateTime(LPCTSTR sFieldName, const time_t &time);

	virtual bool IsFieldValueNull(LPCTSTR sFieldName);

	virtual bool DoesFieldExist(LPCTSTR sFieldName);

    virtual void Flush();
    virtual bool DeleteAllByStringValue(LPCTSTR sField, LPCTSTR sValue);
    virtual bool DeleteAllByLongValue(LPCTSTR sField, long nValue);

    void PrepareInsert();
    void CommitInsert();

private:
    int FindColumnIndex(LPCTSTR sFieldName);
    bool OpenImpl(const char *sql);
    void OpenImpl();
    void CloseStatement();

    void DoInsertDefault();
    void DoUpdate();
    void OnErrorCode(int rc, LPCTSTR sFunctionName);

// Attributes
private:
    CSqLiteDatabaseImpl *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
    std::string m_sTable;
    sqlite3_stmt *m_stmt;    
    sqlite3_stmt *m_insert_stmt;
    bool m_bEOF;
    
    __int64 m_nEditRowId;
    std::unordered_map<CStdString, int, std::hash<std::basic_string<TCHAR>>> m_name_to_index;

    sqlite_util::CFieldDataMap *m_pSaveData;
    sqlite_util::CFieldInfoMap *m_pInfoData;
};

#endif 
