#include "stdafx.h"
#include "sqlite_copy_table.h"

#include "sqlite_include.h"
#include "sqlite_table_info.h"
#include "sqlite_bind_util.h"

#include "SqLiteRecordsetImpl.h"

#include "../dsStrConv.h"
#include "../dsTable.h"

#include "sqlite_database_impl.h"
#include "SqLiteErrorHandler.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace sqlite_util
{   
    bool ImportTableData(dsDatabase *pSrcDB, CSqLiteDatabaseImpl *pDstDB, 
                         const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst, const dsTableFieldInfo &union_info)
    {
        if ( union_info.size() == 0 ) {
            const std::string sTableNameDstUTF8 = ds_str_conv::ConvertToUTF8(sTableNameDst);
            std::string sError = "There are no fields to import in the table: ";
                       sError += sTableNameDstUTF8;
                       sError += ".";
            pDstDB->GetErrorHandler()->OnError(sError.c_str(), "sqlite_util::ImportTableData");
            return false;
        }

        dsTable src_table(pSrcDB, sTableNameSrc);
        if ( !src_table.MoveFirst() ) {
            return true; // empty table
        }
   
        sqlite_util::CFieldDataMap save_data;
        CSqLiteRecordsetImpl dst_table(pDstDB, pDstDB->GetErrorHandler());
        dst_table.Open(sTableNameDst);
        while ( !src_table.IsEOF() )
        {
            // TODO: statement can be cached 
            // only do bind different values
            dst_table.PrepareInsert();
                auto end_it = union_info.end();
                for (auto it = union_info.begin(); it != end_it; ++it) 
                {
                    const TCHAR *sFieldName = it->first.c_str();

                    if ( src_table.IsFieldValueNull(sFieldName) )
                    {
                        dst_table.SetFieldValueNull(sFieldName);
                        continue;
                    }

                    switch (it->second) 
                    {
                    case dsFieldType::Text:
                        {
                            const std::string sFieldNameUTF8 = ds_str_conv::ConvertToUTF8(sFieldName);
                            const std::string sValue = src_table.GetFieldStringUTF8(sFieldNameUTF8.c_str());
                            dst_table.SetFieldStringUTF8(sFieldNameUTF8.c_str(), sValue.c_str());
                        }
                        break;
                    case dsFieldType::Integer:
                        {
                            const int nValue = src_table.GetFieldLong(sFieldName);
                            dst_table.SetFieldInt32(sFieldName, nValue);
                        }
                        break;
                    case dsFieldType::Double:
                        {
                            const double dValue = src_table.GetFieldDouble(sFieldName);
                            dst_table.SetFieldDouble(sFieldName, dValue);
                        }
                        break;
                    case dsFieldType::DateTime:
                        {
                            const time_t nTime = src_table.GetFieldDateTime(sFieldName);
                            dst_table.SetFieldDateTime(sFieldName, nTime);
                        }
                        break;
                    case dsFieldType::Blob:
                        {
                            unsigned char *pData = nullptr;
                            size_t nSize = 0;
                            src_table.GetFieldBinary(sFieldName, &pData, nSize);        
                            dst_table.SetFieldBinary(sFieldName, pData, nSize);
                        }
                        break;
                    }
                }
            dst_table.CommitInsert();

            src_table.MoveNext();
        }

        return true;
    }

    bool sqlite_attach_database(CSqLiteDatabaseImpl *pSrcDB, CSqLiteDatabaseImpl *pDstDB)
    {
        const std::wstring sPath = pSrcDB->GetName();
        std::string sPathUTF8 = ds_str_conv::ConvertToUTF8(sPath.c_str());

        // dsCopyTableData should be constructured before BeginTrans
        std::string sSQL;
        sSQL += "ATTACH DATABASE \"";
        sSQL += sPathUTF8;
        sSQL += "\"";
        sSQL += " as SrcDB;";

        if ( !pDstDB->Execute(sSQL.c_str()) ) {
            return false;
        }
        return true;
    }

    bool sqlite_detach_database(CSqLiteDatabaseImpl *pDstDB) 
    {  
        ASSERT(pDstDB->GetType() == dsDBType::SqLite);
        const char *sSQL = "DETACH DATABASE SrcDB;";
        if ( !pDstDB->Execute(sSQL) ) {
            return false;
        }
        return true;
    }

    bool sqlite_insert_table_from_attached_db(CSqLiteDatabaseImpl *pDstDB, const char *sTableNameSrc, const char *sTableNameDst)
    {
        ASSERT(pDstDB->GetType() == dsDBType::SqLite);

        // INSERT INTO sTableNameDst SELECT * FROM SrcDB.sTableNameSrc;"
        std::string sSQL  = "INSERT INTO ";
                    sSQL += sTableNameDst;
                    sSQL += " SELECT * FROM SrcDB.";
                    sSQL += sTableNameSrc;
                    sSQL += ";";

        if ( !pDstDB->Execute(sSQL.c_str()) ) {
            return false;
        }

        return true;
    }
};