#ifndef __DS_CONFIG_H__
#define __DS_CONFIG_H__
#pragma once

// if log should be protected with mutexes =>
// do define  DS_LOG_MUTEX.
//#define DS_LOG_MUTEX
#ifdef DS_LOG_MUTEX
    // please define mutex class.
    // sample:
    // typedef CMutexImpl wxCriticalSection;
    // typedef CMutexImplLocker wxMutexLocker;
#endif

namespace ds_log_file_path
{
    // LOG file default path
    inline CStdString GetDefaultPath()
    {
       return _T(""); 
    }
};

#endif
