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
    void OnDaoException(CDaoException *e, LPCTSTR sFunctionName);
    void OnError(LPCTSTR sError, LPCTSTR sFunctionName);

    typedef void (*dbErrorHandler)(LPCTSTR msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

// Operations
private:
    dbErrorHandler m_pErrorHandler;
};

#endif