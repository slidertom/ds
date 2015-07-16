#ifndef __SQ_LITE_UTIL_H__
#define __SQ_LITE_UTIL_H__
#pragma once

#ifndef STDSTRING_H
    #include "Collections/StdString.h"
#endif

namespace sqlite_conv
{
    std::string ConvertToUTF8(const wchar_t *wstr);
    CStdString ConvertFromUTF8(const char *str);
    std::string to_string(long nValue);
};


#include "unordered_map"

struct sqlite3_stmt;
class CSqLiteDatabaseImpl;
class CSqLiteErrorHandler;

class dsDatabase;
class dsTableFieldInfo;

namespace sqlite_util
{
    class CFieldData {
    // Construction/Destruction
    public:
        CFieldData() { }
        virtual ~CFieldData() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) = 0;
    };

    class CFieldDataMap : public std::unordered_map<std::string, CFieldData *> 
    {
    public:
        ~CFieldDataMap() {
            clear();
        }

    // Operations
    public:
        void clear();
    };
    void BindStatements(CFieldDataMap &data_map, sqlite3_stmt *pStmt);

    class CFieldDataBinary : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataBinary(unsigned char *pData, unsigned long nSize);
        virtual ~CFieldDataBinary();

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);

    // Attributes
    private:
        unsigned char *m_pData;
        unsigned long m_nSize;
    };

    class CFieldDataText : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataText(const char *sText) : m_sText(sText) { }
        virtual ~CFieldDataText() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);

    // Attributes
    private:
        std::string m_sText;
    };

    class CFieldDataLong : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataLong(int nValue) : m_nValue(nValue) { }
        virtual ~CFieldDataLong() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);

    // Attributes
    private:
        int m_nValue;
    };

    class CFieldDataDouble : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataDouble(double dValue) : m_dValue(dValue) { }
        virtual ~CFieldDataDouble() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);

    // Attributes
    private:
        double m_dValue;
    };

    class CFieldDataDateTime : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataDateTime(const time_t &time) : m_time(time){ }
        virtual ~CFieldDataDateTime() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);

    // Attributes
    private:
        time_t m_time;
    };

    class CFieldDataNull : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataNull() { }
        virtual ~CFieldDataNull() { }

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex);
    };

    enum eFieldType {
        eFieldType_Undefined = -1,
        eFieldType_Text = 0,
        eFieldType_Long,
        eFieldType_Double,
        eFieldType_Binary
    };
    
    class CFieldInfo
    {
    // Attributes
    public:
        eFieldType GetFieldType() const;

        bool m_bPrimary;
        bool m_bAutoIncrement;
        bool m_bNotNull;
        std::string m_sDefault;
        std::string m_sType;
    };

    class CFieldInfoMap : public std::unordered_map<std::string, CFieldInfo> { };
    bool GetTableFieldsdInfo(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler, CFieldInfoMap &map);

    bool ImportTableData(dsDatabase *pSrcDB, CSqLiteDatabaseImpl *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst, dsTableFieldInfo union_info);
};

#endif 
