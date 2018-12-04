#include "stdafx.h"
#include "DaoErrorHandler.h"

#include "afxdao.h"
#include "string"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace dao_exception_format
{
    std::wstring FormatException(CDaoException *e, const wchar_t *sFunctionName)
    {
        TCHAR szCause[255];
        e->GetErrorMessage(szCause, 255);

        std::wstring sErrorCode;
        if (e->m_pErrorInfo) {
            sErrorCode += L"Error code: ";
            sErrorCode += std::to_wstring(e->m_pErrorInfo->m_lErrorCode);
            sErrorCode += L". Description: ";
        }

        std::wstring sFormatted;
        sFormatted = L"DAO Exception [";
        sFormatted += sFunctionName;
        sFormatted += L"] - ";
        sFormatted += sErrorCode.c_str();
        sFormatted += szCause;

        return sFormatted;
    }
};

CDaoErrorHandler::CDaoErrorHandler()
: m_pErrorHandler(nullptr)
{

}

void CDaoErrorHandler::OnDaoException(CDaoException *e, const wchar_t *sFunctionName)
{
    if ( !m_pErrorHandler ) {
        return;
    }
    const std::wstring str = dao_exception_format::FormatException(e, sFunctionName);
    (*m_pErrorHandler)(str.c_str());
}

void CDaoErrorHandler::OnError(const wchar_t *sError, const wchar_t *sFunctionName)
{
    if ( !m_pErrorHandler ) {
        return;
    }
    std::wstring sMsg  = sError;
                 sMsg += L"-";
                 sMsg += sFunctionName;
    (*m_pErrorHandler)(sMsg.c_str());
}

CDaoErrorHandler::dbErrorHandler CDaoErrorHandler::SetErrorHandler(CDaoErrorHandler::dbErrorHandler newHandler)
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    return prevHandler;
}