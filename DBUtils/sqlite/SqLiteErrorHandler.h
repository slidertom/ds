#ifndef __SQ_LITE_ERROR_HANDLER_H__
#define __SQ_LITE_ERROR_HANDLER_H__
#pragma once

#include <string>

struct sqlite3;

class CSqLiteErrorHandler final
{
// Construction/Destruction
public:
    CSqLiteErrorHandler() { } 
    ~CSqLiteErrorHandler() { }

// Operations
public:
    void OnError(const char *sError, const char *sFunctionName);
    void OnErrorCode(int errorCode, const char *sErrorUTF8, const char *sFunctionName);
    void OnErrorCode(int errorCode, sqlite3 *pDB, const char *sFunctionName);
    
    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);
    
    void SetDatabaseLocation(const char *sDbLocation);

// Operations
private:
    dbErrorHandler m_pErrorHandler {nullptr};
    std::string m_sDBLocation;
};

#endif 