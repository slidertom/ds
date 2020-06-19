#ifndef __LOG_IMPL_H__
#define __LOG_IMPL_H__
#pragma once

// internal usage only
class CLogImpl
{
// Static operations
public:
    static void Log(const wchar_t *sMsg);
    static void SetLogPath(const wchar_t *sLogPath);    
};

#endif