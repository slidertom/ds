#ifndef __DS_STR_CONV_H__
#define __DS_STR_CONV_H__
#pragma once

#include <string>
#include <vector>

namespace ds_str_conv
{
    std::string ConvertToUTF8(const wchar_t *wstr);
    std::wstring ConvertFromUTF8(const char *str);

    // String without leading spaces is expected.
    // Otherwise - an error is produced.
    // std::from_chars returns std::errc::invalid_argument if "    123.12345" is passed as sValue.
    int64_t string_to_int64(const char *sValue);
    int32_t string_to_int32(const char *sValue);
    time_t  string_to_time(const char *sValue);

    // String without leading spaces is expected.
    // Otherwise - an error is produced.
    double string_to_double(const char *sValue);
    std::string	double_to_string(const double dVal);

    void MakeLower(std::string &str);
    void MakeLower(std::wstring &str);
    void MakeUpper(std::wstring &str);
    //void MakeUpper(std::string &str);

    template <typename... T>
    std::wstring FormatStr(const wchar_t *pszFormat, const T & ... args)
    {
        const int sz = std::swprintf(nullptr, 0, pszFormat, args...);
        std::vector<wchar_t> buf(sz + 1); // note +1 for null terminator
        std::swprintf(&buf[0], buf.size(), pszFormat, args...);
        std::wstring sTemp(buf.begin(), buf.end());
        return sTemp;
    }
};

#endif 