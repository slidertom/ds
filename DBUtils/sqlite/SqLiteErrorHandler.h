#ifndef __SQ_LITE_ERROR_HANDLER_H__
#define __SQ_LITE_ERROR_HANDLER_H__
#pragma once

class CSqLiteErrorHandler
{
// Construction/Destruction
public:
    CSqLiteErrorHandler() : m_pErrorHandler(nullptr) { } 
    ~CSqLiteErrorHandler() { }

// Operations
public:
    void OnError(LPCTSTR sError, LPCTSTR sFunctionName);
    void OnError(int errorCode, const char *sErrorUTF8, LPCTSTR sFunctionName);
    
    typedef void (*dbErrorHandler)(LPCTSTR msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);
    
// Operations
private:
    dbErrorHandler m_pErrorHandler;
};

#endif 
