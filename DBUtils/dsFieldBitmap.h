#ifndef __DS_FIELD_BITMAP_H__
#define __DS_FIELD_BITMAP_H__
#pragma once

#ifndef __DS_TABLE_H__
    #include "dsTable.h"
#endif

class CBitmap;

namespace ds_table_ext
{
    DB_UTILS_API CBitmap *GetFieldBitmap(const dsTable &table, const wchar_t *sFieldName);
    DB_UTILS_API bool SetFieldBitmap(dsTable &table, const wchar_t *sFieldName, CBitmap *pBmp, bool bUseColors);
};

#define FIELD_BITMAP(name, realname) \
	CBitmap* Get##name() const                                 { return ds_table_ext::GetFieldBitmap(*this, realname); }  \
	bool Set##name(CBitmap* pBmpValue, bool bUseColors = true) { return ds_table_ext::SetFieldBitmap(*this, realname, pBmpValue, bUseColors); } \
	FIELD_INDICATORS(name, realname)

#endif
