#include "StdAfx.h"
#include "sqlite_bind_util.h"

#include "SqLiteRecordsetImpl.h"
#include "sqlite_include.h"
#include "SqLiteDatabaseImpl.h" 

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace sqlite_util
{
    void CFieldDataMap::clear() {
        for ( auto elem : *this ) {
            delete elem.second;
        }
        std::unordered_map<std::string, CFieldData *>::clear();
    }

    int sqlite_bind_statements(CFieldDataMap &data_map, sqlite3_stmt *pStmt)
    {
        auto end_it = data_map.end();
        auto beg_it = data_map.begin();

        int nIndex = 1;
        for (auto it = beg_it; it != end_it; ++it) {
            if ( it->second->Bind(pStmt, nIndex) != SQLITE_OK ) {
                ASSERT(FALSE);
            }
            ++nIndex;
        }
        return nIndex;
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

    int CFieldDataBinary::Bind(sqlite3_stmt *pStmt, int nIndex) {
        return ::sqlite3_bind_blob(pStmt, nIndex, m_pData, m_nSize, SQLITE_STATIC);   
    }

    int CFieldDataText::Bind(sqlite3_stmt *pStmt, int nIndex)
    {
        // if text length is negative, then the length of the string is the number of bytes up to the first zero terminator.
        return ::sqlite3_bind_text(pStmt, nIndex, m_sText.c_str(), -1, SQLITE_STATIC);                            
    }

    int CFieldDataLong::Bind(sqlite3_stmt *pStmt, int nIndex) {
        return ::sqlite3_bind_int(pStmt, nIndex, m_nValue);                            
    }

    int CFieldDataDouble::Bind(sqlite3_stmt *pStmt, int nIndex) {
        return ::sqlite3_bind_double(pStmt, nIndex, m_dValue);                            
    }

    int CFieldDataDateTime::Bind(sqlite3_stmt *pStmt, int nIndex) {
        return ::sqlite3_bind_int64(pStmt, nIndex, m_time);
    }

    int CFieldDataNull::Bind(sqlite3_stmt *pStmt, int nIndex) {
        return sqlite3_bind_null(pStmt, nIndex);
    }

    std::string CFieldDataLong::GetValueAsString() {
        return ds_str_conv::int_to_string(m_nValue);
    }
    std::string CFieldDataDouble::GetValueAsString() {
        return ds_str_conv::double_to_string(m_dValue);
    }
    std::string CFieldDataDateTime::GetValueAsString() {
        return std::to_string(m_time);
    }
};