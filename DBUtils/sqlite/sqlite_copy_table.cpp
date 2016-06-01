#include "StdAfx.h"
#include "sqlite_copy_table.h"

#include "sqlite_include.h"
#include "sqlite_table_info.h"
#include "sqlite_bind_util.h"

#include "SqLiteRecordsetImpl.h"

#include "../dsDatabase.h"
#include "../AbsDatabase.h"
#include "../dsStrConv.h"
#include "../dsTable.h"

#include "SqLiteDatabaseImpl.h" 
#include "SqLiteErrorHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace sqlite_util
{   
    bool ImportTableData(dsDatabase *pSrcDB, CSqLiteDatabaseImpl *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst, const dsTableFieldInfo &union_info)
    {
        if ( union_info.size() == 0 ) {
            CStdStringA sTableNameDstUTF8 = ds_str_conv::ConvertToUTF8(sTableNameDst);
            CStdStringA sError;
            sError.Format("There are no fields to import in the table: %s.", sTableNameDstUTF8.c_str());
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
                    case dsFieldType_Text:
                        {
                            const CStdString sValue = src_table.GetFieldString(sFieldName);
                            dst_table.SetFieldString(sFieldName, sValue.c_str());
                        }
                        break;
                    case dsFieldType_Long:
                        {
                            const int nValue = src_table.GetFieldLong(sFieldName);
                            dst_table.SetFieldLong(sFieldName, nValue);
                        }
                        break;
                    case dsFieldType_Double:
                        {
                            const double dValue = src_table.GetFieldDouble(sFieldName);
                            dst_table.SetFieldDouble(sFieldName, dValue);
                        }
                        break;
                    case dsFieldType_DateTime:
                        {
                            const time_t nTime = src_table.GetFieldDateTime(sFieldName);
                            dst_table.SetFieldDateTime(sFieldName, nTime);
                        }
                        break;
                    case dsFieldType_Binary:
                        {
                            unsigned char *pData = nullptr;
                            unsigned long nSize = 0;
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

    bool sqlite_attach_database(dsDatabase *pSrcDB, dsDatabase *pDstDB)
    {
        ASSERT(pSrcDB->GetType() == dsType_SqLite);
        ASSERT(pDstDB->GetType() == dsType_SqLite);

        const CStdString sPath = pSrcDB->GetName();
        // dsCopyTableData should be constructured before BeginTrans
        CStdString sSQL;
        sSQL.Format(_T("ATTACH DATABASE \"%s\" as SrcDB;"), sPath.c_str());
        if ( !pDstDB->Execute(sSQL.c_str()) ) {
            return false;
        }
        return true;
    }

    bool sqlite_detach_database(dsDatabase *pDstDB) 
    {  
        ASSERT(pDstDB->GetType() == dsType_SqLite);

        LPCTSTR sSQL = _T("DETACH DATABASE SrcDB;");
        if ( !pDstDB->Execute(sSQL) ) {
            return false;
        }
        return true;
    }

    bool sqlite_insert_table_from_attached_db(dsDatabase *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst)
    {
        ASSERT(pDstDB->GetType() == dsType_SqLite);

        CStdString sSQL;
        sSQL.Format(_T("INSERT INTO %s SELECT * FROM SrcDB.%s;"), sTableNameDst, sTableNameSrc);
        if ( !pDstDB->Execute(sSQL.c_str()) ) {
            return false;
        }

        return true;
    }
};