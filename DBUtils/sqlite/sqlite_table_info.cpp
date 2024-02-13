#include "stdafx.h"
#include "sqlite_table_info.h"

#include "SqLiteRecordsetImpl.h"
#include "sqlite_bind_util.h"
#include "sqlite_include.h"

#include "algorithm"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace sqlite_util
{
    dsFieldType CFieldInfo::GetFieldType() const
    {
        std::string sType = m_sType;
        std::transform(sType.begin(), sType.end(), sType.begin(), ::toupper); // do use ANSI based -> check out if statements

        if ( sType == "INTEGER" ) {
            return dsFieldType::Integer;
        }

        if ( sType == "TEXT" ) {
            return dsFieldType::Text;
        }

        if (sType == "LONGTEXT") {
            return dsFieldType::Text;
        }

        if ( sType == "REAL" ) {
            return dsFieldType::Double;
        }

        if ( sType == "BLOB" ) {
            return dsFieldType::Blob;
        }

        if ( sType == "DATE" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "DATETIME" ) {
            return dsFieldType::Integer;
        }
        // If the declared type contains the string "INT" then it is assigned INTEGER affinity.
        if ( sType == "INT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "INTEGER" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "TINYINT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "SMALLINT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "MEDIUMINT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "BIGINT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "UNSIGNED BIG INT" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "INT2" ) {
            return dsFieldType::Integer;
        }
        if ( sType == "INT8" ) {
            return dsFieldType::Integer;
        }

        // If the declared type of the column contains any of the strings "CHAR", "CLOB", or "TEXT" 
        // then that column has TEXT affinity. Notice that the type VARCHAR contains the string "CHAR" and is thus assigned TEXT affinity.
        if ( sType == "CHARACTER" ) {
            return dsFieldType::Text;
        }
        if ( sType == "VARCHAR" ) {
            return dsFieldType::Text;
        }
        if ( sType == "VARYING CHARACTER" ) {
            return dsFieldType::Text;
        }
        if ( sType == "NCHAR" ) {
            return dsFieldType::Text;
        }
        if ( sType == "NATIVE CHARACTER" ) {
            return dsFieldType::Text;
        }
        if ( sType == "NVARCHAR" ) {
            return dsFieldType::Text;
        }
        if ( sType == "CHAR" ) {
            return dsFieldType::Text;
        }
        if ( sType == "CLOB" ) {
            return dsFieldType::Text;
        }
        //SQLite does not have a separate Boolean storage class. Instead, Boolean values are stored as integers 0 (false) and 1 (true).
        if ( sType == "BOOLEAN" ) {
            return dsFieldType::Integer;
        }
        // If the declared type for a column contains any of the strings "REAL", "FLOA", or "DOUB" then the column has REAL affinity
        if ( sType == "NUMERIC" ) {
            return dsFieldType::Double;
        }
        if ( sType == "DECIMAL" ) {
            return dsFieldType::Double;
        }
        if ( sType == "FLOAT" ) {
            return dsFieldType::Double;
        }
        if ( sType == "DOUBLE" ) {
            return dsFieldType::Double;
        }
        if ( sType == "DOUBLE PRECISION" ) {
            return dsFieldType::Double;
        }

        ASSERT(FALSE);
        return dsFieldType::Undefined;
    }

    static bool IsAutoincrement(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler)
    {
        // The autoincrement keyword can only be applied to an
        // integer primary key column, and when it is, sqlite creates a entry in
        // its internal sqlite_sequence table with the name set to the name of the table. 
        bool bAutoIncrement = false;
      
        // https://stopbyte.com/t/how-to-check-if-a-column-is-autoincrement-primary-key-or-not-in-sqlite/174
        // First you should understand that at SQLite AUTOINCREMENT can only be applied to PRIMARY KEY’s,
        // so you can follow these steps:
        // 1.Check if the table has any primary keys, and get them from the table_info table, using the query below:    
        //   PRAGMA table_info("<table name here>");
        // 2.You can go through the returned results using a CURSOR, and only select ones that has "pk" column value 
        //   different from 0, which indicates that is a Primary Key.
        // 3.Now you may check if the your data table, has any AUTOINCREMENT Column. can do that by checking 
        //   the sqlite_master table, this way:
        CSqLiteRecordsetImpl auto_increment_loader(pDB, pErrorHandler);
        std::string sSQL;
        sSQL = "SELECT 'is-autoincrement' FROM sqlite_master WHERE tbl_name='";
        sSQL += sTableName;
        sSQL += "'";
        sSQL += " AND sql LIKE '%AUTOINCREMENT%'";
            
        if ( auto_increment_loader.OpenSQLUTF8(sSQL.c_str()) ) {
            if ( auto_increment_loader.MoveFirstImpl() ) {
                bAutoIncrement = true;
            }
        }

        return bAutoIncrement;
    }

    bool sqlite_get_table_fields_info(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler, CFieldInfoMap &map)
    {
        ASSERT(strlen(sTableName) > 0);

        // http://stackoverflow.com/questions/20979239/how-to-tell-if-a-sqlite-column-is-autoincrement
        // Interpretation:
            // If the count came out as non-zero, the table has an autoincrement primary key column.
            // If the count came out as zero, the table is either empty and has never contained data, or does not have an autoincrement primary key column.

        std::string sPragma; 
        sPragma = "PRAGMA table_info (`"; 
        sPragma += sTableName;
        sPragma += "`)";

        CSqLiteRecordsetImpl loader(pDB, pErrorHandler);

        if ( !loader.OpenSQLUTF8(sPragma.c_str()) ) {
            return false;
        }

        if ( !loader.MoveFirstImpl() ) {
            return false;
        }

        while ( !loader.IsEOF() ) 
        {
            CFieldInfo field_info;
            const std::string sColumn = loader.GetFieldStringUTF8("name");
            field_info.m_sType        = loader.GetFieldStringUTF8("type");
            field_info.m_bNotNull     = loader.GetFieldInt32("notnull") == 1;
            field_info.m_sDefault     = loader.GetFieldStringUTF8("dflt_value");
            field_info.m_bPrimary     = loader.GetFieldInt32("pk") == 1;
            if ( field_info.m_bPrimary ) {
                field_info.m_bAutoIncrement = IsAutoincrement(pDB, sTableName, pErrorHandler);
            } else {
                field_info.m_bAutoIncrement = false;
            }
            map[sColumn] = field_info;

            loader.MoveNext();
        }

        return true;
    }

    void sqlite_get_table_index_info(CSqLiteDatabaseImpl *pDB, const char *sTableName, CSqLiteErrorHandler *pErrorHandler, std::vector<std::string> &sUniqueFields,
                                     std::unordered_map<std::string, std::string> &mapIndexSQLs)
    {
        ASSERT(strlen(sTableName) > 0);

        std::string sPragmaIndexList;
        sPragmaIndexList = "PRAGMA INDEX_LIST (`"; 
        sPragmaIndexList += sTableName;
        sPragmaIndexList += "`)";

        CSqLiteRecordsetImpl loaderIndexList(pDB, pErrorHandler);

        if ( !loaderIndexList.OpenSQLUTF8(sPragmaIndexList.c_str()) ) {
            return;
        }

        if ( !loaderIndexList.MoveFirstImpl() ) {
            return;
        }

        std::vector<std::string> sIndexNames;

        while ( !loaderIndexList.IsEOF() ) {
            const bool bUnique = loaderIndexList.GetFieldInt32("unique") == 1;
            if (bUnique) {
                const std::string sIndexName = loaderIndexList.GetFieldStringUTF8("name");
                if (std::string::npos == sIndexName.find("sqlite_autoindex")) {
                    sIndexNames.push_back(sIndexName);
                }
            }
            loaderIndexList.MoveNext();
        }

        for (const std::string &sIndexName : sIndexNames) {
            std::string sFieldName;
            // Unique fields by index name
            {
                CSqLiteRecordsetImpl loaderIndexInfo(pDB, pErrorHandler);
                const std::string sPragmaIndexInfo = "PRAGMA INDEX_INFO (`" + sIndexName + "`)";
                if ( !loaderIndexInfo.OpenSQLUTF8(sPragmaIndexInfo.c_str()) ) {
                    continue;
                }

                if ( !loaderIndexInfo.MoveFirstImpl() ) {
                    continue;
                }

                sFieldName = loaderIndexInfo.GetFieldStringUTF8("name");
                sUniqueFields.push_back(sFieldName);
                
            }
            // Index create statements by index name
            {
                CSqLiteRecordsetImpl loaderMaster(pDB, pErrorHandler);
                const std::string sMaster = "SELECT sql FROM sqlite_master WHERE name=\"" + sIndexName + "\"";
                if ( !loaderMaster.OpenSQLUTF8(sMaster.c_str()) ) {
                    continue;
                }

                if ( !loaderMaster.MoveFirstImpl() ) {
                    continue;
                }

                const std::string sSQL = loaderMaster.GetFieldStringUTF8("sql");
                mapIndexSQLs[sFieldName] = sSQL;
            }
        }
    }
};