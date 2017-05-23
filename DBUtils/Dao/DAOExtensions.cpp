#include "StdAfx.h"
#include "DAOExtensions.h"

#include "AfxDao.h"
#include "DaoErrorHandler.h"

#include "string"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace dao_extensions
{
	namespace internal
	{
		static void CopyTableData(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, const wchar_t *pszTableNameSrc, const wchar_t *pszTableNameDst, const wchar_t *pszColumnName, CDaoErrorHandler *pErrorHandler)
		{
            CString strFormat;
			strFormat.Format(_T("INSERT INTO %s IN '%s' SELECT %s FROM %s;"), pszTableNameDst, pDbDst->GetName(), pszColumnName, pszTableNameSrc);
            try {
			    pDbSrc->Execute(strFormat);
            }
            catch (CDaoException *e) {
                ASSERT(FALSE);
		        pErrorHandler->OnDaoException(e, _T("CDaoDatabaseImpl::CopyTableData"));
                std::wstring sMsg  = L"CopyTableData From: ";
                             sMsg += pszTableNameSrc;
                             sMsg += L" To ";
                             sMsg += pszTableNameDst;
                             sMsg += L".";
                pErrorHandler->OnError(sMsg.c_str(), L"CDaoDatabaseImpl::CopyTableData");
                pErrorHandler->OnError(strFormat, L"CDaoDatabaseImpl::CopyTableData");
		        e->Delete();
            }
		}
	}

    void CopyTableData(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst, CDaoErrorHandler *pErrorHandler)
    {
        ASSERT(sTableNameSrc);
		ASSERT(sTableNameDst);

        ASSERT(pDbSrc);
        ASSERT(pDbSrc->IsOpen());

        ASSERT(pDbDst);
        ASSERT(pDbDst->IsOpen());

        int nControlValue(1);

        CMap<CString, const wchar_t *, int, int> mapColumns;

        // collect all column names from dst
        {
            CDaoTableDef oSrcDaoTableInfo(pDbSrc);
            CDaoFieldInfo fieldInfo;

            oSrcDaoTableInfo.Open(sTableNameSrc);
            if (oSrcDaoTableInfo.IsOpen())
            {
                short nFields = oSrcDaoTableInfo.GetFieldCount();
                for (short i = 0; i < nFields; ++i) {
                    oSrcDaoTableInfo.GetFieldInfo(i, fieldInfo);
                    mapColumns.SetAt(fieldInfo.m_strName, nControlValue);
                }
                oSrcDaoTableInfo.Close();
            }
        }
    
        // collect all column names from src
        {
            int nValue;
            CDaoTableDef oDstDaoTableInfo(pDbDst);
            CDaoFieldInfo fieldInfo;

            oDstDaoTableInfo.Open(sTableNameDst);
            if ( oDstDaoTableInfo.IsOpen() )
            {
                short nFields = oDstDaoTableInfo.GetFieldCount();
                for (short i = 0; i < nFields; ++i)
                {
                    oDstDaoTableInfo.GetFieldInfo(i, fieldInfo);

                    if (mapColumns.Lookup(fieldInfo.m_strName, nValue)) {
                        mapColumns.SetAt(fieldInfo.m_strName, 2);
                    }
                }
                oDstDaoTableInfo.Close();
            }
        }
       
        POSITION pos = mapColumns.GetStartPosition();
        while ( pos ) // remove key which not union
        {
            int nValue;
            CString str;

            mapColumns.GetNextAssoc(pos, str, nValue);
            if (nValue == 1) {
                mapColumns.RemoveKey(str);
            }
        }

        if (mapColumns.GetCount())
        {
            std::wstring strColumns;
            pos = mapColumns.GetStartPosition();
            while ( pos )
            {
                int nValue;
                CString str;

                mapColumns.GetNextAssoc(pos, str, nValue);
                if (!strColumns.empty())
                {
                    strColumns += _T(",");
                }

                strColumns += CString(sTableNameSrc) + _T(".[") + str + _T("]");
            }

            internal::CopyTableData(pDbSrc, pDbDst, sTableNameSrc, sTableNameDst, strColumns.c_str(), pErrorHandler);
        }
    }

    bool CopyTableDataImpl(CDaoDatabase *pDbSrc, CDaoDatabase *pDbDst, const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst, CDaoErrorHandler *pErrorHandler)
    {
        try {
		    dao_extensions::CopyTableData(pDbSrc, pDbDst, sTableNameSrc, sTableNameDst, pErrorHandler);
	    }
        catch (CDaoException *e) {
            ASSERT(FALSE);
		    pErrorHandler->OnDaoException(e, _T("CopyTableDataImpl"));
            std::wstring sMsg  = L"CopyTableData From: ";
                         sMsg += sTableNameSrc;
                         sMsg += L" To ";
                         sMsg += sTableNameDst;
                         sMsg += L".";
            pErrorHandler->OnError(sMsg.c_str(), _T("CopyTableDataImpl"));
		    e->Delete();
		    return false;
        }

	    return true;
    }
};
