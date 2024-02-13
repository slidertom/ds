#ifndef __ABS_RECORD_SET_H__
#define __ABS_RECORD_SET_H__
#pragma once

#ifndef __DS_TYPES_H__
    #include "dsTypes.h"
#endif

#ifndef __DS_STR_CONV_H__
    #include "dsStrConv.h"
#endif

#include <string>

class CAbsRecordset
{
// Construction/Destruction
public:
    CAbsRecordset() { }
    virtual ~CAbsRecordset() { }

// Overrides
public:
    virtual bool MoveNext()  = 0;
    virtual bool MoveFirst() = 0;
    virtual bool IsEOF()     = 0;

    virtual int GetRecordCount() const = 0;
    virtual int GetColumnCount() const = 0;
    virtual std::wstring GetColumnName(int nCol) const  = 0;
    virtual dsFieldType GetColumnType(int nCol) const = 0;

    virtual bool Delete() = 0;
    virtual void AddNew() = 0;
    virtual void Edit()   = 0;
    virtual bool Update() = 0;

    virtual bool Open(const wchar_t *sTableName)    = 0;
    virtual bool OpenSQL(const wchar_t *sSQL)       = 0;
    virtual bool OpenView(const wchar_t *sViewName) = 0;

    virtual bool SeekByString(const wchar_t *sIndex, const wchar_t *sValue) = 0;
    virtual bool SeekByString(const char    *sIndex, const char *sValue)    = 0;
    virtual bool SeekByLong(const wchar_t   *sIndex, int32_t nValue)        = 0;
    virtual bool SeekByLong(const char      *sIndex, int32_t nValue)        = 0;
    virtual bool SeekByInt64(const wchar_t  *sIndex, int64_t nValue) { return this->SeekByLong(sIndex, (int32_t)nValue); }
    virtual bool SeekByInt64(const char     *sIndex, int64_t nValue) { return this->SeekByLong(sIndex, (int32_t)nValue); }

    virtual void SetFieldBinary(const wchar_t *sFieldName, unsigned char  *pData, size_t nSize)  = 0;
    virtual void SetFieldBinary(const char *sFieldName, unsigned char  *pData, size_t nSize) {
        const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
        this->SetFieldBinary(sFieldNameUTF16.c_str(), pData, nSize);
    }
    virtual void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) = 0;
    virtual void GetFieldBinary(const char *sFieldName, unsigned char **pData, size_t &nSize) {
        const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
        this->GetFieldBinary(sFieldNameUTF16.c_str(), pData, nSize);
    }
    virtual void FreeBinary(unsigned char *pData) = 0;

    virtual void SetFieldValueNull(const char *sFieldName) {
        const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
        this->SetFieldValueNull(sFieldNameUTF16.c_str());
    }
    virtual void SetFieldValueNull(const wchar_t *sFieldName)  = 0;
    virtual bool IsFieldValueNull(const wchar_t *sFieldName)   = 0;
    virtual bool IsFieldValueNull(const char *sFieldName)      = 0;

    virtual std::wstring GetFieldString(const wchar_t *sFieldName)                = 0;
    virtual void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) = 0;

    virtual std::string GetFieldStringUTF8(const char *sFieldName)              = 0;
    virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) = 0;

    virtual int32_t GetFieldInt32(const wchar_t *sFieldName)              = 0;
    virtual int32_t GetFieldInt32(const char *sFieldName)                 = 0;
    virtual void SetFieldInt32(const wchar_t *sFieldName, int32_t lValue) = 0;
    virtual void SetFieldInt32(const char *sFieldName, int32_t lValue)    = 0;

    virtual int64_t GetFieldInt64(const wchar_t *sFieldName)              { return GetFieldInt32(sFieldName);           }
    virtual void SetFieldInt64(const wchar_t *sFieldName, int64_t lValue) { SetFieldInt32(sFieldName, (int32_t)lValue); }

    virtual int64_t GetFieldInt64(const char *sFieldName)              { return GetFieldInt32(sFieldName);           }
    virtual void SetFieldInt64(const char *sFieldName, int64_t lValue) { SetFieldInt32(sFieldName, (int32_t)lValue); }

    virtual double GetFieldDouble(const wchar_t *sFieldName)              = 0;
    virtual double GetFieldDouble(const char *sFieldName) {
        const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
        return this->GetFieldDouble(sFieldNameUTF16.c_str());
    }
    virtual void SetFieldDouble(const wchar_t *sFieldName, double dValue) = 0;
    virtual void SetFieldDouble(const char *sFieldName, double dValue) {
        const std::wstring sFieldNameUTF16 = ds_str_conv::ConvertFromUTF8(sFieldName);
        this->SetFieldDouble(sFieldNameUTF16.c_str(), dValue);
    }

    virtual time_t GetFieldDateTime(const wchar_t *sFieldName)                     = 0;
    virtual time_t GetFieldDateTime(const char *sFieldName)                        = 0;
    virtual void   SetFieldDateTime(const wchar_t *sFieldName, const time_t &time) = 0;
    virtual void   SetFieldDateTime(const char *sFieldName, const time_t &time)    = 0;

    virtual bool DoesFieldExist(const char *sFieldName) = 0;
    virtual bool DoesFieldExist(const wchar_t *sFieldName) = 0; 

// Default realizations - do override if it's required
public:
    virtual void Flush()
    {
        if ( !MoveFirst() ) {
            return;
        }
        
        while ( !IsEOF() ) {
            VERIFY(Delete());
            MoveNext();
        }
    }

    // returns true if any record was been deleted
    virtual bool DeleteAllByStringValue(const wchar_t *sField, const wchar_t *sValue)
    {
        if ( !SeekByString(sField, sValue) ) {
            return false;
        }

        bool bRetVal = false;
        while ( !IsEOF() && GetFieldString(sField) == sValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }

        return bRetVal;
    }

    virtual bool DeleteAllByStringValueUTF8(const char *sField, const char *sValue)
    {
        if ( !SeekByString(sField, sValue) ) {
            return false;
        }

        bool bRetVal = false;
        while ( !IsEOF() && GetFieldStringUTF8(sField) == sValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }

        return bRetVal;
    }

    virtual bool DeleteAllByInt64Value(const wchar_t *sField, int64_t nValue)
    {
        if ( !SeekByInt64(sField, nValue) ) {
            return false;
        }

        bool bRetVal = false;
        while ( !IsEOF() && GetFieldInt64(sField) == nValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }

        return bRetVal;
    }

    virtual bool DeleteAllByInt64ValueUTF8(const char *sField, int64_t nValue) 
    {
        if ( !SeekByInt64(sField, nValue) ) {
            return false;
        }

        bool bRetVal = false;
        while ( !IsEOF() && GetFieldInt64(sField) == nValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }

        return bRetVal;
    }

    virtual bool DeleteAllByLongValue(const wchar_t *sField, int32_t nValue) {
        if ( !SeekByLong(sField, nValue) ) {
            return false;
        }
        bool bRetVal = false;
        while ( !IsEOF() && GetFieldInt32(sField) == nValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }
        return bRetVal;
    }

    virtual bool DeleteAllByLongValueUTF8(const char *sField, int32_t nValue) {
        if ( !SeekByLong(sField, nValue) ) {
            return false;
        }
        bool bRetVal = false;
        while ( !IsEOF() && GetFieldInt32(sField) == nValue ) {
            VERIFY(Delete());
            MoveNext();
            bRetVal = true;
        }
        return bRetVal;
    }

    virtual bool DeleteAllByJsonField(const char *sField, const char *sFieldInJson, int32_t nValue) {
        return false;
    }

    virtual bool DeleteAllByJsonField(const char *sField, const char *sFieldInJson, const wchar_t *sValue) {
        return false;
    }

    virtual bool SeekByJsonField(const char *sField, const char *sFieldInJson, const wchar_t *sValue) {
        return false;
    }
   
    virtual bool DeleteByLongValue(const wchar_t *sField, int32_t nValue)
    {
        if ( !SeekByLong(sField, nValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }
        return true;
    }

    virtual bool DeleteByLongValue(const char *sField, int32_t nValue)
    {
        if ( !SeekByLong(sField, nValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }
        return true;
    }

    virtual bool DeleteByStringValue(const char *sField, const char *sValue)
    {
        if ( !SeekByString(sField, sValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }
        return true;
    }

    virtual bool DeleteByInt64Value(const wchar_t *sField, int64_t nValue)
    {
        if ( !SeekByInt64(sField, nValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }

        return true;
    }

    virtual bool DeleteByInt64Value(const char *sField, int64_t nValue)
    {
        if ( !SeekByInt64(sField, nValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }

        return true;
    }

    virtual bool DeleteByStringValue(const wchar_t *sField, const wchar_t *sValue)
    {
        if ( !SeekByString(sField, sValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }
        return true;
    }

    virtual bool DeleteByStringValueUTF8(const char *sField, const char *sValue)
    {
        if ( !SeekByString(sField, sValue) ) {
            return false; 
        }
        if ( !Delete() ) {
            return false; 
        }
        return true;
    }
};

#endif