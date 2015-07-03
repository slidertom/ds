#include "StdAfx.h"
#include "DaoErrorHandler.h"

#include "Collections/StdString.h"

#include "afxdao.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace dao_exception_format
{
    CStdString FormatException(CDaoException *e, LPCTSTR sFunctionName)
    {
        TCHAR szCause[255];
        e->GetErrorMessage(szCause, 255);

        CStdString sErrorCode;
        if (e->m_pErrorInfo) {
            sErrorCode.Format(_T("Error code: %i. Description: "), e->m_pErrorInfo->m_lErrorCode);
        }

        CStdString sFormatted;
        sFormatted = _T("DAO Exception [");
        sFormatted += sFunctionName;
        sFormatted += _T("] - ");
        sFormatted += sErrorCode.c_str();
        sFormatted += szCause;

        return sFormatted;
    }

    void LogError(LPCTSTR sError, LPCTSTR sFunctionName)
    {
        
    }
};

CDaoErrorHandler::CDaoErrorHandler()
: m_pErrorHandler(nullptr)
{

}

void CDaoErrorHandler::OnDaoException(CDaoException *e, LPCTSTR sFunctionName)
{
    if ( !m_pErrorHandler ) {
        return;
    }
    const CStdString str = dao_exception_format::FormatException(e, sFunctionName);
    (*m_pErrorHandler)(str.c_str());
}

void CDaoErrorHandler::OnError(LPCTSTR sError, LPCTSTR sFunctionName)
{
    if ( !m_pErrorHandler ) {
        return;
    }
    CStdString sMsg;
    sMsg.Format(_T("%s-%s"), sError, sFunctionName);
    (*m_pErrorHandler)(sMsg.c_str());
}

CDaoErrorHandler::dbErrorHandler CDaoErrorHandler::SetErrorHandler(CDaoErrorHandler::dbErrorHandler newHandler)
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    return prevHandler;
}