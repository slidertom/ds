#ifndef __DS_STR_CONV_H__
#define __DS_STR_CONV_H__
#pragma once

#include "string"

namespace ds_str_conv
{
    std::string ConvertToUTF8(const wchar_t *wstr);
    std::wstring ConvertFromUTF8(const char *str);

    long string_to_long(const char *sValue);

    std::string double_to_string(double dValue);
    double string_to_double(const char *sValue);

    void MakeLower(std::wstring &str);
    void MakeUpper(std::wstring &str);
    //void MakeUpper(std::string &str);

    template <typename... T>
    std::wstring FormatStr(LPCTSTR pszFormat, const T & ... args)
    {
        const int sz = std::swprintf(nullptr, 0, pszFormat, args...);
        std::vector<wchar_t> buf(sz + 1); // note +1 for null terminator
        std::swprintf(&buf[0], buf.size(), pszFormat, args...);
        std::wstring sTemp(buf.begin(), buf.end());
        return sTemp;
    }
};

#endif 
