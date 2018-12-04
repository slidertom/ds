#include "stdafx.h"
#include "jsonparser_rgb.h"

#include "jsonparser.h"

#include "string"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace ds_jsonparser_rbg
{
    void SetRGB(ds_json::object &obj, const char *sField, unsigned long color) {
        std::string sRGB = std::to_string(color);
        obj.SetTextUTF8(sField, sRGB.c_str());
    }

    unsigned long GetRGB(const ds_json::object &obj, const char *sField) {
        const unsigned long color = atol(obj.GetTextUTF8(sField).c_str());
        return color;
    }
};