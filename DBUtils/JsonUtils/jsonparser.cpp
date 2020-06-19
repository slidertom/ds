#include "stdafx.h"
#include "jsonparser.h"

#include "rapid_impl.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

//********************************
//#define PICO //Use PicoJson
#define RAPID  //Use RapidJson
//#define SA   //Use SingleAllocationJson
//********************************
// TEST RESULTS
// field count 20
// record count 10000
// Text length in field 20
// ----------------------------------
//   1.504 s. Save 'clasic'
//   0.825 s. Save JSON RAPID
//   1.146 s. Save JSON PICO
// ----------------------------------
//   0.165 s. Load 'clasic'
//   0.318 s. Load JSON RAPID
//   0.440 s. Load JSON PICO
//********************************

#if defined(RAPID)
    #define _impl _impl_rapid
#endif

namespace ds_json
{
    object::object() {
        _impl::create(m_impl);
    }
    object::~object() {
        _impl::destroy(m_impl);
    }

    void object::SetText(const char *sField, const wchar_t *sVal) noexcept {
        const std::string value_str = ds_str_conv::ConvertToUTF8(sVal);
        _impl::set_field_string(m_impl, sField, value_str.c_str());
    }
    void object::SetTextUTF8(const char *sField, const char *sVal) noexcept {
        _impl::set_field_string(m_impl, sField, sVal);
    }
    void object::SetDouble(const char *sField, double dValue) noexcept {
        _impl::set_field_double(m_impl, sField, dValue);
    }
    void object::SetInt32(const char *sField, int32_t nValue) noexcept {
        _impl::set_field_int32(m_impl, sField, nValue);
    }
    void object::SetInt64(const char* sField, int64_t nValue) noexcept {
        _impl::set_field_int64(m_impl, sField, nValue);
    }
    void object::SetArray(const char *sField, const array &array) noexcept {
        _impl::set_field_array(m_impl, sField, array.m_impl);
    }
    void object::SetArrayEx(const char *sField, const array &array) noexcept {
        if ( array.GetSize() > 0 ) { 
            SetArray(sField, array); 
        }
        #ifdef _DEBUG
        else {
            ds_json::array array_check;
            GetArray(sField, array_check); 
            ASSERT(array_check.GetSize() == 0); // should match default value
        }
        #endif
    }

    void object::SetStringArrayUTF8(const char *sField, const std::vector<std::string> &array) noexcept
    {
        ds_json::array array_json;
        for (const auto &it : array) {
            array_json.AddString(it.c_str());
        }
        _impl::set_field_array(m_impl, sField, array_json.m_impl);
    }

    void object::SetStringArray(const char *sField, const std::vector<std::wstring> &array) noexcept 
    {
        ds_json::array array_json;
        for (const auto &it : array) {
            array_json.AddString(it.c_str());
        }
        _impl::set_field_array(m_impl, sField, array_json.m_impl);
    }
    void object::SetJsonObject(const char *sField, const object &obj) noexcept {
        _impl::set_field_object(m_impl, sField, obj.m_impl);
    }
    void object::SetDateTime(const char* sField, time_t value) noexcept {
        _impl::set_field_date_time(m_impl, sField, value);
    }
    std::wstring object::GetText(const char *sField) const noexcept {
        std::string value_str;
        if (!_impl::get_field_string(m_impl, sField, value_str)) {
            return L"";
        }
        const std::wstring value = ds_str_conv::ConvertFromUTF8(value_str.c_str());
        return value;
    }
    std::string object::GetTextUTF8(const char *sField) const noexcept 
    {
        std::string value_str;
        _impl::get_field_string(m_impl, sField, value_str);
        return value_str;
    }
    void object::GetArray(const char *sField, array &array) const noexcept {
        _impl::get_field_array(m_impl, sField, array.m_impl);
    }
    void object::GetStringArray(const char *sField, std::vector<std::wstring> &array) const noexcept
    {
        ds_json::array array_json;
        _impl::get_field_array(m_impl, sField, array_json.m_impl);
        const size_t nCnt = array_json.GetSize();
        for (size_t i = 0; i < nCnt; ++i) {
            array.push_back(array_json.GetString(i));
        }
    }
    void object::GetStringArrayUTF8(const char *sField, std::vector<std::string> &array) const noexcept
    {
        ds_json::array array_json;
        _impl::get_field_array(m_impl, sField, array_json.m_impl);
        const size_t nCnt = array_json.GetSize();
        for (size_t i = 0; i < nCnt; ++i) {
            array.push_back(array_json.GetStringUTF8(i));
        }
    }
    double object::GetDouble(const char *sField) const noexcept {
        double dValue; 
        if (!_impl::get_field_double(m_impl, sField, dValue)) {
            return 0.0;
        }
        return dValue;
    }
    int32_t object::GetInt32(const char *sField) const noexcept {
        int32_t nValue = 0;
        if (!_impl::get_field_int32(m_impl, sField, nValue)) {
            return 0;
        }
        return nValue;
    }
    int64_t object::GetInt64(const char *sField) const noexcept {
        int64_t nValue = 0;
        if (!_impl::get_field_int64(m_impl, sField, nValue)) {
            return 0;
        }
        return nValue;
    }

    bool object::GetJsonObject(const char *sField, object &obj) const noexcept {
        return _impl::get_field_object(m_impl, sField, obj.m_impl);
    }
    time_t object::GetDateTime(const char *sField) const noexcept {
        time_t nValue = 0;
        if (!_impl::get_field_date_time(m_impl, sField, nValue)) {
            return 0;
        }
        return nValue;
    }
    bool object::IsNull(const char *sField) const noexcept {
        return _impl::get_field_null(m_impl, sField);
    }
    void object::SetNull(const char *sField) noexcept {
        return _impl::set_field_null(m_impl, sField);
    }

    bool object::Remove(const char *sField) noexcept {
         return _impl::remove_field(m_impl, sField);
    }

    void str2obj(const char* sJson, object &obj) noexcept {
        _impl::str2obj(sJson, obj.m_impl);
    }
    void obj2str(const object &obj, std::string &sJson) noexcept {
        _impl::obj2str(obj.m_impl, sJson);
    }

    array::array() {
        _impl::create_array(m_impl);
    }
    array::array(array &&ar) {
        m_impl = ar.m_impl;
        ar.m_impl = nullptr;
    }
    array::~array() {
        _impl::destroy(m_impl);
    }
    array &array::operator=(array &&ar) {
        _impl::destroy(m_impl);
        m_impl = ar.m_impl;
        ar.m_impl = nullptr;
        return *this;
    }
    void array::AddObject(const object &obj) noexcept {
        _impl::add_array_object(m_impl, obj.m_impl);
    }
    /*
    void array::AddObject(object &&obj) noexcept {
         _impl::add_array_move_object(m_impl, obj.m_impl);
         obj.m_impl = nullptr;
    }
    */
    void array::SetObject(size_t i, const object &obj) noexcept {
        _impl::set_array_object(m_impl, i, obj.m_impl);
    }
    /*
    void array::AddArray(array &&array) noexcept {
        _impl::add_array_move_object(m_impl, array.m_impl);
        array.m_impl = nullptr;
    }
    */
    void array::AddArray(const array &array) noexcept {
        _impl::add_array_object(m_impl, array.m_impl);
    }
    void array::AddString(const char *str) noexcept {
        _impl::add_array_string(m_impl, str);
    }
    void array::AddString(const wchar_t *str) noexcept {
        std::string sValue = ds_str_conv::ConvertToUTF8(str);
        _impl::add_array_string(m_impl, sValue.c_str());
    }
    void array::AddInt64(int64_t nValue) noexcept {
        _impl::add_array_int64(m_impl, nValue);
    }
    void array::AddInt32(int32_t nValue) noexcept {
        _impl::add_array_int32(m_impl, nValue);
    }
    void array::AddDouble(double dValue) noexcept {
        _impl::add_array_double(m_impl, dValue);
    }
    void array::AddFloat(float fValue) noexcept {
        _impl::add_array_float(m_impl, fValue);
    }

    size_t array::size() const noexcept {
        return _impl::get_array_size(m_impl);
    }
    size_t array::GetSize() const noexcept {
        return _impl::get_array_size(m_impl);
    }
    std::string array::GetStringUTF8(size_t i) const noexcept {
        std::string sValue;
        _impl::get_array_string(m_impl, i, sValue);
        return sValue;
    }
    std::wstring array::GetString(size_t i) const noexcept {
        const std::string sValue = GetStringUTF8(i);
        return ds_str_conv::ConvertFromUTF8(sValue.c_str());
    }
    int64_t array::GetInt64(size_t i) const noexcept {
        return _impl::get_array_int64(m_impl, i);
    }
    int32_t array::GetInt32(size_t i) const noexcept {
        return _impl::get_array_int32(m_impl, i);
    }

    array::iterator array::begin() {
        return iterator(*this, 0);
    }

    array::iterator array::end() {
        return iterator(*this, GetSize());
    }

    void array::GetJsonObject(size_t i, object &obj) const noexcept {
        _impl::get_array_object(m_impl, i, obj.m_impl);
    }
    
    void str2obj(const char* sJson, array &obj) noexcept{
        _impl::str2obj(sJson, obj.m_impl);
    }
    void obj2str(const array &obj, std::string &sJson) noexcept {
        _impl::obj2str(obj.m_impl, sJson);
    }
    void str2obj(const char *sJson, std::vector<int32_t> &v) noexcept {
        ds_json::array arr;
        str2obj(sJson, arr);
        size_t nSize = arr.size();
        v.reserve(nSize);
        for (size_t i1 = 0; i1 < nSize; ++i1) {
            v.push_back(arr.GetInt32(i1));
        }
    }
};
