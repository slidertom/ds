#ifndef __DS_TABLE_H__
#define __DS_TABLE_H__
#pragma once

#ifndef __DS_DATABASE_H__
	#include "dsDatabase.h"
#endif

class CAbsRecordset;

// Do provide MFC CDaoRecordset or CRecordset interface for the different data storage systems

// DAO: Index should contain same name as field!
//         INDEX_NAME EQ FIELD_NAME

// Exceptions are never throwed 

// Expected AddNew/INSERT usage:
// dsTable loader(pDB, _T("Table_Name");
//  loader.AddNew();
//      int nId = loader.GetFieldLong(_T("ID")); // after AddNew you can always retrieved new record key
//      loader.SetFieldString(_T("Code"), _T("My_Code"));
//  loader.Update();
class DB_UTILS_API dsTable
{
// Construction/Destruction
public:
	dsTable(dsDatabase *pDatabase, const wchar_t *sTableName);		
	virtual ~dsTable();

// Attributes
public:
	bool IsEOF() const; 

	void MoveNext();
	bool MoveFirst(); // returns false if no records exist

	dsDatabase *GetDatabase() const;

// Operations
public:
	bool Open();                     
	void OpenSQL(const wchar_t *sSQL);      
	bool OpenView(const wchar_t *sViewName); // returns true if open succeeds

	bool SeekIndex(const wchar_t *sIndex, const wchar_t *sValue);          
	bool SeekIndex(const wchar_t *sIndex, long nValue);             

	std::wstring GetFieldString(const wchar_t *sFieldName) const;     
    void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue); 
    std::string GetFieldStringUTF8(const char *sFieldName) const;     
    void SetFieldStringUTF8(const char *sFieldName, const char *sValue); 

	long GetFieldLong(const wchar_t *sFieldName) const;             
    void SetFieldLong(const wchar_t *sFieldName, long nValue);      

	double GetFieldDouble(const wchar_t *sFieldName) const;         
    void SetFieldDouble(const wchar_t *sFieldName, double dValue);  

	bool GetFieldBool(const wchar_t *sFieldName) const;             
    void SetFieldBool(const wchar_t *sFieldName, bool bValue);      

	time_t GetFieldDateTime(const wchar_t *sFieldName) const;        
    void SetFieldDateTime(const wchar_t *sFieldName, time_t nValue); 

	COLORREF GetFieldRGB(const wchar_t *sFieldName) const;          
	void SetFieldRGB(const wchar_t *sFieldName, COLORREF color);    
    
	bool DoesFieldExist(const wchar_t *sFieldName);         
	bool IsFieldValueNull(const wchar_t *sFieldName) const; 
	void SetFieldNull(const wchar_t *sFieldName);           

	void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, unsigned long &nSize) const; 
	void SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, unsigned long nSize);
    void FreeBinary(unsigned char *pData);
	
	// Deletes all records with given value 
	// Returns true if any record was deleted
	bool DeleteAllByIndex(const wchar_t *sField, const wchar_t *sValue); // returns false in case of error 
	bool DeleteAllByIndex(const wchar_t *sField, long    nValue); // returns false in case of error 
    bool DeleteByIndex(const wchar_t *sField, const wchar_t *sValue);
    bool DeleteByIndex(const wchar_t *sField, long nValue);

    void Flush();  // Deletes all records in table
	bool Delete(); // returns false in case of error 

	void AddNew();
	void Edit();
	bool Update(); 

	long GetRecordCount(); 

	const wchar_t *GetTableName() const;

    std::wstring GetUniqueTextFieldValue(const wchar_t *sFieldName, const wchar_t *sPrefix, int width); 

// Attributes
private:
	std::wstring m_sTableName;
	CAbsRecordset *m_pSet;
	dsDatabase  *m_pDatabase;
};

///////////////////////////////////////////////////////////////////////////////
//  macroses to define indexes and fields
//
			
// fields

// Helper macros
#define FIELD_INDICATORS(name, realname) \
	bool IsNull##name()const { return IsFieldValueNull(realname); } \
	bool DoesExist##name()   { return DoesFieldExist(realname);   } \
    void SetNull##name()     { SetFieldNull(realname); }
		
// Field macros with renaming

#define FIELD_TEXT(name, realname) \
	CStdString Get##name() const          { return GetFieldString(realname);  } \
	void Set##name(const wchar_t *sValue) { SetFieldString(realname, sValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_TEXT_UTF8(name, realname_utf8) \
    std::string Get##name##UTF8() const      { return GetFieldStringUTF8(realname_utf8);  } \
	void Set##name##UTF8(const char *sValue) { SetFieldStringUTF8(realname_utf8, sValue); } \
	
#define FIELD_LONG(name, realname) \
	long Get##name() const      { return GetFieldLong(realname); } \
	void Set##name(long nValue) { SetFieldLong(realname, nValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_DOUBLE(name, realname) \
	double Get##name() const      { return GetFieldDouble(realname);  } \
	void Set##name(double dValue) { SetFieldDouble(realname, dValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_BOOL(name, realname) \
	bool Get##name() const      { return GetFieldBool(realname);  } \
	void Set##name(bool bValue) { SetFieldBool(realname, bValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_BINARY(name, realname) \
	void Get##name(unsigned char **pData, unsigned long &nSize) const { GetFieldBinary(realname, pData, nSize); } \
	void Set##name(unsigned char *pData, unsigned long nSize) { SetFieldBinary(realname, pData, nSize); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_RGB(name, realname) \
	COLORREF Get##name() const     { return GetFieldRGB(realname); } \
	void Set##name(COLORREF value) { SetFieldRGB(realname, value); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_DATE(name, realname) \
	time_t Get##name() const      { return GetFieldDateTime(realname); } \
	void Set##name(time_t nValue) { SetFieldDateTime(realname, nValue); } \
	FIELD_INDICATORS(name, realname)

// indexes
#define KEY_TEXT(name, realname) \
	bool SeekBy##name(const wchar_t *sValue)      { return SeekIndex(realname, sValue); } \
	bool DeleteAllBy##name(const wchar_t *sValue) { return DeleteAllByIndex(realname, sValue); } \
	bool DeleteBy##name(const wchar_t *sValue)    { return DeleteByIndex(realname, sValue); } \
    FIELD_TEXT(name, realname)
	
#define KEY_LONG(name, realname) \
	bool SeekBy##name(long nValue)      { return SeekIndex(realname, nValue); } \
	bool DeleteAllBy##name(long nValue) { return DeleteAllByIndex(realname, nValue); } \
	bool DeleteBy##name(long nValue)    { return DeleteByIndex(realname, nValue); } \
    FIELD_LONG(name, realname)

#define KEY_BOOL(name, realname) \
	bool SeekBy##name(long nValue)      { return SeekIndex(realname, nValue); } \
	bool DeleteAllBy##name(long nValue) { return DeleteAllByIndex(realname, nValue); } \
	bool DeleteBy##name(long nValue)    { return DeleteByIndex(realname, nValue); } \
    FIELD_BOOL(name, realname)

#endif
