#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
	#include "../DBUtilsImpl.h"
#endif

#include "vector"

namespace ds_json
{
    class array;

    class DB_UTILS_API object
    {
    public:
        object();
        ~object();

    public:
        void SetText(const char *sField, const wchar_t *value);
        void SetTextUTF8(const char *sField, const char *sVal);
        void SetDouble(const char *sField, double value);
        void SetInteger(const char* sField, int value);
        void SetDateTime(const char* sField, time_t value);
        void SetBool(const char *sField, bool bValue) { SetInteger(sField, bValue ? 1 : 0); }
        void SetJsonObject(const char *sField, const object &obj); // json prefix applied to avoid conflicts with the general funcion name  SetObject
        void SetArray(const char *sField, const array &array);
		void SetStringArray(const char *sField, const std::vector<std::wstring> &array);
        void SetNull(const char *sField);

        std::wstring GetText(const char *sField) const;
        std::string  GetTextUTF8(const char *sField) const;
        double       GetDouble(const char *sField) const;
        int          GetInteger(const char *sField) const;
        bool         GetBool(const char *sField) const { return GetInteger(sField) != 0; }
        void         GetArray(const char *sField, array &array) const;
		void         GetStringArray(const char *sField, std::vector<std::wstring> &array) const;
        time_t       GetDateTime(const char *sField) const;
        bool         GetJsonObject(const char *sField, object &obj) const; // json prefix applied to avoid conflicts with the general funcion name GetObject
        bool         IsNull(const char *sField) const;

    private:
        object(object &x);
        const object& operator =(const object& x); //#28746 - copy is disabled by rapidjson

    public:
        void *m_impl;
    };
    void DB_UTILS_API str2obj(const char* sJson, object &obj);
    void DB_UTILS_API obj2str(const object &obj, std::string &sJson);

    // json array support
    class DB_UTILS_API array
    {
    // Construction/Destruction
    public:
        array();
        ~array();

    // Operations
    public:
        void AddObject(const object &obj);
        void SetObject(int i, const object &obj);
        void AddString(const char *str);
        void AddString(const wchar_t *str);
		void AddInt(int nValue);

        size_t GetSize() const;
        std::string GetStringUTF8(size_t i) const;
        std::wstring GetString(size_t i) const;
		int GetInt(size_t i) const;
        void GetJsonObject(size_t i, object &obj) const; // prefix json used as GetObject quite general function and can be defined

    private:
        array(array &x);
        const array& operator =(const array& x); // copy is disabled by rapidjson

    public:
        void *m_impl;
    };

    void DB_UTILS_API str2obj(const char *sJson, array &obj);
    void DB_UTILS_API obj2str(const array &obj, std::string &sJson);
};

#define FIELD_JSON(name, realname) \
	void Get##name(ds_json::object &object) const        { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
	void Set##name(const ds_json::array &object)         { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    void Get##name(ds_json::array &object) const         { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
	void Set##name(const ds_json::object &object)        { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    static bool IsNull##name(const ds_json::object &obj) { return obj.IsNull(realname); }                                                                     \
    static void SetNull##name(ds_json::object &obj)      { obj.SetNull(realname); }                                                                           \

#define JSON_LONG(name, realname) \
	static long Get##name(const ds_json::object &obj)        { return obj.GetInteger(realname);  } \
	static void Set##name(ds_json::object &obj, long nValue) { obj.SetInteger(realname, nValue); } \
    static bool IsNull##name(const ds_json::object &obj)     { return obj.IsNull(realname);      } \
    static void SetNull##name(ds_json::object &obj)          { obj.SetNull(realname);            } \

#define JSON_DOUBLE(name, realname) \
	static double Get##name(const ds_json::object &obj)        { return obj.GetDouble(realname);  } \
	static void Set##name(ds_json::object &obj, double dValue) { obj.SetDouble(realname, dValue); } \
    static bool IsNull##name(const ds_json::object &obj)       { return obj.IsNull(realname);     } \
    static void SetNull##name(ds_json::object &obj)            { obj.SetNull(realname);           } \

#define JSON_TEXT(name, realname) \
	static std::wstring Get##name(const ds_json::object &obj)                    { return obj.GetText(realname); }              \
	static void Set##name(ds_json::object &obj, const wchar_t *sValue)           { obj.SetText(realname, sValue); }             \
    static void Set##name(ds_json::object &obj, const std::wstring &sValue)      { obj.SetText(realname, sValue.c_str()); }     \
    static std::string Get##name##UTF8(const ds_json::object &obj)               { return obj.GetTextUTF8(realname); }          \
    static void Set##name##UTF8(ds_json::object &obj, const char *sValue)        { obj.SetTextUTF8(realname, sValue); }         \
    static void Set##name##UTF8(ds_json::object &obj, const std::string &sValue) { obj.SetTextUTF8(realname, sValue.c_str()); } \
    static bool IsNull##name(const ds_json::object &obj)                         { return obj.IsNull(realname); }               \
    static void SetNull##name(ds_json::object &obj)                              { obj.SetNull(realname); }                     \

#define JSON_BOOL(name, realname) \
	static bool Get##name(const ds_json::object &obj)        { return obj.GetBool(realname); }  \
	static void Set##name(ds_json::object &obj, bool bValue) { obj.SetBool(realname, bValue); } \
    static bool IsNull##name(const ds_json::object &obj)     { return obj.IsNull(realname); }   \
    static void SetNull##name(ds_json::object &obj)          { obj.SetNull(realname); }         \

#define JSON_ARRAY(name, realname) \
	static void Get##name(const ds_json::object &obj, ds_json::array &array)            { obj.GetArray(realname, array); } \
	static void Set##name(ds_json::object &obj, const ds_json::array &array)            { obj.SetArray(realname, array); } \
    static bool IsNull##name(const ds_json::object &obj)                                { return obj.IsNull(realname); }   \
    static void SetNull##name(ds_json::object &obj)                                     { obj.SetNull(realname); }         \

#define JSON_STRING_ARRAY(name, realname) \
	static void Get##name(const ds_json::object &obj, std::vector<std::wstring> &array) { obj.GetStringArray(realname, array); } \
	static void Set##name(ds_json::object &obj, const std::vector<std::wstring> &array) { obj.SetStringArray(realname, array); } \
    static bool IsNull##name(const ds_json::object &obj)                                { return obj.IsNull(realname);         } \
    static void SetNull##name(ds_json::object &obj)                                     { obj.SetNull(realname);               } \

#define JSON_DATE(name, realname) \
	static time_t Get##name(const ds_json::object &obj)        { return obj.GetDateTime(realname); }  \
	static void Set##name(ds_json::object &obj, time_t nValue) { obj.SetDateTime(realname, nValue); } \
    static bool IsNull##name(const ds_json::object &obj)       { return obj.IsNull(realname); }       \
    static void SetNull##name(ds_json::object &obj)            { obj.SetNull(realname); }             \

#define JSON_OBJECT(name, realname) \
    static void Get##name(const ds_json::object &obj, ds_json::object &get_obj) { obj.GetJsonObject(realname, get_obj); } \
	static void Set##name(ds_json::object &obj, const ds_json::object &set_obj) { obj.SetJsonObject(realname, set_obj); } \
    static bool IsNull##name(const ds_json::object &obj)                              { return obj.IsNull(realname);          } \
    static void SetNull##name(ds_json::object &obj)                                   { obj.SetNull(realname);                } \

#endif