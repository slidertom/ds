#ifndef __SQLITE_TABLE_INFO_H__
#define __SQLITE_TABLE_INFO_H__
#pragma once

#include "unordered_map"

class CSqLiteDatabaseImpl;
class CSqLiteErrorHandler;

namespace sqlite_util
{
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
    bool sqlite_get_table_fields_info(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler, CFieldInfoMap &map);
};

#endif 
