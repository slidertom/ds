#include "StdAfx.h"
#include "jsonparser_rgb.h"

#include "jsonparser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace ds_jsonparser_rbg
{
    void SetRGB(ds_jsonparser::object &obj, const char *sField, unsigned long color) {
        CStdStringA sRGB;
	    sRGB.Format("%d", color);
        obj.SetTextUTF8(sField, sRGB.c_str());
    }

    unsigned long GetRGB(const ds_jsonparser::object &obj, const char *sField) {
        const unsigned long color = atol(obj.GetTextUTF8(sField).c_str());
        return color;
    }
};