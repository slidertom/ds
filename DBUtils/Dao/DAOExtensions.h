#ifndef __DAO_RECORDSET_EXT_H__
#define __DAO_RECORDSET_EXT_H__
#pragma once

class CDaoDatabase;
class CDaoErrorHandler;

namespace dao_extensions
{
	void CopyTableData(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst, CDaoErrorHandler *pErrorHandler);
    // functions does not throw any exception
    bool CopyTableDataImpl(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst, CDaoErrorHandler *pErrorHandler);
};

#endif
