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
	virtual bool Open(LPCTSTR sTableName) override;
	virtual bool OpenSQL(LPCTSTR sSQL) override;
    bool OpenSQLUTF8(const char *sSQL);
	virtual bool OpenView(LPCTSTR sViewName) override;

    virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue) override;
	virtual bool SeekByLong(LPCTSTR sIndex, long nValue) override;
    bool SeekByLongUTF8(const char *sIndexUTF8, long nValue);

	virtual bool MoveNext() override;
	virtual bool MoveFirst() override;

	virtual long GetRecordCount() override;

	virtual bool Delete() override;
	virtual void AddNew() override;
	virtual void Edit() override;
	virtual bool Update() override;

	virtual bool IsEOF() override;

	virtual void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize) override;
	virtual void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize) override;
	virtual void FreeBinary(unsigned char *pData);

	virtual void SetFieldValueNull(LPCTSTR lpszName) override;

	virtual CStdString GetFieldString(LPCTSTR sFieldName) override;
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue) override;

    virtual std::string GetFieldStringUTF8(const char *sFieldName) override;
	virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) override;

	virtual long GetFieldLong(LPCTSTR sFieldName) override;
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue) override;

	virtual double GetFieldDouble(LPCTSTR sFieldName) override;
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue) override;

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName) override;
	virtual void SetFieldDateTime(LPCTSTR sFieldName, const time_t &time) override;

	virtual bool IsFieldValueNull(LPCTSTR sFieldName) override;

	virtual bool DoesFieldExist(LPCTSTR sFieldName) override;

    virtual void Flush() override;
    virtual bool DeleteAllByStringValue(LPCTSTR sField, LPCTSTR sValue) override;
    virtual bool DeleteAllByLongValue(LPCTSTR sField, long nValue) override;
    virtual bool DeleteByLongValue(LPCTSTR sField, long nValue) override;
    virtual bool DeleteByStringValue(LPCTSTR sField, LPCTSTR sValue) override;

    void PrepareInsert();
    void CommitInsert();
    bool MoveFirstImpl(); // should be used for the inside implementations

private:
    int FindColumnIndex(LPCTSTR sFieldName);
    bool OpenImpl(const char *sql);
    void CloseStatement();

    void DoInsertDefault();
    bool DoUpdate();
    void OnErrorCode(int rc, LPCTSTR sFunctionName);

// Attributes
private:
    CSqLiteDatabaseImpl *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
    std::string m_sTable;
   
    sqlite3_stmt *m_stmt;    
    sqlite3_stmt *m_insert_stmt;
    sqlite3_stmt *m_update_stmt;
    std::string m_sUpdateValues;
    bool m_bEOF;
	bool m_bSQLOpened;
    
    __int64 m_nEditRowId;
    std::unordered_map<CStdString, int, std::hash<std::basic_string<TCHAR>>> m_name_to_index;

    sqlite_util::CFieldDataMap *m_pSaveData;
    const sqlite_util::CFieldInfoMap *m_pFieldInfoData;
};

#endif 
