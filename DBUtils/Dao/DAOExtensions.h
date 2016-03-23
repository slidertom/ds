#ifndef __DAO_RECORDSET_EXT_H__
#define __DAO_RECORDSET_EXT_H__
#pragma once

class CDaoDatabase;
class CDaoErrorHandler;

namespace dao_extensions
{
	void CopyTableData(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst, CDaoErrorHandler *pErrorHandler);
    // functions does not throw any exception
    bool CopyTableDataImpl(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst, CDaoErrorHandler *pErrorHandler);
};

#endif
