#ifndef __ADO_DOT_NET_IMPL_H__
#define __ADO_DOT_NET_IMPL_H__
#pragma once

#ifndef __x86_64__ 
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0501 // Windows (VC 10 MFC requires minimal define)
    #endif
    #ifndef __AFXWIN_H__
        #include "afx.h"
    #endif
#endif

#ifndef __ABS_DATABASE_H__
    #include "../AbsDatabase.h"
#endif

class CDotNetDatabaseAbs
{
public:
    virtual bool Open(const wchar_t *sConn) = 0;
    virtual void Close() = 0;
    virtual bool Execute(const wchar_t *sSQL) = 0;
    virtual bool IsOpen() = 0;
    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) = 0;

    virtual bool GetTableFieldInfo(const wchar_t *sTable, dsTableFieldInfo &info) = 0;

    virtual bool DoesTableExists(const wchar_t *sTable) = 0;
};

class CDotNetRecordSetAbs
{
public:
    virtual bool Open(const wchar_t *sSQL) = 0;
    virtual bool SeekByString(const wchar_t *sSQL, const wchar_t *sValue) = 0;
    virtual bool IsOpen() = 0;
    virtual bool IsEmpty() = 0;
    virtual void Close() = 0;

    virtual bool IsEOF() = 0;

    virtual bool MoveNext() = 0;

    virtual std::wstring GetFieldString(const wchar_t *sFieldName) = 0;
    virtual void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) = 0;
    
    virtual long GetFieldLong(const wchar_t *sFieldName) = 0;
    virtual void SetFieldLong(const wchar_t *sFieldName, long lValue) = 0;

    virtual double GetFieldDouble(const wchar_t *sFieldName) = 0;
    virtual void SetFieldDouble(const wchar_t *sFieldName, double dValue) = 0;

    virtual time_t GetFieldDateTime(const wchar_t *sFieldName) = 0;
    virtual void SetFieldDateTime(const wchar_t *sFieldName, const time_t &time) = 0;

    virtual bool IsFieldValueNull(const wchar_t *sFieldName) = 0;

    virtual bool DoesFieldExist(const wchar_t *sFieldName) = 0;
};

class CAdoDotNetUtils
{
public:
    static CDotNetDatabaseAbs *CreateDatabase();
    static void DeleteDatabase(CDotNetDatabaseAbs *pDB);

    static CDotNetRecordSetAbs *CreateRecordSet(CDotNetDatabaseAbs *pDB);
    static void DeleteRecordSet(CDotNetRecordSetAbs *pSet);
};

#endif 
