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
//      int nId = loader.GetFieldLong(L"ID"); // after AddNew you can always retrieved new record key
//      loader.SetFieldString(L"Code", L"My_Code");
//  loader.Update();
class DB_UTILS_API dsTable
{
// Construction/Destruction
public:
	dsTable(dsDatabase *pDatabase, const wchar_t *sTableName);		
	virtual ~dsTable();

// Attributes
public:
	bool IsEOF() const noexcept; 

	void MoveNext() noexcept;
	bool MoveFirst() noexcept; // returns false if no records exist

	dsDatabase *GetDatabase() const noexcept;

// Operations
public:
	bool Open() noexcept;                     
	void OpenSQL(const wchar_t *sSQL) noexcept;      
	bool OpenView(const wchar_t *sViewName) noexcept; // returns true if open succeeds

	bool SeekIndex(const wchar_t *sIndex, const wchar_t *sValue) noexcept;          
	bool SeekIndex(const wchar_t *sIndex, int nValue) noexcept;             

	std::wstring GetFieldString(const wchar_t *sFieldName) const noexcept;     
    void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) noexcept; 
    std::string GetFieldStringUTF8(const char *sFieldName) const noexcept;     
    void SetFieldStringUTF8(const char *sFieldName, const char *sValue) noexcept; 

	int  GetFieldLong(const wchar_t *sFieldName) const noexcept;             
    void SetFieldLong(const wchar_t *sFieldName, int nValue) noexcept;      

	double GetFieldDouble(const wchar_t *sFieldName) const noexcept;         
    void SetFieldDouble(const wchar_t *sFieldName, double dValue) noexcept;  

	bool GetFieldBool(const wchar_t *sFieldName) const noexcept;             
    void SetFieldBool(const wchar_t *sFieldName, bool bValue) noexcept;      

	time_t GetFieldDateTime(const wchar_t *sFieldName) const noexcept;        
    void SetFieldDateTime(const wchar_t *sFieldName, time_t nValue) noexcept; 

	COLORREF GetFieldRGB(const wchar_t *sFieldName) const noexcept;          
	void SetFieldRGB(const wchar_t *sFieldName, COLORREF color) noexcept;    
    
	bool DoesFieldExist(const wchar_t *sFieldName) noexcept;         
	bool IsFieldValueNull(const wchar_t *sFieldName) const noexcept; 
	void SetFieldNull(const wchar_t *sFieldName) noexcept;           

	void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) const noexcept; 
	void SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize) noexcept;
    void FreeBinary(unsigned char *pData) noexcept;
	
	// Deletes all records with given value 
	// Returns true if any record was deleted
	bool DeleteAllByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept; // returns false in case of error 
	bool DeleteAllByIndex(const wchar_t *sField, int nValue) noexcept;            // returns false in case of error 
    bool DeleteByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept;
    bool DeleteByIndex(const wchar_t *sField, int nValue) noexcept;

    void Flush() noexcept;  // Deletes all records in table
	bool Delete() noexcept; // returns false in case of error 

	void AddNew() noexcept;
	void Edit() noexcept;
	bool Update() noexcept; 

	int GetRecordCount() noexcept; 
    int GetColumnCount() noexcept;
    std::wstring GetColumnName(int nCol) noexcept;
    dsFieldType GetColumnType(int nCol) noexcept;

	const wchar_t *GetTableName() const noexcept;

    std::wstring GetUniqueTextFieldValue(const wchar_t *sFieldName, const wchar_t *sPrefix, int width) noexcept; 

// Attributes
private:
	std::wstring m_sTableName;
	CAbsRecordset *m_pSet;
	dsDatabase *m_pDatabase;
};

///////////////////////////////////////////////////////////////////////////////
//  macroses to define indexes and fields
//
#define DS_WIDEN(quote) DS_WIDEN2(quote)
#define DS_WIDEN2(quote) L##quote	
// fields

// Helper macros
#define FIELD_INDICATORS(name, realname) \
	bool IsNull##name()const noexcept { return IsFieldValueNull(DS_WIDEN(realname)); } \
	bool DoesExist##name()   noexcept { return DoesFieldExist(DS_WIDEN(realname));   } \
    void SetNull##name()     noexcept { SetFieldNull(DS_WIDEN(realname)); }
		
// Field macros with renaming

#define FIELD_TEXT(name, realname) \
	std::wstring Get##name() const             noexcept { return GetFieldString(DS_WIDEN(realname));  }         \
	void Set##name(const wchar_t *sValue)      noexcept { SetFieldString(DS_WIDEN(realname), sValue); }         \
    void Set##name(const std::wstring &sValue) noexcept { SetFieldString(DS_WIDEN(realname), sValue.c_str()); } \
    std::string Get##name##UTF8() const        noexcept { return GetFieldStringUTF8(realname);  }				\
	void Set##name##UTF8(const char *sValue)   noexcept { SetFieldStringUTF8(realname, sValue); }				\
	FIELD_INDICATORS(name, realname)
	
#define FIELD_LONG(name, realname) \
	int32_t Get##name() const         noexcept { return GetFieldLong(DS_WIDEN(realname));  } \
	void    Set##name(int32_t nValue) noexcept { SetFieldLong(DS_WIDEN(realname), nValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_DOUBLE(name, realname) \
	double Get##name() const      noexcept { return GetFieldDouble(DS_WIDEN(realname));  } \
	void Set##name(double dValue) noexcept { SetFieldDouble(DS_WIDEN(realname), dValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_BOOL(name, realname) \
	bool Get##name() const      noexcept { return GetFieldBool(DS_WIDEN(realname));  } \
	void Set##name(bool bValue) noexcept { SetFieldBool(DS_WIDEN(realname), bValue); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_BINARY(name, realname) \
	void Get##name(unsigned char **pData, size_t &nSize) const noexcept { GetFieldBinary(DS_WIDEN(realname), pData, nSize); } \
	void Set##name(unsigned char *pData, size_t nSize)         noexcept { SetFieldBinary(DS_WIDEN(realname), pData, nSize); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_RGB(name, realname) \
	COLORREF Get##name() const     noexcept { return GetFieldRGB(DS_WIDEN(realname)); } \
	void Set##name(COLORREF value) noexcept { SetFieldRGB(DS_WIDEN(realname), value); } \
	FIELD_INDICATORS(name, realname)

#define FIELD_DATE(name, realname) \
	time_t Get##name() const      noexcept { return GetFieldDateTime(DS_WIDEN(realname)); } \
	void Set##name(time_t nValue) noexcept { SetFieldDateTime(DS_WIDEN(realname), nValue); } \
	FIELD_INDICATORS(name, realname)

// indexes
#define KEY_TEXT(name, realname) \
	bool SeekBy##name(const wchar_t *sValue)           noexcept { return SeekIndex(DS_WIDEN(realname), sValue);                }  \
    bool SeekBy##name(const std::wstring &sValue)      noexcept { return SeekIndex(DS_WIDEN(realname), sValue.c_str());        }  \
	bool DeleteAllBy##name(const wchar_t *sValue)      noexcept { return DeleteAllByIndex(DS_WIDEN(realname), sValue);         }  \
    bool DeleteAllBy##name(const std::wstring &sValue) noexcept { return DeleteAllByIndex(DS_WIDEN(realname), sValue.c_str()); }  \
	bool DeleteBy##name(const wchar_t *sValue)         noexcept { return DeleteByIndex(DS_WIDEN(realname), sValue);            }  \
    bool DeleteBy##name(const std::wstring &sValue)    noexcept { return DeleteByIndex(DS_WIDEN(realname), sValue.c_str());    }  \
	std::wstring GetUnique##name(const wchar_t *sPrefix, int32_t width) noexcept { return GetUniqueTextFieldValue(DS_WIDEN(realname), sPrefix, width); } \
    FIELD_TEXT(name, realname)
	
#define KEY_LONG(name, realname) \
	bool SeekBy##name(int32_t nValue)      noexcept { return SeekIndex(DS_WIDEN(realname), nValue);        } \
	bool DeleteAllBy##name(int32_t nValue) noexcept { return DeleteAllByIndex(DS_WIDEN(realname), nValue); } \
	bool DeleteBy##name(int32_t nValue)    noexcept { return DeleteByIndex(DS_WIDEN(realname), nValue);    } \
    FIELD_LONG(name, realname)

#endif
