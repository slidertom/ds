#ifndef __DAO_RECORD_SET_IMPL_H__
#define __DAO_RECORD_SET_IMPL_H__
#pragma once

#ifndef __ABS_RECORD_SET_H__
	#include "../AbsRecordset.h"
#endif

// Remarks:
// 1. Sort doesn't work with dbOpenTable
// 2. GetRecordCount() not always works correctly, please do some testing before using this

class CDaoDatabaseImpl;
class CDaoQueryDef;
class CDaoRecordset;
class CDaoDatabaseImpl;
class CDaoException;
class CDaoErrorHandler;

class CDaoRecordsetImpl : public CAbsRecordset
{
// Construction/Destruction
public:
    CDaoRecordsetImpl(CDaoDatabaseImpl *pDatabase, CDaoErrorHandler *pErrorHandler);
	virtual ~CDaoRecordsetImpl();

// Overrides
public:
	virtual bool Open(LPCTSTR sTableName);
	virtual bool OpenSQL(LPCTSTR sSQL);
	virtual bool OpenView(LPCTSTR sViewName);

    virtual bool SeekByString(LPCTSTR sIndex, LPCTSTR sValue);
	virtual bool SeekByLong(LPCTSTR sIndex, long nValue);

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
	virtual void SetFieldDateTime(LPCTSTR sFieldName, const time_t &time);

	virtual bool IsFieldValueNull(LPCTSTR sFieldName);

	virtual bool DoesFieldExist(LPCTSTR sFieldName);

private:
	bool IsOpen() const;
	void SetIndex(LPCTSTR sIndex);
    void DoOnDaoException(CDaoException *e, LPCTSTR sFunction);
    
// Attributes
private:
	CString m_sCurIndex;
	CDaoQueryDef *m_pTableDef;
	CDaoRecordset *m_pSet;
    CDaoErrorHandler *m_pErrorHandler;
    CDaoDatabaseImpl *m_pDatabase;
};

#endif 
