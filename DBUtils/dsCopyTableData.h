#ifndef __DS_COPY_TABLE_DATA_H__
#define __DS_COPY_TABLE_DATA_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
	#include "DBUtilsImpl.h"
#endif

class dsDatabase;
class dsTable;
class dsTableFieldInfo;

class DB_UTILS_API dsCopyTableData
{
// Constuction/Destruction
public:
    dsCopyTableData(dsDatabase *pSrcDB, dsDatabase *pDstDB);
    ~dsCopyTableData();

// Static operations
public:
    // General approach for the Table data copy
    static bool CopyTableData(dsTable &src_table, dsTable &dst_table, const dsTableFieldInfo &union_info) noexcept;

// Operations	
public:
    bool BeginCopy() noexcept;
    bool EndCopy() noexcept;

	bool CopyTableData(const wchar_t *sTableName) noexcept;
	bool CopyTableData(const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst) noexcept;
	bool CopyTableDataEx(const wchar_t *sTableName) noexcept; // Slow simple copy. Dst table flushed.

    dsDatabase *GetSrcDB() noexcept;
    dsDatabase *GetDstDB() noexcept;

// Attributes
private:
    dsDatabase *m_pSrcDB;
	dsDatabase *m_pDstDB;
    bool m_bAttached;
};

#endif