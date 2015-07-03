#include "StdAfx.h"
#include "SqLiteUtil.h"

#include "sqlite3.h"

#include "SqLiteRecordsetImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Convert a wide Unicode string to an UTF8 string
namespace sqlite_conv
{
    std::string ConvertToUTF8(const wchar_t *wstr)
    {
        const int nLen = wcslen(wstr);
        if( nLen <= 0  ) {
            return std::string();
        }

        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, NULL, 0, NULL, NULL);

        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, &strTo[0], size_needed, NULL, NULL);

        return strTo;
    }

    // Convert an UTF8 string to a wide Unicode String
    CStdString ConvertFromUTF8(const char *str)
    {
        if ( !str ) {
            return CStdString();
        }

        const int nLen = strlen(str);
        if ( nLen <= 0 ) {
            return CStdString();
        }

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, NULL, 0);

        CStdString wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, &wstrTo[0], size_needed);

        return wstrTo;
    }
};

namespace sqlite_util
{
    class CDeletor  {
    public:
        template <class TPair>
        void operator()(TPair &pair) {
            delete pair.second;
        }
    };

    void CFieldDataMap::clear() {
        std::for_each(begin(), end(), CDeletor());
        std::unordered_map<std::string, CFieldData *>::clear();
    }

    void BindStatements(CFieldDataMap &data_map, sqlite3_stmt *pStmt)
    {
        auto end_it = data_map.end();
        auto beg_it = data_map.begin();

        int nIndex = 1;
        for (auto it = beg_it; it != end_it; ++it) {
            if ( it->second->Bind(pStmt, nIndex) != SQLITE_OK )
            {
                ASSERT(FALSE);
            }
            ++nIndex;
        }
    }

    CFieldDataBinary::CFieldDataBinary(unsigned char *pData, unsigned long nSize) 
    {
        m_pData = new unsigned char[nSize];
        memcpy(m_pData, pData, nSize);
        m_nSize = nSize;
    }

    CFieldDataBinary::~CFieldDataBinary() 
    {
        delete [] m_pData;
    }

    int CFieldDataBinary::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        return ::sqlite3_bind_blob(pStmt, nIndex, m_pData, m_nSize, SQLITE_STATIC);   
    }

    int CFieldDataText::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        // if text length is negative, then the length of the string is the number of bytes up to the first zero terminator.
        return ::sqlite3_bind_text(pStmt, nIndex, m_sText.c_str(), -1, SQLITE_STATIC);                            
    }

    int CFieldDataLong::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        return ::sqlite3_bind_int(pStmt, nIndex, m_nValue);                            
    }

    int CFieldDataDouble::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        return ::sqlite3_bind_double(pStmt, nIndex, m_dValue);                            
    }

    int CFieldDataDateTime::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        return ::sqlite3_bind_int64(pStmt, nIndex, m_time);
    }

    int CFieldDataNull::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        return sqlite3_bind_null(pStmt, nIndex);
    }

    eFieldType CFieldInfo::GetFieldType() const
    {
        CStdStringA sType = m_sType;
        sType.ToUpper();
        if ( sType == "INTEGER" ) {
            return eFieldType_Long;
        }

        if ( sType == "TEXT" ) {
            return eFieldType_Text;
        }

        if ( sType == "REAL" ) {
            return eFieldType_Double;
        }

        if ( sType == "BLOB" ) {
            return eFieldType_Binary;
        }

        if ( sType == "DATE" ) {
            return eFieldType_Long;
        }
        if ( sType == "DATETIME" ) {
            return eFieldType_Long;
        }
        // If the declared type contains the string "INT" then it is assigned INTEGER affinity.
        if ( sType == "INT" ) {
            return eFieldType_Long;
        }
        if ( sType == "INTEGER" ) {
            return eFieldType_Long;
        }
        if ( sType == "TINYINT" ) {
            return eFieldType_Long;
        }
        if ( sType == "SMALLINT" ) {
            return eFieldType_Long;
        }
        if ( sType == "MEDIUMINT" ) {
            return eFieldType_Long;
        }
        if ( sType == "BIGINT" ) {
            return eFieldType_Long;
        }
        if ( sType == "UNSIGNED BIG INT" ) {
            return eFieldType_Long;
        }
        if ( sType == "INT2" ) {
            return eFieldType_Long;
        }
        if ( sType == "INT8" ) {
            return eFieldType_Long;
        }

        // If the declared type of the column contains any of the strings "CHAR", "CLOB", or "TEXT" 
        // then that column has TEXT affinity. Notice that the type VARCHAR contains the string "CHAR" and is thus assigned TEXT affinity.
        if ( sType == "CHARACTER" ) {
            return eFieldType_Text;
        }
        if ( sType == "VARCHAR" ) {
            return eFieldType_Text;
        }
        if ( sType == "VARYING CHARACTER" ) {
            return eFieldType_Text;
        }
        if ( sType == "NCHAR" ) {
            return eFieldType_Text;
        }
        if ( sType == "NATIVE CHARACTER" ) {
            return eFieldType_Text;
        }
        if ( sType == "NVARCHAR" ) {
            return eFieldType_Text;
        }
        if ( sType == "CHAR" ) {
            return eFieldType_Text;
        }
        if ( sType == "CLOB" ) {
            return eFieldType_Text;
        }
        //SQLite does not have a separate Boolean storage class. Instead, Boolean values are stored as integers 0 (false) and 1 (true).
        if ( sType == "BOOLEAN" ) {
            return eFieldType_Long;
        }
        // If the declared type for a column contains any of the strings "REAL", "FLOA", or "DOUB" then the column has REAL affinity
        if ( sType == "NUMERIC " ) {
            return eFieldType_Double;
        }
        if ( sType == "DECIMAL " ) {
            return eFieldType_Double;
        }
        if ( sType == "FLOAT " ) {
            return eFieldType_Double;
        }
        if ( sType == "DOUBLE" ) {
            return eFieldType_Double;
        }
        if ( sType == "DOUBLE PRECISION " ) {
            return eFieldType_Double;
        }

        ASSERT(FALSE);
        return eFieldType_Undefined;
    }

    bool GetTableFieldsdInfo(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler, CFieldInfoMap &map)
    {
        ASSERT(strlen(sTableName) > 0);

        CStdStringA sPragma;
        sPragma.Format("PRAGMA table_info(%s)", sTableName);

        CSqLiteRecordsetImpl loader(pDB, pErrorHandler);

        if ( !loader.OpenSQLUTF8(sPragma.c_str()) ) {
            return false;
        }

        if ( !loader.MoveFirst() ) {
            return false;
        }

        while ( !loader.IsEOF() ) 
        {
            CFieldInfo field_info;
            const std::string sColumn = loader.GetFieldStringUTF8(_T("name"));
            field_info.m_sType    = loader.GetFieldStringUTF8(_T("type"));
            field_info.m_bNotNull = loader.GetFieldLong(_T("notnull")) == 1;
            field_info.m_sDefault = loader.GetFieldStringUTF8(_T("dflt_value"));
            field_info.m_bPrimary = loader.GetFieldLong(_T("pk")) == 1;
            map[sColumn] = field_info;

            loader.MoveNext();
        }

        return true;
    }
};