#ifndef __JSON_PARSER_RGB_H__
#define __JSON_PARSER_RGB_H__
#pragma once

namespace ds_json { class object; };

namespace ds_jsonparser_rbg
{
    void DB_UTILS_API SetRGB(ds_json::object &obj, const char *sField, unsigned long color);
    unsigned long DB_UTILS_API GetRGB(const ds_json::object &obj, const char *sField);
};

#define JSON_RGB(name, realname) \
	static unsigned long Get##name(const ds_json::object &obj) { return ds_jsonparser_rbg::GetRGB(obj, realname); } \
	static void Set##name(ds_json::object &obj, unsigned long color) { ds_jsonparser_rbg::SetRGB(obj, realname, color); } 

#endif