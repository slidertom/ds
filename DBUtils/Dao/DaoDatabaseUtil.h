#ifndef __DAO_DATABASE_UTIL_H__
#define __DAO_DATABASE_UTIL_H__
#pragma once

#ifndef STDSTRING_H
    #include "Collections/StdString.h"
#endif

namespace dao_database_util
{
    template <class TOleVariant>
    inline double GetDouble(const TOleVariant &varSrc)
    {
	    LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	    switch (pSrc->vt)
	    {
		    case VT_R4:
			    return pSrc->fltVal;

		    case VT_R8:
			    return pSrc->dblVal;

		    case VT_BOOL:
			    return V_BOOL(pSrc);

		    case VT_UI1:
			    return pSrc->bVal;

		    case VT_I2:
			    return pSrc->iVal;

		    case VT_I4:
			    return pSrc->lVal;

		    case VT_CY:
			    return (double)COleCurrency(varSrc).m_cur.int64 / 10000;
	    }

	    return 0;
    }

    template <class TOleVariant>
    inline int GetInt(const TOleVariant &varSrc)
    {
	    LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	    switch (pSrc->vt)
	    {
		    case VT_R4:
			    return (int)pSrc->fltVal;

		    case VT_R8:
			    return (int)pSrc->dblVal;

		    case VT_BOOL:
			    return V_BOOL(pSrc);

		    case VT_UI1:
			    return pSrc->bVal;

		    case VT_I2:
			    return pSrc->iVal;

		    case VT_I4:
			    return pSrc->lVal;

		    case VT_BSTR:
			    return _ttoi((LPCTSTR)pSrc->bstrVal);
	    }

	    return 0;
    }

    template <class TOleVariant>
    inline bool IsDouble(TOleVariant &varSrc)
    {
	    LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	    switch (pSrc->vt)
	    {
	    case VT_R4:
	    case VT_R8:
		    return true;

	    }

	    return false;
    }

    template <class TOleVariant>
    inline CStdString GetString(const TOleVariant &varSrc)
    {
	    LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	    switch (pSrc->vt)
	    {
	    case VT_BSTR:
		    return CStdString((LPCTSTR)pSrc->bstrVal);

	    case VT_DATE:
		    return (LPCTSTR)COleDateTime(varSrc).Format();

	    case VT_UI1:
            {
                CStdString str;
		        str.Format(_T("%i"),int(pSrc->bVal));
		        return str;
            }
	    case VT_I2:
            {
                CStdString str;
		        str.Format(_T("%i"),int(pSrc->iVal));
		        return str;
            }
	    case VT_I4:
            {
                CStdString str;
		        str.Format(_T("%i"),int(pSrc->lVal));
		        return str;
            }
	    case VT_R4:
            {
                CStdString str;
		        str.Format(_T("%.2f"), double(pSrc->fltVal));
		        return str;
            }
	    case VT_R8:
            {
                CStdString str;
		        str.Format(_T("%.2f"), double(pSrc->dblVal));
		        return str;
            }
	    case VT_CY:
            {
                CStdString str;
		        str.Format(_T("%.2f"), _tcstod(COleCurrency(varSrc).Format(), NULL));
		        return str;
            }
	    case VT_BOOL:
		    return V_BOOL(pSrc) != 0 ? _T("true") : _T("false");
	    }

	    return _T("");
    }
};

#endif