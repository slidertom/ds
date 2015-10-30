#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
	#include "../DBUtilsImpl.h"
#endif

#include "vector"

namespace ds_jsonparser
{
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

        CStdString GetText(const char *sField) const;
        std::string GetTextUTF8(const char *sField) const;
        double     GetDouble(const char *sField) const;
        int        GetInteger(const char *sField) const;
        bool       GetBool(const char *sField) const { return GetInteger(sField) != 0; }

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
        void GetObject(int i, object &obj) const;

    public:
        void *m_impl;
    };
    void DB_UTILS_API str2obj(const char* sJson, json_array &obj);
    void DB_UTILS_API obj2str(const json_array &obj, std::string &sJson);

    // json utils
    template <class TString>
    inline void str_array_to_json(const std::vector<TString> &arr, std::string &sJson) 
    {
        json_array obj;

        auto end_it = arr.end();
        for (auto it = arr.begin(); it != end_it; ++it) {
            obj.AddString((*it).c_str());
        }

        obj2str(obj, sJson);
    }

    template <class TString> // TString -> wchar_t base expected
    inline void json_to_str_array(const char *sJson, std::vector<TString> &arr) 
    {
        arr.clear();

        json_array obj;
        str2obj(sJson, obj);

        const int nCnt = obj.GetSize();
        for (int i1 = 0; i1 < nCnt; ++i1) {
             arr.push_back(obj.GetString(i1));
        }
    }
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

#endif