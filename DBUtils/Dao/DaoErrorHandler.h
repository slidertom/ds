#ifndef __DAO_ERROR_HANDLER_H__
#define __DAO_ERROR_HANDLER_H__
#pragma once

class CDaoException;

class CDaoErrorHandler
{
// Construction/Destruction
public:
    CDaoErrorHandler();
    ~CDaoErrorHandler() { }

// Operations
public:
    void OnDaoException(CDaoException *e, const wchar_t *sFunctionName);
    void OnError(const wchar_t *sError, const wchar_t *sFunctionName);

    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

// Operations
private:
    dbErrorHandler m_pErrorHandler;
};

#endif