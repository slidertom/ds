#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
	#include "../DBUtilsImpl.h"
#endif

#include "vector"

namespace ds_jsonparser
{
    class json_array;

    class DB_UTILS_API object
    {
    public:
        object();
        ~object();

    public:
        void SetText(const char *sField, LPCTSTR value);
        void SetTextUTF8(const char *sField, const char *sVal);
        void SetDouble(const char *sField, double value);
        void SetInteger(const char* sField, int value);
        void SetBool(const char *sField, bool bValue) { SetInteger(sField, bValue ? 1 : 0); }
        void SetObject(const char *sField, const object &obj);
        void SetArray(const char *sField, const json_array &array);

        CStdString  GetText(const char *sField) const;
        std::string GetTextUTF8(const char *sField) const;
        double      GetDouble(const char *sField) const;
        int         GetInteger(const char *sField) const;
        bool        GetBool(const char *sField) const { return GetInteger(sField) != 0; }
        void        GetArray(const char *sField, json_array &array) const;

    public:
        void *m_impl;
    };
    void DB_UTILS_API str2obj(const char* sJson, object &obj);
    void DB_UTILS_API obj2str(const object &obj, std::string &sJson);

    // json array support
    class DB_UTILS_API json_array
    {
    // Construction/Destruction
    public:
        json_array();
        ~json_array();

    // Operations
    public:
        void AddObject(const object &obj);
        void AddString(const char *str);
        void AddString(const wchar_t *str);

        int GetSize() const;
        std::string GetStringUTF8(int i) const;
        CStdString GetString(int i) const;
        void GetJsonObject(int i, object &obj) const; // prefix json used as GetObject quite general function and can be defined

    public:
        void *m_impl;
    };

    void DB_UTILS_API str2obj(const char* sJson, json_array &obj);
    void DB_UTILS_API obj2str(const json_array &obj, std::string &sJson);
};

#define FIELD_JSON(name, realname) \
	void Get##name(ds_jsonparser::object &object) const { \
        ds_jsonparser::str2obj(GetFieldStringUTF8(realname).c_str(), object); } \
	void Set##name(const ds_jsonparser::object &object) { \
        std::string sJson; ds_jsonparser::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); }

#define JSON_LONG(name, realname) \
	static long Get##name(const ds_jsonparser::object &obj) { return obj.GetInteger(realname); } \
	static void Set##name(ds_jsonparser::object &obj, long nValue) { obj.SetInteger(realname, nValue); } \

#define JSON_DOUBLE(name, realname) \
	static double Get##name(const ds_jsonparser::object &obj) { return obj.GetDouble(realname); } \
	static void Set##name(ds_jsonparser::object &obj, double dValue) { obj.SetDouble(realname, dValue); } \

#define JSON_TEXT(name, realname) \
	static CStdString Get##name(const ds_jsonparser::object &obj) { return obj.GetText(realname); } \
	static void Set##name(ds_jsonparser::object &obj, LPCTSTR sValue) { obj.SetText(realname, sValue); } \
    static std::string Get##name##UTF8(const ds_jsonparser::object &obj) { return obj.GetTextUTF8(realname); } \
    static void Set##name##UTF8(ds_jsonparser::object &obj, const char *sValue) { obj.SetTextUTF8(realname, sValue); } \

#define JSON_BOOL(name, realname) \
	static bool Get##name(const ds_jsonparser::object &obj) { return obj.GetBool(realname); } \
	static void Set##name(ds_jsonparser::object &obj, bool bValue) { obj.SetBool(realname, bValue); } \

#define JSON_ARRAY(name, realname) \
	static void Get##name(const ds_jsonparser::object &obj, ds_jsonparser::json_array &array) { obj.GetArray(realname, array); } \
	static void Set##name(ds_jsonparser::object &obj, const ds_jsonparser::json_array &array) { obj.SetArray(realname, array); } \

#endif