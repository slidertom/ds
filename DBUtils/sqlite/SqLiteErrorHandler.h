#ifndef __SQ_LITE_ERROR_HANDLER_H__
#define __SQ_LITE_ERROR_HANDLER_H__
#pragma once

struct sqlite3;

class CSqLiteErrorHandler
{
// Construction/Destruction
public:
    CSqLiteErrorHandler() : m_pErrorHandler(nullptr) { } 
    ~CSqLiteErrorHandler() { }

// Operations
public:
    void OnError(const char *sError, const char *sFunctionName);
    void OnErrorCode(int errorCode, const char *sErrorUTF8, const char *sFunctionName);
    void OnErrorCode(int errorCode, sqlite3 *pDB, const char *sFunctionName);
    
    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);
    
// Operations
private:
    dbErrorHandler m_pErrorHandler;
};

#endif 
