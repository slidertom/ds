#ifndef __SQ_LITE_UTIL_H__
#define __SQ_LITE_UTIL_H__
#pragma once

#include "unordered_map"

struct sqlite3_stmt;
class CSqLiteErrorHandler;

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
        virtual std::string GetValueAsString() const = 0;
    };

    class CFieldDataMap : public std::unordered_map<std::string, CFieldData *>  // vector has to be used?
    {
    public:
        ~CFieldDataMap() {
            clear();
        }

    // Operations
    public:
        void clear();
    };

    int sqlite_bind_statements(CFieldDataMap &data_map, sqlite3_stmt *pStmt);
    std::string save_data_to_update_values_string(sqlite_util::CFieldDataMap *pSaveData);
    std::string save_data_to_insert_values_string(sqlite_util::CFieldDataMap *pSaveData);
    std::string save_data_to_insert_columns_string(sqlite_util::CFieldDataMap *pSaveData);
    std::string save_data_to_error_values_string(sqlite_util::CFieldDataMap *pSaveData);
    
    class CFieldDataBinary : public CFieldData 
    {
    // Construction/Destruction
    public:
        CFieldDataBinary(unsigned char *pData, unsigned long nSize);
        virtual ~CFieldDataBinary();

    // Overrides
    public:
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override { return "?"; } // TODO: do update: Base64 should be returned?

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
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override { return m_sText; }

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
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override;

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
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override;

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
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override;

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
        virtual int Bind(sqlite3_stmt *pStmt, int nIndex) override;
        virtual std::string GetValueAsString() const override { return "null"; }
    };
};

#endif 
