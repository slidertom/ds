#ifndef __OLE_DATE_TIME_UTIL_H__
#define __OLE_DATE_TIME_UTIL_H__
#pragma once

template <class TOleDateTime>
inline time_t OleDateTimeToTime(const TOleDateTime &oleDt)
{
    struct tm tmDate;
	memset(&tmDate, 0, sizeof(tm));
    tmDate.tm_sec  = oleDt.GetSecond();
    tmDate.tm_min  = oleDt.GetMinute();
    tmDate.tm_hour = oleDt.GetHour();
    tmDate.tm_mday = oleDt.GetDay();
    tmDate.tm_mon  = oleDt.GetMonth() - 1;
    tmDate.tm_year = oleDt.GetYear() - 1900;
    tmDate.tm_isdst = -1;
    return mktime(&tmDate);
}

template <class TOleVariant>
inline time_t VariantToTime(const TOleVariant &oleTime) {
    COleDateTime oleDt(oleTime);
	return ::OleDateTimeToTime(oleDt);
}

#endif
