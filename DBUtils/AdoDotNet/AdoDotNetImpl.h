#ifndef __ADO_DOT_NET_IMPL_H__
#define __ADO_DOT_NET_IMPL_H__
#pragma once

//No STDAFX because off /clr option
#ifndef __PRAGMA_SET_H__
	#include "Collections/PragmaSet.h"
#endif

#ifndef __AFXWIN_H__
	#include "afx.h"
#endif

#ifndef __ABS_DATABASE_H__
	#include "../AbsDatabase.h"
#endif

class CDotNetDatabaseAbs
{
public:
	virtual bool Open(LPCTSTR sConn) = 0;
	virtual void Close() = 0;
	virtual bool Execute(LPCTSTR sSQL) = 0;
	virtual bool IsOpen() = 0;
	typedef void (*dbErrorHandler)(LPCTSTR msg); 
	virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) = 0;

	virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info) = 0;
};

class CDotNetRecordSetAbs
{
public:
	virtual bool Open(LPCTSTR sTableName) = 0;
	virtual bool IsOpen() = 0;
	virtual bool IsEmpty() = 0;
	virtual void Close() = 0;

	virtual bool IsEOF() = 0;

	virtual bool MoveNext() = 0;

	virtual CString GetFieldString(LPCTSTR sFieldName) = 0;
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue) = 0;
	
	virtual long GetFieldLong(LPCTSTR sFieldName) = 0;
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue) = 0;

	virtual double GetFieldDouble(LPCTSTR sFieldName) = 0;
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue) = 0;

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName) = 0;
	virtual void SetFieldDateTime(LPCTSTR sFieldName, const time_t &time) = 0;

	virtual bool IsFieldValueNull(LPCTSTR sFieldName) = 0;
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
