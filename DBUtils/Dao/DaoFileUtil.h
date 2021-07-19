#ifndef __DAO_FILE_UTIL_H__
#define __DAO_FILE_UTIL_H__
#pragma once

class CDaoFileUtil
{
// Static operations
public:
    static bool IsDaoDB(const wchar_t *sPath)
    {
        FILE *pFile = _tfopen(sPath, L"rb");
        if ( !pFile ) {
            return false;
        }
        //  Standard Jet DB
    
        bool bDao = false;
        char buffer[16];
        size_t nRead = fread(buffer, 1, 16, pFile);
        if ( nRead > 15 && 
                buffer[4]  == 'S' && 
                buffer[5]  == 't' && 
                buffer[6]  == 'a' && 
                buffer[7]  == 'n' && 
                buffer[8]  == 'd' && 
                buffer[9]  == 'a' &&
                buffer[10] == 'r' && 
                buffer[11] == 'd' && 
                buffer[13] == 'J' && 
                buffer[14] == 'e' &&
                buffer[15] == 't' ) 
        {
            bDao = true;
        }

        fclose(pFile);

        return bDao;
    }
};

#endif