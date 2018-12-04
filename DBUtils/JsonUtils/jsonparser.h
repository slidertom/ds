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
        void SetText(const char *sField, const wchar_t *sVal) noexcept;
        void SetTextEx(const char *sField, const wchar_t *sVal) noexcept {
            if ( ::wcslen(sVal) > 0 ) { 
                SetText(sField, sVal);
            }
            #ifdef _DEBUG
            else {
                ASSERT(GetText(sField).empty()); // should match default value if exists
            }
            #endif
        }
        void SetTextUTF8(const char *sField, const char *sVal) noexcept;
        void SetTextUTF8Ex(const char *sField, const char *sVal) noexcept {
            if ( ::strlen(sVal) > 0 ) { 
                SetTextUTF8(sField, sVal);         
            } 
            #ifdef _DEBUG
            else {
                ASSERT(GetTextUTF8(sField).empty()); // should match default value if exists
            }
            #endif
        } 
        void SetDouble(const char *sField, double dValue) noexcept;
        void SetDoubleEx(const char *sField, double dValue) noexcept {
            if ( (dValue > DBL_EPSILON) || (dValue < -DBL_EPSILON) ) { 
                SetDouble(sField, dValue); 
            } 
            #ifdef _DEBUG
            else {
                const double dCheck = GetDouble(sField);
                ASSERT((dCheck < DBL_EPSILON) && (dCheck > -DBL_EPSILON)); // should match default value if exists
            }
            #endif
        }
        void SetInt32(const char* sField, int32_t nValue) noexcept;
        void SetInt32Ex(const char* sField, int32_t nValue) noexcept {
            if ( nValue != 0) { 
                SetInt32(sField, nValue); 
            }
            #ifdef _DEBUG
            else {
                ASSERT(GetInt32(sField) == 0); // should match default value if exists
            }
            #endif
        }
        void SetInt64(const char* sField, int64_t nValue) noexcept;
        void SetDateTime(const char* sField, time_t value) noexcept;
        void SetBool(const char *sField, bool bValue) noexcept { SetInt32(sField, bValue ? 1 : 0); }
        void SetBoolEx(const char *sField, bool bValue) noexcept { 
            if ( bValue ) { 
                SetBool(sField, bValue); 
            }
            #ifdef _DEBUG
            else {
                ASSERT(!GetBool(sField)); // should match default value if exists
            }
            #endif
        }
        void SetJsonObject(const char *sField, const object &obj) noexcept; // json prefix applied to avoid conflicts with the general funcion name  SetObject
        void SetArray(const char *sField, const array &array) noexcept;
        void SetArrayEx(const char *sField, const array &array) noexcept;
        void SetStringArray(const char *sField, const std::vector<std::wstring> &array) noexcept;
        void SetNull(const char *sField) noexcept;

        std::wstring GetText(const char *sField) const noexcept;
        std::string  GetTextUTF8(const char *sField) const noexcept;
        double       GetDouble(const char *sField) const noexcept;
        int32_t      GetInt32(const char *sField) const noexcept;
        int64_t      GetInt64(const char *sField) const noexcept;
        bool         GetBool(const char *sField) const noexcept { return GetInt32(sField) != 0; }
        void         GetArray(const char *sField, array &array) const noexcept;
        void         GetStringArray(const char *sField, std::vector<std::wstring> &array) const noexcept;
        time_t       GetDateTime(const char *sField) const noexcept;
        bool         GetJsonObject(const char *sField, object &obj) const noexcept; // json prefix applied to avoid conflicts with the general funcion name GetObject
        bool         IsNull(const char *sField) const noexcept;

        bool Remove(const char *sField) noexcept;

    private:
        object(object &x) = delete;
        const object& operator =(const object& x) = delete; // copy is disabled by rapidjson

    public:
        void *m_impl {nullptr};
    };
    void DB_UTILS_API str2obj(const char* sJson, object &obj) noexcept;
    void DB_UTILS_API obj2str(const object &obj, std::string &sJson) noexcept;

    // json array support
    class DB_UTILS_API array
    {
    // Construction/Destruction
    public:
        array();
        ~array();

    // Operations
    public:
        void AddObject(const object &obj) noexcept;
        void SetObject(size_t i, const object &obj) noexcept;
        void AddString(const char *str) noexcept;
        void AddString(const wchar_t *str) noexcept;
        void AddInt32(int32_t nValue) noexcept;
        void AddInt64(int64_t nValue) noexcept;
        void AddDouble(double dValue) noexcept;
        void AddFloat(float fValue) noexcept;
        void AddArray(const array &array) noexcept;

        size_t GetSize() const noexcept;
        std::string GetStringUTF8(size_t i) const noexcept;
        std::wstring GetString(size_t i) const noexcept;
        int32_t GetInt32(size_t i) const noexcept;
        int64_t GetInt64(size_t i) const noexcept;
        void GetJsonObject(size_t i, object &obj) const noexcept; // prefix json used as GetObject quite general function and can be defined

    private:
        array(array &x)                         = delete;
        const array& operator =(const array& x) = delete; // copy is disabled by rapidjson

    public:
        void *m_impl {nullptr};
    };

    void DB_UTILS_API str2obj(const char *sJson, array &obj);
    void DB_UTILS_API obj2str(const array &obj, std::string &sJson);

    // do use object_vect as a cache if required multi time access to the array object elements
    class object_vect : public std::vector<ds_json::object *> {
    // Construction/Destruction
    public:
        object_vect() { }
        ~object_vect() {
            for (auto *pObj : *this) {
                delete pObj;
            }
            this->clear();
        }

    private:
        object_vect(const object_vect &x)    = delete;
        void operator=(const object_vect &x) = delete;
    };

    inline void array2vect(const ds_json::array &arr, object_vect &v) noexcept
    {
        const size_t nCnt = arr.GetSize();
        v.reserve(nCnt);
        for (size_t i = 0; i < nCnt; ++i)
        {
            ds_json::object *pObj = new ds_json::object;
            arr.GetJsonObject(i, *pObj);
            v.push_back(pObj);
        }
    }
};

#define FIELD_JSON(name, realname) \
    void Get##name(ds_json::object &object) const        { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
    void Set##name(const ds_json::array &object)         { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    void Get##name(ds_json::array &object) const         { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
    void Set##name(const ds_json::object &object)        { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    bool DeleteAllByJson##name(const char *sJsonField, int32_t nValue) noexcept { return DeleteAllByJsonField(realname, sJsonField, nValue); }                \
    static bool IsNull##name(const ds_json::object &obj) { return obj.IsNull(realname); }                                                                     \
    static void SetNull##name(ds_json::object &obj)      { obj.SetNull(realname); }                                                                           \

#define JSON_NULL(name, realname) \
    static bool IsNull##name(const ds_json::object &obj)        { return obj.IsNull(realname);     } \
    static void SetNull##name(ds_json::object &obj)             { obj.SetNull(realname);           } \

#define JSON_REMOVE(name, realname) \
    static bool Remove##name(ds_json::object &obj)        { return obj.Remove(realname);     } \

#define JSON_INT32(name, realname) \
    static int32_t Get##name(const ds_json::object &obj)        { return obj.GetInt32(realname);   } \
    static void Set##name(ds_json::object &obj, int32_t nValue) { obj.SetInt32(realname, nValue);  } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

// default: 0 -> if default do not save
#define JSON_INT32_EX(name, realname) \
    static int32_t Get##name(const ds_json::object &obj)        { return obj.GetInt32(realname);  } \
    static void Set##name(ds_json::object &obj, int32_t nValue) { obj.SetInt32Ex(realname, nValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_INT64(name, realname) \
    static int64_t Get##name(const ds_json::object &obj)         { return obj.GetInt64(realname);    } \
    static void Set##name(ds_json::object &obj, int64_t  nValue) { obj.SetInt64(realname, nValue);   } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_DOUBLE(name, realname) \
    static double Get##name(const ds_json::object &obj)        { return obj.GetDouble(realname);  } \
    static void Set##name(ds_json::object &obj, double dValue) { obj.SetDouble(realname, dValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

// default: 0.0 -> if default do not save
#define JSON_DOUBLE_EX(name, realname) \
    static double Get##name(const ds_json::object &obj)        { return obj.GetDouble(realname);    } \
    static void Set##name(ds_json::object &obj, double dValue) { obj.SetDoubleEx(realname, dValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_TEXT(name, realname) \
    static std::wstring Get##name(const ds_json::object &obj)                    { return obj.GetText(realname); }              \
    static void Set##name(ds_json::object &obj, const wchar_t *sValue)           { obj.SetText(realname, sValue); }             \
    static void Set##name(ds_json::object &obj, const std::wstring &sValue)      { obj.SetText(realname, sValue.c_str()); }     \
    static std::string Get##name##UTF8(const ds_json::object &obj)               { return obj.GetTextUTF8(realname); }          \
    static void Set##name##UTF8(ds_json::object &obj, const char *sValue)        { obj.SetTextUTF8(realname, sValue); }         \
    static void Set##name##UTF8(ds_json::object &obj, const std::string &sValue) { obj.SetTextUTF8(realname, sValue.c_str()); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

// default: empty string -> if default do not save
#define JSON_TEXT_EX(name, realname) \
    static std::wstring Get##name(const ds_json::object &obj)                    { return obj.GetText(realname);                } \
    static void Set##name(ds_json::object &obj, const wchar_t *sValue)           { obj.SetTextEx(realname, sValue);             } \
    static void Set##name(ds_json::object &obj, const std::wstring &sValue)      { obj.SetTextEx(realname, sValue.c_str());     } \
    static std::string Get##name##UTF8(const ds_json::object &obj)               { return obj.GetTextUTF8(realname);            } \
    static void Set##name##UTF8(ds_json::object &obj, const char *sValue)        { obj.SetTextUTF8Ex(realname, sValue);         } \
    static void Set##name##UTF8(ds_json::object &obj, const std::string &sValue) { obj.SetTextUTF8Ex(realname, sValue.c_str()); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_BOOL(name, realname) \
    static bool Get##name(const ds_json::object &obj)        { return obj.GetBool(realname); }  \
    static void Set##name(ds_json::object &obj, bool bValue) { obj.SetBool(realname, bValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

// default: false
#define JSON_BOOL_EX(name, realname) \
    static bool Get##name(const ds_json::object &obj)        { return obj.GetBool(realname);    } \
    static void Set##name(ds_json::object &obj, bool bValue) { obj.SetBoolEx(realname, bValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_ARRAY(name, realname) \
    static void Get##name(const ds_json::object &obj, ds_json::array &array)            { obj.GetArray(realname, array); } \
    static void Set##name(ds_json::object &obj, const ds_json::array &array)            { obj.SetArray(realname, array); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

// if array empty -> do no create a record
#define JSON_ARRAY_EX(name, realname) \
    static void Get##name(const ds_json::object &obj, ds_json::array &array)            { obj.GetArray(realname, array);   } \
    static void Set##name(ds_json::object &obj, const ds_json::array &array)            { obj.SetArrayEx(realname, array); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_STRING_ARRAY(name, realname) \
    static void Get##name(const ds_json::object &obj, std::vector<std::wstring> &array) { obj.GetStringArray(realname, array); } \
    static void Set##name(ds_json::object &obj, const std::vector<std::wstring> &array) { obj.SetStringArray(realname, array); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_DATE(name, realname) \
    static time_t Get##name(const ds_json::object &obj)        { return obj.GetDateTime(realname); }  \
    static void Set##name(ds_json::object &obj, time_t nValue) { obj.SetDateTime(realname, nValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_OBJECT(name, realname) \
    static void Get##name(const ds_json::object &obj, ds_json::object &get_obj) { obj.GetJsonObject(realname, get_obj); } \
    static void Set##name(ds_json::object &obj, const ds_json::object &set_obj) { obj.SetJsonObject(realname, set_obj); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#endif