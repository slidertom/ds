#include "StdAfx.h"
#include "jsonparser.h"

#include "pico_impl.h"
#include "rapid_impl.h"
#include "sa_impl.h"

#include "../dsStrConv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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


#if defined(PICO)
    #define _impl _impl_pico
#elif defined(RAPID)
    #define _impl _impl_rapid
#elif defined(SA)
    #define _impl _impl_sa
#endif

namespace ds_jsonparser
{
    object::object() : m_impl(nullptr) {
        _impl::create(m_impl);
    }
    object::~object() {
        _impl::destroy(m_impl);
    }

    void object::SetText(const char* sField, LPCTSTR value) {
        const std::string value_str = ds_str_conv::ConvertToUTF8(value);
        _impl::set_field(m_impl, sField, value_str);
    }
    void object::SetTextUTF8(const char* sField, const char *sVal) {
        _impl::set_field(m_impl, sField, sVal);
    }
    void object::SetDouble(const char* sField, double value) {
        const std::string value_str = ds_str_conv::double_to_string(value);
        _impl::set_field(m_impl, sField, value_str);
    }
    void object::SetInteger(const char* sField, int value) {
        const std::string value_str = ds_str_conv::long_to_string(value);
        _impl::set_field(m_impl, sField, value_str);
    }

    CStdString object::GetText(const char* sField) const {
        std::string value_str;
        if (!_impl::get_field(m_impl, sField, value_str)) {
            return _T("");
        }
        const CStdString value = ds_str_conv::ConvertFromUTF8(value_str.c_str());
        return value;
    }
    std::string object::GetTextUTF8(const char* sField) const
    {
        std::string value_str;
        _impl::get_field(m_impl, sField, value_str);
        return value_str;
    }

    double object::GetDouble(const char* sField) const {
        std::string value_str;
        if (!_impl::get_field(m_impl, sField, value_str)) {
            return 0.0;
        }
        const double value = ds_str_conv::string_to_double(value_str.c_str()); // != pico_value->get(sField).get<double>();
        return value;
    }
    int object::GetInteger(const char* sField) const {
        std::string value_str;
        if (!_impl::get_field(m_impl, sField, value_str)) {
            return 0;
        }
        const int value = ds_str_conv::string_to_long(value_str.c_str()); // ~= atoi(value_str.c_str());
        return value;
    }

    void str2obj(const char* sJson, object &obj) {
        _impl::str2obj(sJson, obj.m_impl);
    }
    void obj2str(const object &obj, std::string &sJson) {
        _impl::obj2str(obj.m_impl, sJson);
    }

    json_array::json_array() : m_impl(nullptr) {
        _impl::create_array(m_impl);
    }
    json_array::~json_array() {
        _impl::destroy(m_impl);
    }
    void json_array::AddObject(const object &obj) {
        std::string value_str;
        obj2str(obj, value_str);
        _impl::add_array_string(m_impl, value_str.c_str());
    }
    void json_array::AddString(const char *str) {
        _impl::add_array_string(m_impl, str);
    }
    void json_array::AddString(const wchar_t *str) {
        std::string sValue = ds_str_conv::ConvertToUTF8(str);
        _impl::add_array_string(m_impl, sValue.c_str());
    }

    int json_array::GetSize() const {
        return _impl::get_array_size(m_impl);
    }
    std::string json_array::GetStringUTF8(int i) const {
        return _impl::get_value(m_impl, i);
    }
    CStdString json_array::GetString(int i) const {
        std::string sValue = _impl::get_value(m_impl, i);
        return ds_str_conv::ConvertFromUTF8(sValue.c_str());
    }
    void json_array::GetObject(int i, object &obj) const {
        const std::string sValue = GetStringUTF8(i);
        str2obj(sValue.c_str(), obj);
    }

    void str2obj(const char* sJson, json_array &obj) {
        _impl::str2obj(sJson, obj.m_impl);
    }
    void obj2str(const json_array &obj, std::string &sJson) {
        _impl::obj2str(obj.m_impl, sJson);
    }
};
