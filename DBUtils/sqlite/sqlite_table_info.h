#ifndef __SQLITE_TABLE_INFO_H__
#define __SQLITE_TABLE_INFO_H__
#pragma once

#ifndef __DS_TYPES_H__
    #include "../dsTypes.h"
#endif

#include "unordered_map"

class CSqLiteDatabaseImpl;
class CSqLiteErrorHandler;

namespace sqlite_util
{
    class CFieldInfo
    {
    // Attributes
    public:
        dsFieldType GetFieldType() const;

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
