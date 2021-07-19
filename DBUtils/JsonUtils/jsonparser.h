#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
    #include "../DBUtilsImpl.h"
#endif

#include "vector"
#include "unordered_set"
#include "string"

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
                ASSERT(GetText(sField).empty()); // should match default value if exists also indicates that not the whole object datastructure is being saved
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
                ASSERT(GetTextUTF8(sField).empty()); // should match default value if exists also indicates that not the whole object datastructure is being saved
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
                ASSERT((dCheck < DBL_EPSILON) && (dCheck > -DBL_EPSILON)); // should match default value if exists also indicates that not the whole object datastructure is being saved
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
                ASSERT(!GetBool(sField)); // should match default value if exists, also indicates that not the whole object datastructure is being saved
            }
            #endif
        }
        // json prefix applied to avoid conflicts with the general funcion name  SetObject
        void SetJsonObject(const char *sField, const object &obj) noexcept; 
        void SetArray(const char *sField, const array &array) noexcept;
        void SetArrayEx(const char *sField, const array &array) noexcept;
        void SetStringArray(const char *sField, const std::vector<std::wstring> &array) noexcept;
        void SetStringArrayUTF8(const char *sField, const std::vector<std::string> &array) noexcept;
        void SetInt32Array(const char *sField, const std::vector<int32_t> &array) const noexcept;
        void SetNull(const char *sField) noexcept;

        std::wstring GetText(const char *sField) const noexcept;
        std::string  GetTextUTF8(const char *sField) const noexcept;
        double       GetDouble(const char *sField) const noexcept;
        int32_t      GetInt32(const char *sField) const noexcept;
        int64_t      GetInt64(const char *sField) const noexcept;
        bool         GetBool(const char *sField) const noexcept { return GetInt32(sField) != 0; }
        void         GetArray(const char *sField, array &array) const noexcept;
        void         GetStringArray(const char *sField, std::vector<std::wstring> &array) const noexcept;
        void         GetStringArrayUTF8(const char *sField, std::vector<std::string> &array) const noexcept;
        void         GetInt32Array(const char *sField, std::vector<int32_t> &array) const noexcept;
            
        time_t       GetDateTime(const char *sField) const noexcept;
        // json prefix applied to avoid conflicts with the general funcion name GetObject
        bool         GetJsonObject(const char *sField, object &obj) const noexcept; 
        bool         IsNull(const char *sField) const noexcept;
        
        bool Remove(const char *sField) noexcept;

    private:
        object(object &x) = delete;
        const object& operator=(const object& x) = delete; // copy is disabled by rapidjson

    public:
        void *m_impl {nullptr};
    };

    void DB_UTILS_API str2obj(const char *sJson, object &obj) noexcept;
    void DB_UTILS_API str2obj(const wchar_t *sJson, object &obj) noexcept;
    void DB_UTILS_API obj2str(const object &obj, std::string &sJson) noexcept;
    void DB_UTILS_API obj2str(const object &obj, std::wstring &sJson) noexcept;

    // json array support
    class DB_UTILS_API array
    {
    // Construction/Destruction
    public:
        array();
        array(array &&ar); 
        ~array();

    // Operators
    public:
        array &operator=(array &&x);

    // Operations
    public:
        void AddObject(const object &obj) noexcept;
        //void AddObject(object &&obj) noexcept;
        void SetObject(size_t i, const object &obj) noexcept;
        void AddString(const char *str) noexcept;
        void AddString(const wchar_t *str) noexcept;
        void AddInt32(int32_t nValue) noexcept;
        void AddInt64(int64_t nValue) noexcept;
        void AddDouble(double dValue) noexcept;
        void AddFloat(float fValue) noexcept;
        void AddArray(const array &array) noexcept;
        //void AddArray(array &&array) noexcept;

        size_t size() const noexcept;
        std::string GetStringUTF8(size_t i) const noexcept;
        std::wstring GetString(size_t i) const noexcept;
        int32_t GetInt32(size_t i) const noexcept;
        int64_t GetInt64(size_t i) const noexcept;
        double GetDouble(size_t i) const noexcept;
        // prefix json used as GetObject quite general function and can be defined
        void GetJsonObject(size_t i, object &obj) const noexcept; 

     public:
        class DB_UTILS_API array_object final
        {
        public:    
            using reference = typename object&;
            using pointer   = typename object*;

            array_object() = delete;
            array_object(array_object &&x) = delete;
            array_object(const array_object &x) : m_pArr(x.m_pArr), m_nPos(x.m_nPos) { }
            array_object(const array &arr, size_t nPos) : m_pArr(&arr), m_nPos(nPos) { }
            ~array_object() = default;
            
        public:
            operator int32_t() const noexcept;
            operator int64_t() const noexcept;
            operator reference() const noexcept;
            operator bool() const noexcept;
            operator double() const noexcept;
            operator std::string() const noexcept;
            operator std::wstring() const noexcept;

        // Attributes
        public:
            const array *m_pArr;
            size_t m_nPos;
            mutable ds_json::object m_object;
        };

        class DB_UTILS_API iterator final
        {
        public:
            using reference = typename array_object&;
            using pointer   = typename array_object*;

            iterator(const array &arr, const size_t nPos) : m_array_object(arr, nPos) { }
            iterator(const iterator &x) : m_array_object(x.m_array_object) { }
            ~iterator() { }

        // Operators
        public:
            iterator &operator++()   { ++m_array_object.m_nPos; return *this; } // prefix++
            iterator operator++(int) { ++m_array_object.m_nPos; return iterator(*this); } // postfix++ 

            bool operator!=(iterator &rhs) const { return m_array_object.m_nPos != rhs.m_array_object.m_nPos; }

            reference operator*() const;
            pointer operator->() const { return &(operator*()); }

        // Attributes 
        private:
            mutable array_object m_array_object;
        };

    public:
        iterator begin() { return iterator(*this, 0); }
        iterator end() { return iterator(*this, size()); }
        iterator begin() const { return iterator(*this, 0); }
        iterator end() const { return iterator(*this, size()); }

    private:
        array(array &x)                        = delete;
        const array& operator=(const array& x) = delete; // copy is disabled by rapidjson

    public:
        void *m_impl {nullptr};
    };

    void DB_UTILS_API str2obj(const char *sJson, array &obj) noexcept;
    void DB_UTILS_API str2obj(const char *sJson, std::vector<int32_t> &v) noexcept;
    void DB_UTILS_API str2obj(const char *sJson, std::vector<std::string> &v) noexcept;
    void DB_UTILS_API str2obj(const char *sJson, std::unordered_set<std::string> &v) noexcept;
    void DB_UTILS_API obj2str(const array &obj, std::string &sJson) noexcept;

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

    template <class ds_json_array>
    inline void array2vect(const ds_json_array &arr, object_vect &v) noexcept
    {
        const size_t nCnt = arr.size();
        v.reserve(nCnt);
        for (size_t i = 0; i < nCnt; ++i) {
            ds_json::object *pObj = new ds_json::object;
            arr.GetJsonObject(i, *pObj);
            v.push_back(pObj);
        }
    }
};

namespace ds_jsonparser_rbg
{
    void DB_UTILS_API SetRGB(ds_json::object &obj, const char *sField, unsigned long color);
    unsigned long DB_UTILS_API GetRGB(const ds_json::object &obj, const char *sField);
};

#define FIELD_JSON(name, realname) \
    void Get##name(ds_json::object &object) const        { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
    void Set##name(const ds_json::array &object)         { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    void Get##name(ds_json::array &object) const         { ds_json::str2obj(GetFieldStringUTF8(realname).c_str(), object); }                                  \
    void Set##name(const ds_json::object &object)        { std::string sJson; ds_json::obj2str(object, sJson); SetFieldStringUTF8(realname, sJson.c_str()); } \
    void Set##name(const char *sJson)                    { SetFieldStringUTF8(realname, sJson);         }                                                     \
    void Set##name(const std::string &sJson)             { SetFieldStringUTF8(realname, sJson.c_str()); }                                                     \
    bool DeleteAllByJson##name(const char *sJsonField, int32_t nValue)        noexcept { return DeleteAllByJsonField(realname, sJsonField, nValue); }         \
    bool DeleteAllByJson##name(const char *sJsonField, const wchar_t *sValue) noexcept { return DeleteAllByJsonField(realname, sJsonField, sValue); }         \
    bool SeekByJson##name(const char *sJsonField, const wchar_t *sValue) noexcept { return SeekByJsonField(realname, sJsonField, sValue); }                   \
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
    static void Get##name##UTF8(const ds_json::object &obj, std::vector<std::string> &array) { obj.GetStringArrayUTF8(realname, array); } \
    static void Set##name##UTF8(ds_json::object &obj, const std::vector<std::string> &array) { obj.SetStringArrayUTF8(realname, array); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_INT32_ARRAY(name, realname) \
    static void Get##name(const ds_json::object &obj, std::vector<int32_t> &array) { obj.GetInt32Array(realname, array); } \
    static void Set##name(ds_json::object &obj, const std::vector<int32_t> &array) { obj.SetInt32Array(realname, array); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_DATE(name, realname) \
    static time_t Get##name(const ds_json::object &obj)        { return obj.GetDateTime(realname); }  \
    static void Set##name(ds_json::object &obj, time_t nValue) { obj.SetDateTime(realname, nValue); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_OBJECT(name, realname) \
    static bool Get##name(const ds_json::object &obj, ds_json::object &get_obj) { return obj.GetJsonObject(realname, get_obj); } \
    static void Set##name(ds_json::object &obj, const ds_json::object &set_obj) { obj.SetJsonObject(realname, set_obj); } \
    JSON_NULL(name, realname) \
    JSON_REMOVE(name, realname) \

#define JSON_RGB(name, realname) \
    static unsigned long Get##name(const ds_json::object &obj) { return ds_jsonparser_rbg::GetRGB(obj, realname); } \
    static void Set##name(ds_json::object &obj, unsigned long color) { ds_jsonparser_rbg::SetRGB(obj, realname, color); } 

#endif