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
	virtual bool Open(LPCTSTR sTableName) override;
	virtual bool OpenSQL(LPCTSTR sSQL) override;
	virtual bool OpenView(LPCTSTR sViewName) override;

	virtual bool MoveNext() override;
	virtual bool MoveFirst() override;

	virtual long GetRecordCount() override;

	virtual bool Delete() override;
	virtual void AddNew() override;
	virtual void Edit() override;
	virtual bool Update() override;

	virtual bool IsEOF() override;

	virtual void SetFieldBinary(LPCTSTR sFieldName, unsigned char *pData, unsigned long nSize) override;
	virtual void GetFieldBinary(LPCTSTR sFieldName, unsigned char **pData, unsigned long &nSize) override;
    virtual void FreeBinary(unsigned char *pData) override;

	virtual void SetFieldValueNull(LPCTSTR lpszName) override;

	virtual CStdString GetFieldString(LPCTSTR sFieldName) override;
	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue) override;

    virtual std::string GetFieldStringUTF8(const char *sFieldName) override;
	virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) override;

	virtual long GetFieldLong(LPCTSTR sFieldName) override;
	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue) override;

	virtual double GetFieldDouble(LPCTSTR sFieldName) override;
	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue) override;

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName) override;
	virtual void   SetFieldDateTime(LPCTSTR sFieldName, const time_t &time) override;
	virtual bool IsFieldValueNull(LPCTSTR sFieldName) override;

	virtual bool DoesFieldExist(LPCTSTR sFieldName) override;

	virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue) override;
	virtual bool SeekByLong(LPCTSTR sIndex, long nValue) override;

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
