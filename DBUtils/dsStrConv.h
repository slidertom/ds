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
};

#endif 
