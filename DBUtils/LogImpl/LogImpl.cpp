#include "stdafx.h"
#include "LogImpl.h"

#include "../dsStrConv.h"

#include "mutex"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace internal
{
    // http://www.cplusplus.com/reference/ctime/asctime/ 
    // do not return the \n at the end (diff. from asctime)
    static inline char* asctime_impl(const struct tm *timeptr)
    {
        static const char wday_name[][4] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
        };
        static const char mon_name[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        static char result[26];
        sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
        wday_name[timeptr->tm_wday],
        mon_name[timeptr->tm_mon],
        timeptr->tm_mday, timeptr->tm_hour,
        timeptr->tm_min, timeptr->tm_sec,
        1900 + timeptr->tm_year);
        return result;
    }
};

static std::mutex g_log_mutex; // just in case do allow only one thread to log data
                               // dao single threaded

static std::wstring g_sLogPath = L"";

void CLogImpl::Log(const wchar_t *sMsg)
{
    std::lock_guard<std::mutex> lock(g_log_mutex);

    const std::wstring sFilePath = g_sLogPath;
    ASSERT(!sFilePath.empty());

    time_t rawtime;
    struct tm *timeinfo;
            
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    const char *sTime = internal::asctime_impl(timeinfo);

    std::string sMsgImpl;
    sMsgImpl += "[";
    sMsgImpl += sTime;
    sMsgImpl += "] ";
    sMsgImpl += ds_str_conv::ConvertToUTF8(sMsg).c_str();
    sMsgImpl += "\n";

    FILE *pFile = _tfopen(sFilePath.c_str(), _T("a"));    
    if ( !pFile ) {
        ASSERT(FALSE);
        return;
    }
    fprintf(pFile, "%s", sMsgImpl.c_str());
    fclose(pFile);
}

void CLogImpl::SetLogPath(const wchar_t *sLogPath)
{
    g_sLogPath = sLogPath;
}