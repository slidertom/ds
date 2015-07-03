#ifndef __DAO_RECORDSET_EXT_H__
#define __DAO_RECORDSET_EXT_H__
#pragma once

class CDaoDatabase;

namespace dao_extensions
{
	void CopyTableData(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst);
};

#endif
