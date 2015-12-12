#include "StdAfx.h"
#include "SqLiteErrorHandler.h"

#include "SqLiteUtil.h"

#include "Collections/StdString.h"

#include "sqlite_include.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CSqLiteErrorHandler::OnError(LPCTSTR sError, LPCTSTR sFunctionName)
{
    if ( !m_pErrorHandler ) {
        return;
    }

    CStdString sMsg  = sError;
               sMsg += _T("-");
               sMsg += sFunctionName;
    (*m_pErrorHandler)(sMsg.c_str());
}

void CSqLiteErrorHandler::OnError(int errorCode, const char *sErrorUTF8, LPCTSTR sFunctionName)
{
    const char *sErrMsg = sqlite3_errstr(errorCode);

    CStdStringW sCode;
    sCode.Format(_T("%d"), errorCode);

    CStdStringW sErrorW  = ds_str_conv::ConvertFromUTF8(sErrorUTF8);               
    CStdStringW sErrMsgW = ds_str_conv::ConvertFromUTF8(sErrMsg);               

    CStdStringW sError = sErrMsgW.c_str();
                sError += _T("[");
                sError += sCode.c_str();
                sError += _T("]: ");
                sError += sErrorW;
    OnError(sError .c_str(), sFunctionName);
}
   
CSqLiteErrorHandler::dbErrorHandler CSqLiteErrorHandler::SetErrorHandler(dbErrorHandler newHandler)
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    return prevHandler;
}

