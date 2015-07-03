#ifndef __ADO_DOT_NET_RECORD_SET_IMPL_H__
#define __ADO_DOT_NET_RECORD_SET_IMPL_H__
#pragma once

#ifndef __ABS_RECORD_SET_H__
	#include "../AbsRecordset.h"
#endif

class CDotNetDatabaseAbs;
class CDotNetRecordSetAbs;

class CAdoDotNetRecordsetImpl : public CAbsRecordset
{
// Construction/Destruction
public:
	CAdoDotNetRecordsetImpl(CDotNetDatabaseAbs *pDatabase);
	virtual ~CAdoDotNetRecordsetImpl();

// Overrides
public:
	virtual bool Open(LPCTSTR sTableName);
	virtual bool OpenSQL(LPCTSTR sSQL);
	virtual bool OpenView(LPCTSTR sViewName);

	virtual bool MoveNext();
	virtual bool MoveFirst();

	virtual long GetRecordCount();

	virtual bool Delete();
	virtual void AddNew();
	virtual void Edit();
	virtual bool Update();

	virtual bool IsEOF();

	virtual void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize);
	virtual void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize);

	virtual void SetFieldValueNull(LPCTSTR lpszName);

	virtual CStdString GetFieldString(LPCTSTR sFieldName);
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue);

	virtual long GetFieldLong(LPCTSTR sFieldName);
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue);

	virtual double GetFieldDouble(LPCTSTR sFieldName);
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue);

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName);
	virtual void   SetFieldDateTime(LPCTSTR sFieldName, const time_t &time);
	virtual bool IsFieldValueNull(LPCTSTR sFieldName);

	virtual bool DoesFieldExist(LPCTSTR sFieldName);

	virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue);
	virtual bool SeekByLong(LPCTSTR sIndex, long nValue);

private:
	void OpenImpl();
	bool IsOpen() const;

// Attributes
private:
	CString m_sTable;

private:
	CDotNetRecordSetAbs *m_pSet;
};

#endif 
