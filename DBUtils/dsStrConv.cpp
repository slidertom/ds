#include "stdafx.h"
#include "dsStrConv.h"

#include <charconv>

#include <vector>
#include <algorithm>
#include <functional>
#include <locale>
#include <codecvt>

//#include "codecvt"
#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Convert a wide Unicode string to an UTF8 string
namespace ds_str_conv
{
    // or maybe should be used:
    // http://stackoverflow.com/questions/4358870/convert-wstring-to-string-encoded-in-utf-8
    // std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    // return myconv.to_bytes(str);
    std::string ConvertToUTF8(const wchar_t *wstr)
    {
        const int32_t nLen = (int32_t)::wcslen(wstr);
        if ( nLen <= 0  ) {
            return std::string();
        }
        
#if defined(_WIN64) || defined(_WIN32) 
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        // Win32's UTF8 transcode since Vista uses SSE internally to great effect
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, &strTo[0], size_needed, NULL, NULL);
        return strTo;
#else 
        // https://stackoverflow.com/questions/26196686/utf8-utf16-codecvt-poor-performance
        std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        return myconv.to_bytes(wstr);
#endif
    }

    // Convert an UTF8 string to a wide Unicode String
    std::wstring ConvertFromUTF8(const char *str)
    {
        // Possible to get nullptr from CSqLiteRecordsetImpl::GetFieldString() -> (const char*)sqlite3_column_text(m_stmt, nColumnIndex);
        if ( !str ) {
            return std::wstring();
        }

        const int32_t nLen = (int32_t)::strlen(str);
        if ( nLen <= 0 ) {
            return std::wstring();
        }
        
#if defined(_WIN64) || defined(_WIN32) 
        int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        // Win32's UTF8 transcode since Vista uses SSE internally to great effect
        ::MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, &wstrTo[0], size_needed);
        return wstrTo;
#else 
        // https://stackoverflow.com/questions/26196686/utf8-utf16-codecvt-poor-performance
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_utf8_utf16;
        const std::wstring str_utf16 = converter_utf8_utf16.from_bytes(str);
        return str_utf16;
#endif
    }
   
    // String without leading spaces is expected.
    // Otherwise - an error is produced.
    // std::from_chars returns std::errc::invalid_argument if "    123.12345" is passed as sValue.
    template <typename T>
    class StringToNumeric
    {
    public:
        static T Convert(const char *sValue)
        {
            T val = 0;
	        [[maybe_unused]] auto [p, ec] = std::from_chars(sValue, sValue + ::strlen(sValue), val);
	        // yes, we can get std::errc() if we pass here non numeric string
	        ASSERT(ec == std::errc());
	        return val;
        }
    };

    time_t string_to_time(const char *sValue) 
    {   // if required specific implementation can be applied
        return StringToNumeric<time_t>::Convert(sValue);
    }

    int32_t string_to_int32(const char *sValue)
    {   // if required specific implementation can be applied
        return StringToNumeric<int32_t>::Convert(sValue);
    }

    int64_t string_to_int64(const char *sValue)
    {   // if required specific implementation can be applied
        return StringToNumeric<int64_t>::Convert(sValue);
    }

    double string_to_double(const char *sValue)
    {   // if required specific implementation can be applied
        return StringToNumeric<double>::Convert(sValue);
    }

    std::string	double_to_string(const double dVal)
    {
	    char sBuf[30]; // 30 is large enough for double values because the longest possible outputs are "-1.23456735e-36".
	    auto [p, ec] = std::to_chars(sBuf, sBuf + std::size(sBuf), dVal);
	    ASSERT(ec == std::errc());
	    if (ec == std::errc{}) // no error
		    return std::string(&sBuf[0], p - sBuf);

	    ASSERT(false);
	    return "";
    }

    void MakeLower(std::string &str)
    {
        const std::locale &loc = std::locale();
        std::transform(str.begin(), str.end(), str.begin(),
                [&loc](int c) { return std::tolower(c, loc); });
    }

    void MakeLower(std::wstring &str)
    {
        const std::locale &loc = std::locale();
        std::transform(str.begin(), str.end(), str.begin(),
                [&loc](int c) { return std::tolower(c, loc); });
    }

      void MakeUpper(std::wstring &str)
    {
        const std::locale &loc = std::locale();
        std::transform(str.begin(), str.end(), str.begin(),
                [&loc](int c) { return std::toupper(c, loc); });
    }

    /*
    void MakeUpper(std::string &str)
    {
        const std::locale &loc = std::locale();
        std::transform(str.begin(), str.end(), str.begin(),
                       std::bind1st(std::mem_fun(&std::ctype<char>::toupper), &std::use_facet<std::ctype<char>>(loc)));
    }
    */
    /*
    std::wstring FormatStr(const wchar_t *pszFormat, ...)
    {
        va_list arglist;
        va_start(arglist, pszFormat);
            const int sz = std::swprintf(nullptr, 0, pszFormat, arglist);
            std::vector<wchar_t> buf(sz + 1); // note +1 for null terminator
            std::swprintf(&buf[0], buf.size(), pszFormat, arglist);
            std::wstring sTemp(buf.begin(), buf.end());
        va_end(arglist);

        return sTemp;
    }
    */
};