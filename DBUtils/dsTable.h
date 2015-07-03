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
	dsTable(dsDatabase *pDatabase, LPCTSTR sTableName);		
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
	void OpenSQL(LPCTSTR sSQL);      
	bool OpenView(LPCTSTR sViewName); // returns true if open succeeds

	bool SeekIndex(LPCTSTR sIndex, LPCTSTR sValue);          
	bool SeekIndex(LPCTSTR sIndex, long nValue);             

	CStdString GetFieldString(LPCTSTR sFieldName) const;     
    void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue); 

	long GetFieldLong(LPCTSTR sFieldName) const;             
    void SetFieldLong(LPCTSTR sFieldName, long nValue);      

	double GetFieldDouble(LPCTSTR sFieldName) const;         
    void SetFieldDouble(LPCTSTR sFieldName, double dValue);  

	bool GetFieldBool(LPCTSTR sFieldName) const;             
    void SetFieldBool(LPCTSTR sFieldName, bool bValue);      

	time_t GetFieldDateTime(LPCTSTR sFieldName) const;        
    void SetFieldDateTime(LPCTSTR sFieldName, time_t nValue); 

	COLORREF GetFieldRGB(LPCTSTR sFieldName) const;          
	void SetFieldRGB(LPCTSTR sFieldName, COLORREF color);    
    
	bool DoesFieldExist(LPCTSTR sFieldName);         
	bool IsFieldValueNull(LPCTSTR sFieldName) const; 
	void SetFieldNull(LPCTSTR sFieldName);           

	void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize) const; 
	void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize);         
	
	// Deletes all records with given value 
	// Returns true if any record was deleted
	bool DeleteAllByIndex(LPCTSTR sField, LPCTSTR sValue); // returns false in case of error 
	bool DeleteAllByIndex(LPCTSTR sField, long    nValue); // returns false in case of error 
    bool DeleteByIndex(LPCTSTR sField, LPCTSTR sValue);
    bool DeleteByIndex(LPCTSTR sField, long nValue);

    void Flush();  // Deletes all records in table
	bool Delete(); // returns false in case of error 

	void AddNew();
	void Edit();
	bool Update(); 

	long GetRecordCount(); 

	LPCTSTR GetTableName() const;

    CStdString GetUniqueTextFieldValue(LPCTSTR sFieldName, LPCTSTR sPrefix, int width); 

// Attributes
private:
	CStdString m_sTableName;
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
	CStdString Get##name() const   { return GetFieldString(realname);  } \
	void Set##name(LPCTSTR sValue) { SetFieldString(realname, sValue); } \
	FIELD_INDICATORS(name, realname)

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
	bool SeekBy##name(LPCTSTR sValue)      { return SeekIndex(realname, sValue); } \
	bool DeleteAllBy##name(LPCTSTR sValue) { return DeleteAllByIndex(realname, sValue); } \
	bool DeleteBy##name(LPCTSTR sValue)    { return DeleteByIndex(realname, sValue); } \
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
