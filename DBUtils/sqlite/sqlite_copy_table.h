#ifndef __SQLITE_COPY_TABLE_DATA_H__
#define __SQLITE_COPY_TABLE_DATA_H__
#pragma once

#include "unordered_map"

class CSqLiteDatabaseImpl;
class dsDatabase;
class dsTableFieldInfo;

namespace sqlite_util
{
    bool ImportTableData(dsDatabase *pSrcDB, CSqLiteDatabaseImpl *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst, const dsTableFieldInfo &union_info);

    // Attach, detach controls attached database name as SrcDB.
    // Do attach src db for the dst db.
    bool sqlite_attach_database(CSqLiteDatabaseImpl *pSrcDB, CSqLiteDatabaseImpl *pDstDB);
    bool sqlite_detach_database(CSqLiteDatabaseImpl *pDstDB);
    bool sqlite_insert_table_from_attached_db(CSqLiteDatabaseImpl *pDstDB, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst);
};

#endif 
