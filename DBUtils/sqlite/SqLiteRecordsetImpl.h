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
	virtual bool Open(const wchar_t *sTableName) override;
	virtual bool OpenSQL(const wchar_t *sSQL) override;
    bool OpenSQLUTF8(const char *sSQL);
	virtual bool OpenView(const wchar_t *sViewName) override;

    virtual bool SeekByString(const wchar_t *sIndex, const wchar_t *sValue) override;
	virtual bool SeekByLong(const wchar_t *sIndex, int nValue) override;
    bool SeekByLongUTF8(const char *sIndexUTF8, long nValue);

	virtual bool MoveNext() override;
	virtual bool MoveFirst() override;

	virtual int GetRecordCount() const override;
    virtual int GetColumnCount() const override;
    virtual std::wstring GetColumnName(int nCol) const override;
    virtual dsFieldType GetColumnType(int nCol) const override;

	virtual bool Delete() override;
	virtual void AddNew() override;
	virtual void Edit() override;
	virtual bool Update() override;

	virtual bool IsEOF() override;

	virtual void SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize) override;
	virtual void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) override;
	virtual void FreeBinary(unsigned char *pData);

	virtual void SetFieldValueNull(const wchar_t *lpszName) override;

	virtual std::wstring GetFieldString(const wchar_t *sFieldName) override;
	virtual void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) override;

    virtual std::string GetFieldStringUTF8(const char *sFieldName) override;
	virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) override;

	virtual int     GetFieldLong(const wchar_t *sFieldName) override;
    virtual int64_t GetFieldInt64(const wchar_t *sFieldName) override;
	virtual void    SetFieldLong(const wchar_t *sFieldName, int lValue) override;
    virtual void    SetFieldInt64(const wchar_t *sFieldName, int64_t lValue) override;
    
	virtual double GetFieldDouble(const wchar_t *sFieldName) override;
	virtual void SetFieldDouble(const wchar_t *sFieldName, double dValue) override;

	virtual time_t GetFieldDateTime(const wchar_t *sFieldName) override;
	virtual void SetFieldDateTime(const wchar_t *sFieldName, const time_t &time) override;

	virtual bool IsFieldValueNull(const wchar_t *sFieldName) override;

	virtual bool DoesFieldExist(const wchar_t *sFieldName) override;

    virtual void Flush() override;
    virtual bool DeleteAllByStringValue(const wchar_t *sField, const wchar_t *sValue) override;
    virtual bool DeleteAllByLongValue(const wchar_t *sField, int nValue) override;
    virtual bool DeleteByLongValue(const wchar_t *sField, int nValue) override;
    virtual bool DeleteByStringValue(const wchar_t *sField, const wchar_t *sValue) override;

    void PrepareInsert();
    void CommitInsert();
    bool MoveFirstImpl(); // should be used for the inside implementations

private:
    int FindColumnIndex(const wchar_t *sFieldName);
    bool OpenImpl(const char *sql);
    void CloseStatement();

    void DoInsertDefault();
    bool DoUpdate();
    void OnErrorCode(int rc, const char *sFunctionName);

// Attributes
private:
    CSqLiteDatabaseImpl *m_pDB;
    CSqLiteErrorHandler *m_pErrorHandler;
    std::string m_sTable;
   
    sqlite3_stmt *m_stmt;  // main statement
    sqlite3_stmt *m_insert_stmt;
    sqlite3_stmt *m_update_stmt;
    std::string m_sUpdateValues;
    bool m_bEOF;
	bool m_bSQLOpened;
    
    int64_t m_nEditRowId;
    std::unordered_map<std::wstring, int, std::hash<std::basic_string<wchar_t>>> m_name_to_index;

    sqlite_util::CFieldDataMap *m_pSaveData;
    const sqlite_util::CFieldInfoMap *m_pFieldInfoData;
};

#endif 
