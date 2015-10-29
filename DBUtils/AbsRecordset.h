#ifndef __ABS_RECORD_SET_H__
#define __ABS_RECORD_SET_H__
#pragma once

#ifndef STDSTRING_H
    #include "Collections/StdString.h"
#endif

#include "vector"

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

	virtual long GetRecordCount() = 0;

	virtual bool Delete() = 0;
	virtual void AddNew() = 0;
	virtual void Edit()	  = 0;
	virtual bool Update() = 0;

	virtual bool Open(LPCTSTR sTableName)    = 0;
	virtual bool OpenSQL(LPCTSTR sSQL)       = 0;
	virtual bool OpenView(LPCTSTR sViewName) = 0;

    virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue) = 0;
	virtual bool SeekByLong(LPCTSTR sIndex,   long nValue)    = 0;

	virtual void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize)   = 0;
	virtual void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize) = 0;
    virtual void FreeBinary(unsigned char *pData) = 0;

	virtual void SetFieldValueNull(LPCTSTR lpszName)  = 0;
    virtual bool IsFieldValueNull(LPCTSTR sFieldName) = 0;

	virtual CStdString GetFieldString(LPCTSTR sFieldName) = 0;
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue) = 0;

    virtual std::string GetFieldStringUTF8(const char *sFieldName) = 0;
	virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) = 0;

	virtual long GetFieldLong(LPCTSTR sFieldName) = 0;
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue) = 0;

	virtual double GetFieldDouble(LPCTSTR sFieldName) = 0;
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue) = 0;

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName) = 0;
	virtual void   SetFieldDateTime(LPCTSTR sFieldName, const time_t &time) = 0;

	virtual bool DoesFieldExist(LPCTSTR sFieldName) = 0; 

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
    virtual bool DeleteAllByStringValue(LPCTSTR sField, LPCTSTR sValue)
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

    virtual bool DeleteAllByLongValue(LPCTSTR sField, long nValue)
    {
        if ( !SeekByLong(sField, nValue) ) {
            return false;
        }

        bool bRetVal = false;
	    while ( !IsEOF() && GetFieldLong(sField) == nValue ) {
		    VERIFY(Delete());
		    MoveNext();
		    bRetVal = true;
	    }

	    return bRetVal;
    }

    virtual bool DeleteByLongValue(LPCTSTR sField, long nValue)
    {
        if ( !SeekByLong(sField, nValue) ) {
            return false; 
        }

        if ( !Delete() ) {
            return false; 
        }

        return true;
    }

    virtual bool DeleteByStringValue(LPCTSTR sField, LPCTSTR sValue)
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
