#include "StdAfx.h"
#include "dsStrConv.h"

#include "sstream"
#include "iomanip"
#include "limits"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Convert a wide Unicode string to an UTF8 string
namespace ds_str_conv
{
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
    CStdString ConvertFromUTF8(const char *str)
    {
        if ( !str ) {
            return CStdString();
        }

        const int nLen = strlen(str);
        if ( nLen <= 0 ) {
            return CStdString();
        }

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, NULL, 0);

        CStdString wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], nLen, &wstrTo[0], size_needed);

        return wstrTo;
    }

    std::string long_to_string(long nValue)
    {
        std::string sValue;
        std::stringstream strstream;
        strstream << nValue;
        strstream >> sValue;
        return sValue;
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
};
 