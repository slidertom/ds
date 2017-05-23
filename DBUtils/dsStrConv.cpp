#include "StdAfx.h"
#include "dsStrConv.h"

#include "vector"
#include "sstream"
#include "iomanip"
#include "limits"
#include "algorithm"
#include "functional"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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
        const int nLen = wcslen(wstr);
        if ( nLen <= 0  ) {
            return std::string();
        }

        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, NULL, 0, NULL, NULL);

        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], nLen, &strTo[0], size_needed, NULL, NULL);

        return strTo;
    }

    // Convert an UTF8 string to a wide Unicode String
    std::wstring ConvertFromUTF8(const char *str)
    {
        if ( !str ) {
            return std::wstring();
        }

        const int nLen = strlen(str);
        if ( nLen <= 0 ) {
            return std::wstring();
        }

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, NULL, 0);

        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, &wstrTo[0], size_needed);

        return wstrTo;
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
                       std::bind1st(std::mem_fun(&std::ctype<wchar_t>::tolower), &std::use_facet<std::ctype<wchar_t>>(loc)));
    }

    void MakeUpper(std::wstring &str)
    {
        const std::locale &loc = std::locale();
        std::transform(str.begin(), str.end(), str.begin(),
                       std::bind1st(std::mem_fun(&std::ctype<wchar_t>::toupper), &std::use_facet<std::ctype<wchar_t>>(loc)));
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
    std::wstring FormatStr(LPCTSTR pszFormat, ...)
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
 