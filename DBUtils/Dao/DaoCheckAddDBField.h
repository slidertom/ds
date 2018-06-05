#ifndef __DAO_CHECK_ADD_CB_FIELD_H__
#define __DAO_CHECK_ADD_CB_FIELD_H__
#pragma once 

#ifndef __AFXDAO_H
	#include "afxdao.h"
#endif

template <class TDatabase>
inline bool DaoCheckAddDBField(TDatabase *pDB, 
							   const wchar_t *sTableName, 
							   const wchar_t *sFieldName, 
							   short nType, 
							   long lSize =0L, long lAttributes = dbFixedField)
{
	// Can be used for "on-fly" field adding to database table
	// For params or more advanced field creation see - http://msdn.microsoft.com/en-us/library/x07sew9d%28v=vs.80%29.aspx

	ASSERT(pDB != NULL);
	ASSERT(pDB->IsOpen());
	ASSERT(0 != _tcslen(sTableName));
	ASSERT(0 != _tcslen(sFieldName));

	try
	{
		CDaoTableDef table(pDB->Get());
		table.Open(sTableName);

		try
		{
			CDaoFieldInfo field;
			table.GetFieldInfo(sFieldName, field);

			ASSERT(field.m_nType == nType);
			ASSERT(field.m_lSize == lSize || 0 == lSize);
			//ASSERT(field.m_lAttributes == lAttributes);

			return false; // field already exist
		}
		catch (CDaoException *e)
		{
			// Item not found in this collection (error code #3265)
			ASSERT(3265 == e->m_pErrorInfo->m_lErrorCode);
			e->Delete();
		}
		table.CreateField(sFieldName, nType, lSize, lAttributes);
    
        return true;
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

    return false;
}

#endif