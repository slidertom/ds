#ifndef __DS_STR_CONV_H__
#define __DS_STR_CONV_H__
#pragma once

#ifndef STDSTRING_H
    #include "Collections/StdString.h"
#endif

namespace ds_str_conv
{
    std::string ConvertToUTF8(const wchar_t *wstr);
    CStdString ConvertFromUTF8(const char *str);

    std::string int_to_string(int nValue);
    std::string long_to_string(long nValue);
    long string_to_long(const char *sValue);

    std::string double_to_string(double dValue);
    double string_to_double(const char *sValue);
};

#endif 
