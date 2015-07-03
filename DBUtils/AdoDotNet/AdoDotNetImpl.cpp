#include "AdoDotNetImpl.h"
//No STDAFX because off /clr option

#using <mscorlib.dll>
#using <System.dll>
#using <System.Data.dll>

 #include "Collections/StdString.h"

#include <vcclr.h>

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace dot_net_exception_format
{
    CStdString FormatSQLException(SqlException ^e, LPCTSTR sFunctionName)
    {
        CStdString sFormatted;
        sFormatted = _T(".NET SQL Exception [");
        sFormatted += sFunctionName;
        sFormatted += _T("] - ");
        sFormatted += (CString)e->Message;

        return sFormatted;
    }

	CStdString FormatArgException(ArgumentException ^e, LPCTSTR sFunctionName)
    {
        CStdString sFormatted;
        sFormatted = _T(".NET Argument Exception [");
        sFormatted += sFunctionName;
        sFormatted += _T("] - ");
        sFormatted += (CString)e->Message;

        return sFormatted;
    }

	CStdString FormatIndexOutOfRangeException(IndexOutOfRangeException ^e, LPCTSTR sFunctionName)
	{
        CStdString sFormatted;
        sFormatted = _T(".NET IndexOutOfRange Exception [");
        sFormatted += sFunctionName;
        sFormatted += _T("] - ");
        sFormatted += (CString)e->Message;

        return sFormatted;
	}

	CStdString FormatInvalidCastException(InvalidCastException ^e, LPCTSTR sFunctionName)
	{
        CStdString sFormatted;
        sFormatted = _T(".NET InvalidCast Exception [");
        sFormatted += sFunctionName;
        sFormatted += _T("] - ");
        sFormatted += (CString)e->Message;

        return sFormatted;
	}
};

class CDotNetErrorHandler
{
// Construction/Destruction
public:
	CDotNetErrorHandler() : m_pErrorHandler(nullptr) {};
    ~CDotNetErrorHandler() { }

// Operations
public:
    void OnDotNetSQLException(SqlException ^e, LPCTSTR sFunctionName)
	{
		if ( !m_pErrorHandler ) {
			return;
		}
		const CStdString str = dot_net_exception_format::FormatSQLException(e, sFunctionName);
		(*m_pErrorHandler)(str.c_str());
	}

	void OnDotNetArgException(ArgumentException ^e, LPCTSTR sFunctionName)
	{
		if ( !m_pErrorHandler ) {
			return;
		}
		const CStdString str = dot_net_exception_format::FormatArgException(e, sFunctionName);
		(*m_pErrorHandler)(str.c_str());
	}

	void OnDotNetIndexOutOfRangeException(IndexOutOfRangeException ^e, LPCTSTR sFunctionName)
	{
		if ( !m_pErrorHandler ) {
			return;
		}
		const CStdString str = dot_net_exception_format::FormatIndexOutOfRangeException(e, sFunctionName);
		(*m_pErrorHandler)(str.c_str());
	}

	void OnDotNetInvalidCastException(InvalidCastException ^e, LPCTSTR sFunctionName)
	{
		if ( !m_pErrorHandler ) {
			return;
		}
		const CStdString str = dot_net_exception_format::FormatInvalidCastException(e, sFunctionName);
		(*m_pErrorHandler)(str.c_str());
	}

    void OnError(LPCTSTR sError, LPCTSTR sFunctionName)
	{
		if ( !m_pErrorHandler ) {
			return;
		}
		CStdString sMsg;
		sMsg.Format(_T("%s-%s"), sError, sFunctionName);
		(*m_pErrorHandler)(sMsg.c_str());
	}

    typedef void (*dbErrorHandler)(LPCTSTR msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler)
	{
		dbErrorHandler prevHandler = m_pErrorHandler;
		m_pErrorHandler = newHandler;
		return prevHandler;
	}

// Operations
private:
    dbErrorHandler m_pErrorHandler;
};

class CDotNetDatabaseImpl : public CDotNetDatabaseAbs
{
public:
	CDotNetDatabaseImpl()
	{
		m_pErrorHandler = new CDotNetErrorHandler;
	}

	~CDotNetDatabaseImpl()
	{
		delete m_pErrorHandler;
	}

public:
	virtual bool Open(LPCTSTR sConn)
	{
		SqlConnection ^pConnection = gcnew SqlConnection;
		try
		{
			pConnection->ConnectionString = gcnew String(sConn);
			pConnection->Open();
		}
		catch (ArgumentException ^aex)
		{
			m_pErrorHandler->OnDotNetArgException(aex, _T("CDotNetDatabaseImpl::Open"));
			delete aex;
			return false;
		}
		catch(SqlException ^sql_ex)
		{
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetDatabaseImpl::Open"));
			delete sql_ex;
			return false;
		}

		pConnection->Close();
		delete pConnection;

		m_sConn = sConn;

		return true;
	}

	virtual void Close()
	{
		m_sConn.Empty();
	}

	virtual bool IsOpen()
	{
		return !m_sConn.IsEmpty();
	}

	virtual bool Execute(LPCTSTR sSQL)
	{
		SqlConnection ^pConnection = gcnew SqlConnection;
		SqlCommand ^pCommand = gcnew SqlCommand();
		try
		{
			pConnection->ConnectionString = gcnew String(m_sConn);
			pConnection->Open();
			pCommand->Connection = pConnection;
			pCommand->CommandText = gcnew String(sSQL);
			pCommand->ExecuteReader();
		}
		catch (ArgumentException ^aex)
		{
			m_pErrorHandler->OnDotNetArgException(aex, _T("CDotNetDatabaseImpl::Execute"));
			delete aex;
			return false;
		}
		catch(SqlException ^sql_ex)
		{
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetDatabaseImpl::Execute"));
			delete sql_ex;
			return false;
		}
        return true;
	}

	static dsFieldType GetFieldType(Type ^field_type)
	{
		if (field_type == int().GetType())
		{
			return dsFieldType_Long;
		}

		if (field_type == long().GetType())
		{
			return dsFieldType_Long;
		}

		if (field_type == short().GetType())
		{
			return dsFieldType_Long;
		}

		if (field_type == float().GetType())
		{
			return dsFieldType_Double;
		}

		if (field_type == double().GetType())
		{
			return dsFieldType_Double;
		}

		if (field_type == bool().GetType())
		{			
			return dsFieldType_Long;
		}

		if (field_type == String(_T("")).GetType())
		{			
			return dsFieldType_Text;
		}

		if (field_type == DateTime().GetType())
		{			
			return dsFieldType_DateTime;
		}

		if (field_type == System::Guid().GetType())
		{
			return dsFieldType_Undefined;
		}

		ASSERT(FALSE);

		return dsFieldType_Undefined;
	}

	virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info)
	{
		SqlConnection ^pConnection = gcnew SqlConnection;
		SqlCommand ^pCommand = gcnew SqlCommand();
		try
		{
			pConnection->ConnectionString = gcnew String(m_sConn);
			pConnection->Open();

			pCommand->Connection = pConnection;
			CString sSqlEx;
			sSqlEx.Format(_T("SELECT TOP 0 * FROM %s WHERE 1=2"), sTable);
			pCommand->CommandText = gcnew String(sSqlEx);
			SqlDataReader ^reader = pCommand->ExecuteReader();
			
			reader->Read();

			const int nCount = reader->FieldCount;

			for(int i=0;i<nCount;i++)
			{
				const CString sColName = reader->GetName(i);sColName;
				const dsFieldType field_type = GetFieldType(reader->GetFieldType(i));
				info[CStdString(sColName)] = field_type;
			}
		}
		catch (ArgumentException ^aex)
		{
			m_pErrorHandler->OnDotNetArgException(aex, _T("CDotNetDatabaseImpl::GetTableFieldInfo"));
			delete aex;
			return false;
		}
		catch (SqlException ^sql_ex)
		{
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetDatabaseImpl::GetTableFieldInfo"));
			delete sql_ex;
			return false;
		}
        return true;
	}

	virtual CDotNetErrorHandler::dbErrorHandler SetErrorHandler(CDotNetErrorHandler::dbErrorHandler newHandler)
	{
	    ASSERT(m_pErrorHandler);
	    return m_pErrorHandler->SetErrorHandler(newHandler);
	}

	CDotNetErrorHandler *GetDotNetErrorHandler()
	{
		return m_pErrorHandler;
	}

public:
	CString GetConnection() { return m_sConn; }

private:
	CString m_sConn;
	CDotNetErrorHandler *m_pErrorHandler;
};

public class Conversions
{
public:
    static time_t DateTime2time_t(DateTime date)
	{
        TimeSpan diff = date.ToUniversalTime() - DateTime(1970, 1, 1);
        return static_cast<time_t>(diff.TotalMilliseconds);
	}
};

class CDotNetRecordSetImpl : public CDotNetRecordSetAbs
{
public:
	CDotNetRecordSetImpl(CDotNetDatabaseImpl *pDB)
	{
		m_pCommand = gcnew SqlCommand();
		m_pConnection = gcnew SqlConnection(gcnew String(pDB->GetConnection()));
		m_pConnection->Open();
		m_pCommand->Connection = m_pConnection;
		m_bOpen = false;

		m_pErrorHandler = pDB->GetDotNetErrorHandler();
	};

	~CDotNetRecordSetImpl()
	{
		//if(m_pReader != NULL && m_pReader->)
		/*try
		{
			if(!m_pReader->IsClosed)
			{
				m_pReader->Close();
				delete m_pReader;
			}
		}
		catch(NullReferenceException ^e)
		{
			TRACE((CString)e->Message);
			delete e;
		}*/

		m_pConnection->Close();

		delete m_pCommand;
		delete m_pConnection;
	};

public:
	virtual bool Open(LPCTSTR sSQL)
	{
		try
		{
			m_pCommand->CommandText = gcnew String(sSQL);
			m_pReader = m_pCommand->ExecuteReader();
			m_bIsOk = m_pReader->Read();//To get on first record
			m_bOpen = true;
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::Open"));
			delete sql_ex;
			return false;
		}

		return true;
	}

	virtual void Close()
	{
		m_pReader->Close();
		m_bIsOk = false;
		m_bOpen = false;
	}

	virtual bool IsOpen()
	{
		return m_bOpen;
	}

	virtual bool IsEmpty()
	{
		try {
			return !m_pReader->HasRows;
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::IsEmpty"));
			delete sql_ex;
		}

		return true;
	}

	virtual bool MoveNext()
	{
		try {
			m_bIsOk = m_pReader->Read();
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::MoveNext"));
			delete sql_ex;
			m_bIsOk = false;
		}

		return m_bIsOk;
	}

	virtual bool IsEOF()
	{
		return !m_bIsOk;
	}

	virtual CString GetFieldString(LPCTSTR sFieldName)
	{
		try
		{
			String ^sFieldName_ = gcnew String(sFieldName);
			int nColID = m_pReader->GetOrdinal(sFieldName_);
			delete sFieldName_;

			if (m_pReader->IsDBNull(nColID))
			{
				return _T("");
			}

			if (m_pReader->GetFieldType(nColID) == int().GetType())
			{
				return m_pReader->GetInt32(nColID).ToString();
			}

			if (m_pReader->GetFieldType(nColID) == System::Guid().GetType() ) {
				ASSERT(FALSE); //#22569 - should not go here
				return _T("");
			}

			return m_pReader->GetString(nColID);
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::GetFieldString"));
			delete sql_ex;
		}
		catch (IndexOutOfRangeException ^index_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetIndexOutOfRangeException(index_ex, _T("CDotNetRecordSetImpl::GetFieldString"));
			delete index_ex;
		}
		catch (InvalidCastException ^cast_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetInvalidCastException(cast_ex, _T("CDotNetRecordSetImpl::GetFieldString"));
			delete cast_ex;
		}

		return _T("");
	}

	virtual void SetFieldString(LPCTSTR sFieldName, LPCTSTR sValue)
	{
		ASSERT(FALSE);
	}

	virtual long GetFieldLong(LPCTSTR sFieldName)
	{
		try
		{
			String ^sFieldName_ = gcnew String(sFieldName);
			int nColID = m_pReader->GetOrdinal(sFieldName_);
			delete sFieldName_;
			if (m_pReader->IsDBNull(nColID))
			{
				return 0;
			}

			Type ^field_type = m_pReader->GetFieldType(nColID);

			if (field_type == int().GetType())
			{
				return m_pReader->GetInt32(nColID);
			}

			if (field_type == long().GetType())
			{
				return (long)m_pReader->GetInt64(nColID);
			}

			if (field_type == short().GetType())
			{
				return (long)m_pReader->GetInt16(nColID);
			}

			if (field_type == float().GetType())
			{
				return (long)m_pReader->GetFloat(nColID);
			}

			if (field_type == double().GetType())
			{
				return (long)m_pReader->GetDouble(nColID);
			}

			if (field_type == bool().GetType())
			{
				const bool bValue = m_pReader->GetBoolean(nColID);
				if(bValue)
				{
					return 1;
				}
				
				return 0;
			}
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::GetFieldLong"));
			delete sql_ex;
		}
		catch (IndexOutOfRangeException ^index_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetIndexOutOfRangeException(index_ex, _T("CDotNetRecordSetImpl::GetFieldLong"));
			delete index_ex;
		}
		catch (InvalidCastException ^cast_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetInvalidCastException(cast_ex, _T("CDotNetRecordSetImpl::GetFieldLong"));
			delete cast_ex;
		}

		ASSERT(FALSE);
		return 0;
	}

	virtual void SetFieldLong(LPCTSTR sFieldName, long lValue)
	{
		ASSERT(FALSE);
	}

	virtual double GetFieldDouble(LPCTSTR sFieldName)
	{
		try
		{
			String ^sFieldName_ = gcnew String(sFieldName);
			int nColID = m_pReader->GetOrdinal(sFieldName_);
			delete sFieldName_;
			if (m_pReader->IsDBNull(nColID))
			{
				return 0;
			}

			if (m_pReader->GetFieldType(nColID) == int().GetType())
			{
				return (double)m_pReader->GetInt32(nColID);
			}

			return m_pReader->GetDouble(nColID);
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::GetFieldDouble"));
			delete sql_ex;
		}
		catch (IndexOutOfRangeException ^index_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetIndexOutOfRangeException(index_ex, _T("CDotNetRecordSetImpl::GetFieldDouble"));
			delete index_ex;
		}
		catch (InvalidCastException ^cast_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetInvalidCastException(cast_ex, _T("CDotNetRecordSetImpl::GetFieldDouble"));
			delete cast_ex;
		}

		return 0.0;
	}

	virtual void SetFieldDouble(LPCTSTR sFieldName, double dValue)
	{
		ASSERT(FALSE);
	}

	virtual time_t GetFieldDateTime(LPCTSTR sFieldName)
	{
		try
		{
			String ^sFieldName_ = gcnew String(sFieldName);
			int nColID = m_pReader->GetOrdinal(sFieldName_);
			DateTime date = m_pReader->GetDateTime(nColID);
			delete sFieldName_;
			return Conversions::DateTime2time_t(date);
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::GetFieldDateTime"));
			delete sql_ex;
		}
		catch (IndexOutOfRangeException ^index_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetIndexOutOfRangeException(index_ex, _T("CDotNetRecordSetImpl::GetFieldDateTime"));
			delete index_ex;
		}
		catch (InvalidCastException ^cast_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetInvalidCastException(cast_ex, _T("CDotNetRecordSetImpl::GetFieldDateTime"));
			delete cast_ex;
		}
		
		return 0;
	}

	virtual void SetFieldDateTime(LPCTSTR sFieldName, const time_t &time)
	{
		ASSERT(FALSE);
	}

	virtual bool IsFieldValueNull(LPCTSTR sFieldName)
	{
		try
		{
			String ^sFieldName_ = gcnew String(sFieldName);
			int nColID = m_pReader->GetOrdinal(sFieldName_);
			delete sFieldName_;
			if (m_pReader->IsDBNull(nColID))
			{
				return true;
			}
		}
		catch (SqlException ^sql_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetSQLException(sql_ex, _T("CDotNetRecordSetImpl::IsFieldValueNull"));
			delete sql_ex;
		}
		catch (IndexOutOfRangeException ^index_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetIndexOutOfRangeException(index_ex, _T("CDotNetRecordSetImpl::IsFieldValueNull"));
			delete index_ex;
		}
		catch (InvalidCastException ^cast_ex)
		{
			ASSERT(FALSE);
			m_pErrorHandler->OnDotNetInvalidCastException(cast_ex, _T("CDotNetRecordSetImpl::IsFieldValueNull"));
			delete cast_ex;
		}

		return false;
	}

private:
	gcroot<SqlCommand^> m_pCommand;
	gcroot<SqlDataReader^> m_pReader;
	gcroot<SqlConnection^> m_pConnection;

	bool m_bIsOk;
	bool m_bOpen;

	CDotNetErrorHandler *m_pErrorHandler;
};

CDotNetDatabaseAbs *CAdoDotNetUtils::CreateDatabase()
{
	return new CDotNetDatabaseImpl;
}

void CAdoDotNetUtils::DeleteDatabase(CDotNetDatabaseAbs *pDB)
{
	CDotNetDatabaseImpl *pDotNetDB = (CDotNetDatabaseImpl *)pDB;
	ASSERT(pDotNetDB);
	delete pDotNetDB;
};

CDotNetRecordSetAbs *CAdoDotNetUtils::CreateRecordSet(CDotNetDatabaseAbs *pDB)
{
	CDotNetDatabaseImpl *pDotNetDB = dynamic_cast<CDotNetDatabaseImpl *>(pDB);
	ASSERT(pDotNetDB);
	return new CDotNetRecordSetImpl(pDotNetDB);
}

void CAdoDotNetUtils::DeleteRecordSet(CDotNetRecordSetAbs *pSet)
{
	CDotNetRecordSetImpl *pDotNetSet = dynamic_cast<CDotNetRecordSetImpl *>(pSet);
	ASSERT(pDotNetSet);
	delete pDotNetSet;
}