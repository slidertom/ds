#include "stdafx.h"
#include "SqLiteErrorHandler.h"

#include "sqlite_bind_util.h"
#include "sqlite_include.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

void CSqLiteErrorHandler::OnError(const char *sError, const char *sFunctionName)
{
    ASSERT(FALSE);
    if ( !m_pErrorHandler ) {
        return;
    }

    std::string sMsg  = sError;
                sMsg += "-";
                sMsg += sFunctionName;
    std::wstring sErrorMsg = ds_str_conv::ConvertFromUTF8(sMsg.c_str());  
    (*m_pErrorHandler)(sErrorMsg.c_str());
}

void CSqLiteErrorHandler::OnErrorCode(int errorCode, sqlite3 *pDB, const char *sFunctionName)
{
    const char *localError = sqlite3_errmsg(pDB);
    OnErrorCode(errorCode, localError, sFunctionName);
}

void CSqLiteErrorHandler::SetDatabaseLocation(const char *sDbLocation)
{
    m_sDBLocation = sDbLocation;
}

void CSqLiteErrorHandler::OnErrorCode(int errorCode, const char *sErrorUTF8, const char *sFunctionName)
{
    ASSERT(FALSE);
    const char *sErrMsg = sqlite3_errstr(errorCode);
    std::string sCode = std::to_string(errorCode);
    std::string sError = sErrMsg;
                sError += "[";
                sError += sCode.c_str();
                sError += "]: ";
                sError += sErrorUTF8;

                if ( !m_sDBLocation.empty() ) 
                {   // memory database m_sDBLocatio can be empty
                    sError += " ";
                    sError += "Database Location: ";
                    sError += m_sDBLocation;
                    sError += " ";
                }

    OnError(sError.c_str(), sFunctionName);
}

CSqLiteErrorHandler::dbErrorHandler CSqLiteErrorHandler::SetErrorHandler(dbErrorHandler newHandler)
{
    dbErrorHandler prevHandler = m_pErrorHandler;
    m_pErrorHandler = newHandler;
    return prevHandler;
}