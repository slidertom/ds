#include "StdAfx.h"
#include "dsFieldBitmap.h"

#include "MxKImage/Include/MxKImageAll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace ds_table_ext
{
    CBitmap *GetFieldBitmap(const dsTable &table, const wchar_t *sFieldName)
    {
        unsigned char *pData = nullptr;
	    unsigned long nSize = 0;
        table.GetFieldBinary(sFieldName, &pData, nSize);
        CBitmap *pBitmap = CBitmapConverter::DecodeBitmap(pData, nSize);
        delete pData;
	    return pBitmap;
    }

    bool SetFieldBitmap(dsTable &table, const wchar_t *sFieldName, CBitmap *pBmp, bool bUseColors)
    {
	    if ( !pBmp ) {
		    return false;
	    }

	    unsigned char *data = nullptr;
	    long nSize;

	    if ( !CBitmapConverter::EncodeBitmap(pBmp, data, nSize, CBitmapConverter::MxKImage_JPEG, bUseColors) ) {
		    return false;
	    }
	
	    table.SetFieldBinary(sFieldName, data, nSize);
        /*
#ifdef _DEBUG
        FILE* file = fopen( "D:/image.jpg", "wb" );
        fwrite(data, 1, nSize, file);
        fclose(file);
#endif
*/
	    ::free(data);
	    return true;
    }
};