#ifndef __DS_TABLE_H__
#define __DS_TABLE_H__
#pragma once

#ifndef __DS_DATABASE_H__
    #include "dsDatabase.h"
#endif

#include <string> // std::string

class CAbsRecordset;

// Do provide MFC CDaoRecordset or CRecordset interface for the different data storage systems

// DAO: Index should contain same name as field!
//         INDEX_NAME EQ FIELD_NAME

// Exceptions are never throwed 

// Expected AddNew/INSERT usage:
// dsTable loader(pDB, L"Table_Name");
//  loader.AddNew();
//      int nId = loader.GetFieldLong(L"ID"); // after AddNew you can always retrieved new record key
//      loader.SetFieldString(L"Code", L"My_Code");
//  loader.Update();
class DB_UTILS_API dsTable
{
// Construction/Destruction
public:
    dsTable(dsDatabase *pDatabase, const wchar_t *sTableName);        
    dsTable(dsDatabase *pDatabase, const char *sTableNameUTF8);        
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
    bool SeekIndex(const char *sIndex, const char *sValue) noexcept;
    bool SeekIndex(const wchar_t *sIndex, int32_t nValue) noexcept;             
    bool SeekIndex(const char *sIndex, int32_t nValue) noexcept;             
    bool SeekIndex(const wchar_t *sIndex, int64_t nValue) noexcept;             
    bool SeekIndex(const char *sIndex, int64_t nValue) noexcept;   

    std::wstring GetFieldString(const wchar_t *sFieldName) const noexcept;     
    void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) noexcept; 
    std::string GetFieldStringUTF8(const char *sFieldName) const noexcept;     
    void SetFieldStringUTF8(const char *sFieldName, const char *sValue) noexcept; 

    int32_t GetFieldLong(const wchar_t *sFieldName) const noexcept;             
    int32_t GetFieldLong(const char *sFieldName) const noexcept;             
    void SetFieldLong(const wchar_t *sFieldName, int32_t nValue) noexcept;      
    void SetFieldLong(const char *sFieldName, int32_t nValue) noexcept;      

    int64_t GetFieldInt64(const wchar_t *sFieldName) const noexcept;             
    int64_t GetFieldInt64(const char *sFieldName) const noexcept;             
    void SetFieldInt64(const wchar_t *sFieldName, int64_t nValue) noexcept;      
    void SetFieldInt64(const char *sFieldName, int64_t nValue) noexcept;      

    double GetFieldDouble(const wchar_t *sFieldName) const noexcept;         
    double GetFieldDouble(const char *sFieldName) const noexcept;         
    void SetFieldDouble(const wchar_t *sFieldName, double dValue) noexcept;  
    void SetFieldDouble(const char *sFieldName, double dValue) noexcept;  

    bool GetFieldBool(const wchar_t *sFieldName) const noexcept;             
    void SetFieldBool(const wchar_t *sFieldName, bool bValue) noexcept;      
    bool GetFieldBool(const char *sFieldName) const noexcept;             
    void SetFieldBool(const char *sFieldName, bool bValue) noexcept;      

    time_t GetFieldDateTime(const wchar_t *sFieldName) const noexcept;        
    time_t GetFieldDateTime(const char *sFieldName) const noexcept;        
    void SetFieldDateTime(const wchar_t *sFieldName, time_t nValue) noexcept; 
    void SetFieldDateTime(const char *sFieldName, time_t nValue) noexcept; 

    COLORREF GetFieldRGB(const wchar_t *sFieldName) const noexcept;          
    COLORREF GetFieldRGB(const char *sFieldName) const noexcept;          
    void SetFieldRGB(const wchar_t *sFieldName, COLORREF color) noexcept;    
    void SetFieldRGB(const char *sFieldName, COLORREF color) noexcept;    
    
    bool DoesFieldExist(const wchar_t *sFieldName) noexcept;         
    bool DoesFieldExist(const char *sFieldName) noexcept;         
    bool IsFieldValueNull(const wchar_t *sFieldName) const noexcept; 
    bool IsFieldValueNull(const char *sFieldName) const noexcept; 
    void SetFieldNull(const wchar_t *sFieldName) noexcept;           
    void SetFieldNull(const char *sFieldName) noexcept;           

    void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) const noexcept; 
    void GetFieldBinary(const char *sFieldName, unsigned char **pData, size_t &nSize) const noexcept; 
    void SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize) noexcept;
    void SetFieldBinary(const char *sFieldName, unsigned char *pData, size_t nSize) noexcept;
    void FreeBinary(unsigned char *pData) noexcept;
    
    // Deletes all records with given value 
    // Returns true if any record was deleted
    bool DeleteAllByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept; // returns false in case of error or no record found
    bool DeleteAllByIndex(const char *sField, const char *sValue) noexcept;       // returns false in case of error or no record found 
    bool DeleteAllByIndex(const wchar_t *sField, int32_t nValue) noexcept;        // returns false in case of error or no record found
    bool DeleteAllByIndex(const char *sField, int32_t nValue) noexcept;           // returns false in case of error or no record found
    bool DeleteAllByIndex(const wchar_t *sField, int64_t nValue) noexcept;        // returns false in case of error or no record found
    bool DeleteAllByIndex(const char *sField, int64_t nValue) noexcept;           // returns false in case of error or no record found

    template <class TVector>
    bool DeleteAllByIndex(const char *sField, const TVector &values) noexcept {
        for (const TVector::value_type &val: values) {
            if ( !this->DeleteByIndex(sField, val) ) {
                return false;
            }
        }
        return true;
    }

    bool DeleteAllByJsonField(const char *sField, const char *sJsonField, int32_t nValue) noexcept;
    bool DeleteAllByJsonField(const char *sField, const char *sJsonField, const wchar_t *sValue) noexcept;
    bool SeekByJsonField(const char *sField, const char *sJsonField, const wchar_t *sValue) noexcept;

    bool DeleteByIndex(const wchar_t *sField, const wchar_t *sValue) noexcept;
    bool DeleteByIndex(const char *sField, const char *sValue) noexcept;
    bool DeleteByIndex(const wchar_t *sField, int32_t nValue) noexcept;
    bool DeleteByIndex(const char *sField, int32_t nValue) noexcept;
    bool DeleteByIndex(const wchar_t *sField, int64_t nValue) noexcept;
    bool DeleteByIndex(const char *sField, int64_t nValue) noexcept;

    void Flush() noexcept;  // Deletes all records in table
    bool Delete() noexcept; // returns false in case of error 

    void AddNew() noexcept;
    void Edit() noexcept;
    bool Update() noexcept; 

    int32_t GetRecordCount() noexcept; 
    int32_t GetColumnCount() noexcept;
    std::wstring GetColumnName(int32_t nCol) noexcept;
    dsFieldType GetColumnType(int32_t nCol) noexcept;

    std::wstring GetTableName() const noexcept;

    std::wstring GetUniqueTextFieldValue(const wchar_t *sFieldName, const wchar_t *sPrefix, int32_t width) noexcept; 

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
    bool IsNull##name()const noexcept { return this->IsFieldValueNull(realname); } \
    bool DoesExist##name()   noexcept { return this->DoesFieldExist(realname);   } \
    void SetNull##name()     noexcept { this->SetFieldNull(realname); }
        
// Field macros with renaming

#define FIELD_TEXT(name, realname) \
    std::wstring Get##name() const                  noexcept { return this->GetFieldString(DS_WIDEN(realname));          } \
    void Set##name(const wchar_t *sValue)           noexcept { this->SetFieldString(DS_WIDEN(realname), sValue);         } \
    void Set##name(const std::wstring &sValue)      noexcept { this->SetFieldString(DS_WIDEN(realname), sValue.c_str()); } \
    std::string Get##name##UTF8() const             noexcept { return this->GetFieldStringUTF8(realname);                } \
    void Set##name##UTF8(const char *sValue)        noexcept { this->SetFieldStringUTF8(realname, sValue);               } \
    void Set##name##UTF8(const std::string &sValue) noexcept { this->SetFieldStringUTF8(realname, sValue.c_str());       } \
    FIELD_INDICATORS(name, realname)
    
#define FIELD_LONG(name, realname) \
    int32_t Get##name() const         noexcept { return this->GetFieldLong(realname);  } \
    void    Set##name(int32_t nValue) noexcept { this->SetFieldLong(realname, nValue); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_INT64(name, realname) \
    int64_t Get##name() const         noexcept { return this->GetFieldInt64(realname);  } \
    void    Set##name(int64_t nValue) noexcept { this->SetFieldInt64(realname, nValue); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_DOUBLE(name, realname) \
    double Get##name() const      noexcept { return this->GetFieldDouble(realname);  } \
    void Set##name(double dValue) noexcept { this->SetFieldDouble(realname, dValue); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_BOOL(name, realname) \
    bool Get##name() const      noexcept { return this->GetFieldBool(realname);  } \
    void Set##name(bool bValue) noexcept { this->SetFieldBool(realname, bValue); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_BINARY(name, realname) \
    void Get##name(unsigned char **pData, size_t &nSize) const noexcept { this->GetFieldBinary(realname, pData, nSize); } \
    void Set##name(unsigned char *pData, size_t nSize)         noexcept { this->SetFieldBinary(realname, pData, nSize); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_RGB(name, realname) \
    COLORREF Get##name() const     noexcept { return this->GetFieldRGB(realname); } \
    void Set##name(COLORREF value) noexcept { this->SetFieldRGB(realname, value); } \
    FIELD_INDICATORS(name, realname)

#define FIELD_DATE(name, realname) \
    time_t Get##name() const      noexcept { return this->GetFieldDateTime(realname);  } \
    void Set##name(time_t nValue) noexcept { this->SetFieldDateTime(realname, nValue); } \
    FIELD_INDICATORS(name, realname)

// indexes
#define KEY_TEXT(name, realname) \
    bool SeekBy##name(const wchar_t *sValue)           noexcept { return this->SeekIndex(DS_WIDEN(realname), sValue);                }  \
    bool SeekBy##name(const std::wstring &sValue)      noexcept { return this->SeekIndex(DS_WIDEN(realname), sValue.c_str());        }  \
    bool SeekBy##name(const char *sValue)              noexcept { return this->SeekIndex(realname, sValue);                          }  \
    bool SeekBy##name(const std::string &sValue)       noexcept { return this->SeekIndex(realname, sValue.c_str());                  }  \
    bool DeleteAllBy##name(const wchar_t *sValue)      noexcept { return this->DeleteAllByIndex(DS_WIDEN(realname), sValue);         }  \
    bool DeleteAllBy##name(const std::wstring &sValue) noexcept { return this->DeleteAllByIndex(DS_WIDEN(realname), sValue.c_str()); }  \
    bool DeleteAllBy##name(const std::string &sValue)  noexcept { return this->DeleteAllByIndex(realname, sValue.c_str());           }  \
    bool DeleteAllBy##name(const char *sValue)         noexcept { return this->DeleteAllByIndex(realname, sValue);                   }  \
    bool DeleteBy##name(const wchar_t *sValue)         noexcept { return this->DeleteByIndex(DS_WIDEN(realname), sValue);            }  \
    bool DeleteBy##name(const std::wstring &sValue)    noexcept { return this->DeleteByIndex(DS_WIDEN(realname), sValue.c_str());    }  \
    bool DeleteBy##name(const char *sValue)            noexcept { return this->DeleteByIndex(realname, sValue);                      }  \
    bool DeleteBy##name(const std::string &sValue)     noexcept { return this->DeleteByIndex(realname, sValue.c_str());              }  \
    std::wstring GetUnique##name(const wchar_t *sPrefix, int32_t width) noexcept { return this->GetUniqueTextFieldValue(DS_WIDEN(realname), sPrefix, width); } \
    FIELD_TEXT(name, realname)
    
#define KEY_LONG(name, realname) \
    bool SeekBy##name(int32_t nValue)      noexcept { return this->SeekIndex(realname, nValue);        } \
    bool DeleteAllBy##name(int32_t nValue) noexcept { return this->DeleteAllByIndex(realname, nValue); } \
    bool DeleteBy##name(int32_t nValue)    noexcept { return this->DeleteByIndex(realname, nValue);    } \
    template <class TVector> \
    bool DeleteAllBy##name(const TVector &values) noexcept { return this->DeleteAllByIndex<TVector>(realname, values); } \
    FIELD_LONG(name, realname)

#define KEY_INT64(name, realname) \
    bool SeekBy##name(int64_t nValue)      noexcept { return this->SeekIndex(realname, nValue);        } \
    bool DeleteAllBy##name(int64_t nValue) noexcept { return this->DeleteAllByIndex(realname, nValue); } \
    bool DeleteBy##name(int64_t nValue)    noexcept { return this->DeleteByIndex(realname, nValue);    } \
    template <class TVector> \
    bool DeleteAllBy##name(const TVector &values) noexcept { return this->DeleteAllByIndex<TVector>(realname, values); } \
    FIELD_INT64(name, realname)

#endif