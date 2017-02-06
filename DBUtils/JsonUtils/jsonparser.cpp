#include "StdAfx.h"
#include "jsonparser.h"

#include "rapid_impl.h"

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

#if defined(RAPID)
    #define _impl _impl_rapid
#endif

namespace ds_jsonparser
{
    object::object() : m_impl(nullptr) {
        _impl::create(m_impl);
    }
    object::~object() {
        _impl::destroy(m_impl);
    }

    void object::SetText(const char* sField, const wchar_t *value) {
        const std::string value_str = ds_str_conv::ConvertToUTF8(value);
        _impl::set_field_string(m_impl, sField, value_str.c_str());
    }
    void object::SetTextUTF8(const char* sField, const char *sVal) {
        _impl::set_field_string(m_impl, sField, sVal);
    }
    void object::SetDouble(const char* sField, double value) {
        _impl::set_field_double(m_impl, sField, value);
    }
    void object::SetInteger(const char* sField, int value) {
        _impl::set_field_int(m_impl, sField, value);
    }
    void object::SetArray(const char *sField, const json_array &array) {
        _impl::set_field_array(m_impl, sField, array.m_impl);
    }
	void object::SetStringArray(const char *sField, const std::vector<std::wstring> &array)
	{
		ds_jsonparser::json_array array_json;
		for ( const auto &it : array ) {
			array_json.AddString(it.c_str());
		}
		_impl::set_field_array(m_impl, sField, array_json.m_impl);
	}
    void object::SetObject(const char *sField, const object &obj) {
        _impl::set_field_object(m_impl, sField, obj.m_impl);
    }
    void object::SetDateTime(const char* sField, time_t value) {
        _impl::set_field_date_time(m_impl, sField, value);
    }
    std::wstring object::GetText(const char *sField) const {
        std::string value_str;
        if (!_impl::get_field_string(m_impl, sField, value_str)) {
            return _T("");
        }
        const std::wstring value = ds_str_conv::ConvertFromUTF8(value_str.c_str());
        return value;
    }
    std::string object::GetTextUTF8(const char *sField) const
    {
        std::string value_str;
        _impl::get_field_string(m_impl, sField, value_str);
        return value_str;
    }
    void object::GetArray(const char *sField, json_array &array) const {
        _impl::get_field_array(m_impl, sField, array.m_impl);
    }
	void object::GetStringArray(const char *sField, std::vector<std::wstring> &array) const
	{
		json_array array_json;
		_impl::get_field_array(m_impl, sField, array_json.m_impl);
		const int nCnt = array_json.GetSize();
		for (int i = 0; i < nCnt; ++i) {
			array.push_back(array_json.GetString(i));
		}	
	}
    double object::GetDouble(const char *sField) const {
        double dValue; 
        if (!_impl::get_field_double(m_impl, sField, dValue)) {
            return 0.0;
        }
        return dValue;
    }
    int object::GetInteger(const char *sField) const {
        int nValue = 0;
        if (!_impl::get_field_int(m_impl, sField, nValue)) {
            return 0;
        }
        return nValue;
    }
    time_t object::GetDateTime(const char *sField) const {
        time_t nValue = 0;
        if (!_impl::get_field_date_time(m_impl, sField, nValue)) {
            return 0;
        }
        return nValue;
    }
    bool object::IsNull(const char *sField) const {
        return _impl::get_field_null(m_impl, sField);
    }
    void object::SetNull(const char *sField) {
        return _impl::set_field_null(m_impl, sField);
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
        _impl::add_array_object(m_impl, obj.m_impl);
    }
    void json_array::AddString(const char *str) {
        _impl::add_array_string(m_impl, str);
    }
    void json_array::AddString(const wchar_t *str) {
        std::string sValue = ds_str_conv::ConvertToUTF8(str);
        _impl::add_array_string(m_impl, sValue.c_str());
    }
	void json_array::AddInt(int nValue) {
		_impl::add_array_int(m_impl, nValue);
	}

    int json_array::GetSize() const {
        return _impl::get_array_size(m_impl);
    }
    std::string json_array::GetStringUTF8(int i) const {
        std::string sValue;
        _impl::get_array_string(m_impl, i, sValue);
        return sValue;
    }
    std::wstring json_array::GetString(int i) const {
        const std::string sValue = GetStringUTF8(i);
        return ds_str_conv::ConvertFromUTF8(sValue.c_str());
    }
	int json_array::GetInt(int i) const {
		return _impl::get_array_int(m_impl, i);
	}
    
    void json_array::GetJsonObject(int i, object &obj) const {
        _impl::get_array_object(m_impl, i, obj.m_impl);
    }
    
    void str2obj(const char* sJson, json_array &obj) {
        _impl::str2obj(sJson, obj.m_impl);
    }
    void obj2str(const json_array &obj, std::string &sJson) {
        _impl::obj2str(obj.m_impl, sJson);
    }
};
