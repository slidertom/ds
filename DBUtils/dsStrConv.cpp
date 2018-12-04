#include "stdafx.h"
#include "dsStrConv.h"

#include "vector"
#include "sstream"
#include "iomanip"
#include "limits"
#include "algorithm"
#include "functional"
#include "locale"

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
        const int32_t nLen = (int32_t)wcslen(wstr);
        if ( nLen <= 0  ) {
            return std::string();
        }
        
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        // Win32's UTF8 transcode since Vista uses SSE internally to great effect
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, &strTo[0], size_needed, NULL, NULL);
        return strTo;
        // https://stackoverflow.com/questions/26196686/utf8-utf16-codecvt-poor-performance
        //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_utf8_utf16;
        //const std::string str_utf8 = converter_utf8_utf16.to_bytes(wstr);
        //return str_utf8;
    }

    // Convert an UTF8 string to a wide Unicode String
    std::wstring ConvertFromUTF8(const char *str)
    {
        if ( !str ) {
            return std::wstring();
        }

        const int32_t nLen = (int32_t)strlen(str);
        if ( nLen <= 0 ) {
            return std::wstring();
        }
        
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        // Win32's UTF8 transcode since Vista uses SSE internally to great effect
        MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, &wstrTo[0], size_needed);
        return wstrTo;

        // https://stackoverflow.com/questions/26196686/utf8-utf16-codecvt-poor-performance
        //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter_utf8_utf16;
        //const std::wstring str_utf16 = converter_utf8_utf16.from_bytes(str);
        //return str_utf16;      
    }

    long string_to_long(const char *sValue)
    {
        long nValue;
        std::stringstream strstream;
        strstream << sValue;
        strstream >> nValue;
        return nValue;
    }

    std::string double_to_string(double dValue)
    {   
        typedef std::numeric_limits<double> dbl;
        
        std::string sValue;
        std::stringstream strstream;
        // max_digits10
        // http://www.cplusplus.com/reference/limits/numeric_limits/
        //strstream.precision(dbl::digits10+2); // double: 15 decimal places + point
        strstream.precision(dbl::digits10+1); // double: 15 decimal places + point
        strstream << dValue;
        strstream >> sValue;
        return sValue;
    }

    double string_to_double(const char *sValue)
    {
        double dValue;
        std::stringstream strstream;
        strstream << sValue;
        strstream >> dValue;
        return dValue;
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
 