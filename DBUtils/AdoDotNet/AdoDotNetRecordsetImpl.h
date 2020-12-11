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
    virtual bool Open(const wchar_t *sTableName) override;
    virtual bool OpenSQL(const wchar_t *sSQL) override;
    virtual bool OpenView(const wchar_t *sViewName) override;

    virtual bool MoveNext() override;
    virtual bool MoveFirst() override;

    virtual int GetRecordCount() const override;
    virtual int GetColumnCount() const override;
    virtual std::wstring GetColumnName(int nCol) const override;
    virtual dsFieldType GetColumnType(int nCol) const override;

    virtual bool Delete() override;
    virtual void AddNew() override;
    virtual void Edit() override;
    virtual bool Update() override;

    virtual bool IsEOF() override;

    virtual void SetFieldBinary(const wchar_t *sFieldName, unsigned char *pData, size_t nSize) override;
    virtual void GetFieldBinary(const wchar_t *sFieldName, unsigned char **pData, size_t &nSize) override;
    virtual void FreeBinary(unsigned char *pData) override;

    virtual void SetFieldValueNull(const wchar_t *lpszName) override;

    virtual std::wstring GetFieldString(const wchar_t *sFieldName) override;
    virtual void SetFieldString(const wchar_t *sFieldName, const wchar_t *sValue) override;

    virtual std::string GetFieldStringUTF8(const char *sFieldName) override;
    virtual void SetFieldStringUTF8(const char *sFieldName, const char *sValue) override;

    virtual int32_t  GetFieldInt32(const wchar_t *sFieldName) override;
    virtual int32_t  GetFieldInt32(const char *sFieldName) override;
    virtual void SetFieldInt32(const wchar_t *sFieldName, int32_t lValue) override;
    virtual void SetFieldInt32(const char *sFieldName, int32_t lValue)  override;

    virtual double GetFieldDouble(const wchar_t *sFieldName) override;
    virtual void SetFieldDouble(const wchar_t *sFieldName, double dValue) override;

    virtual time_t GetFieldDateTime(const wchar_t *sFieldName) override;
    virtual void   SetFieldDateTime(const wchar_t *sFieldName, const time_t &time) override;
    virtual bool IsFieldValueNull(const wchar_t *sFieldName) override;

    virtual bool DoesFieldExist(const wchar_t *sFieldName) override;

    virtual bool SeekByString(const wchar_t *sIndex, const wchar_t *sValue) override;
    virtual bool SeekByString(const char *sIndex, const char *sValue)       override;
    virtual bool SeekByLong(const wchar_t *sIndex, int32_t nValue)          override;
    virtual bool SeekByLong(const char    *sIndex, int32_t nValue)          override;

private:
    void OpenImpl();
    bool IsOpen() const;

// Attributes
private:
    std::wstring m_sTable;

private:
    CDotNetRecordSetAbs *m_pSet;
    CDotNetDatabaseAbs *m_pDB;

    bool m_bSQLOpened {false};
};

#endif 
