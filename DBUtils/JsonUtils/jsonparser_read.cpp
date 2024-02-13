#include "stdafx.h"
#include "jsonparser.h"

#include "../dsStrConv.h"

//#define RAPID_READ  // Use RapidJson
#define SIMD_READ

#if defined(RAPID_READ)
    #include "rapid_impl.h"
    #define _impl_read _impl_rapid
#elif defined(SIMD_READ)
    #include "simd_impl.h"
    #define _impl_read _impl_simd
#elif defined(YY_READ)
    #include "yy_impl.h"
    #define _impl_read _impl_yy
#endif

//  test name
// articles load - multi threaded
// rapid: 0.033 0.034 0.034 0.04  0.033 0.033 0.037 0.035 0.032
// simd:  0.024 0.025 0.024 0.025 0.024 0.026 0.024 0.023 0.022  
//
// LH load (single threaded)
// rapid: 0.011
// simd:  0.022

// generic (multi-threaded)
// rapid: 0.31
// simd:  0.12

// rapid in the faster solution as a single threaded 

namespace ds_json
{
    namespace read
    {
        object::object() {
            _impl_read::create(m_impl);
        }
        object::~object() {
            _impl_read::destroy(m_impl);
        }

        std::wstring object::GetText(const char *sField) const noexcept {
            std::string value_str;
            if (!_impl_read::get_field_string(m_impl, sField, value_str)) {
                return L"";
            }
            const std::wstring value = ds_str_conv::ConvertFromUTF8(value_str.c_str());
            return value;
        }
        std::string object::GetTextUTF8(const char *sField) const noexcept 
        {
            std::string value_str;
            _impl_read::get_field_string(m_impl, sField, value_str);
            return value_str;
        }
        void object::GetArray(const char *sField, array &array) const noexcept {
            _impl_read::get_field_array(m_impl, sField, array.m_impl);
        }
        void object::GetStringArray(const char *sField, std::vector<std::wstring> &array) const noexcept 
        {
            ds_json::read::array array_json;
            if ( !_impl_read::get_field_array(m_impl, sField, array_json.m_impl) ) {
                return;
            }
            
            const size_t nCnt = array_json.size();
            array.reserve(nCnt);
            for (size_t i = 0; i < nCnt; ++i) {
                array.push_back(array_json.GetString(i));
            }    
        }
        void object::GetStringArrayUTF8(const char *sField, std::vector<std::string> &array) const noexcept
        {
            ds_json::read::array array_json;
            if ( !_impl_read::get_field_array(m_impl, sField, array_json.m_impl) ) {
                return;
            }

            const size_t nCnt = array_json.size();
            array.reserve(nCnt);
            for (size_t i = 0; i < nCnt; ++i) {
                array.push_back(array_json.GetStringUTF8(i));
            }   
        }
        void object::GetInt32Array(const char *sField, std::vector<int32_t> &array) const noexcept
        {
            ds_json::read::array array_json;
            if ( !_impl_read::get_field_array(m_impl, sField, array_json.m_impl) ) {
                return;
            }

            const size_t nCnt = array_json.size();
            array.reserve(nCnt);
            for (size_t i = 0; i < nCnt; ++i) {
                array.push_back(array_json.GetInt32(i));
            }
        }

        double object::GetDouble(const char *sField) const noexcept {
            double dValue; 
            if (!_impl_read::get_field_double(m_impl, sField, dValue)) {
                return 0.0;
            }
            return dValue;
        }
        int32_t object::GetInt32(const char *sField) const noexcept {
            int32_t nValue = 0;
            if (!_impl_read::get_field_int32(m_impl, sField, nValue)) {
                return 0;
            }
            return nValue;
        }
        int64_t object::GetInt64(const char *sField) const noexcept {
            int64_t nValue = 0;
            if (!_impl_read::get_field_int64(m_impl, sField, nValue)) {
                return 0;
            }
            return nValue;
        }

        bool object::GetJsonObject(const char *sField, object &obj) const noexcept {
            return _impl_read::get_field_object(m_impl, sField, obj.m_impl);
        }
        time_t object::GetDateTime(const char *sField) const noexcept {
            time_t nValue = 0;
            if (!_impl_read::get_field_date_time(m_impl, sField, nValue)) {
                return 0;
            }
            return nValue;
        }
        bool object::IsNull(const char *sField) const noexcept {
            return _impl_read::get_field_null(m_impl, sField);
        }
        
        array::array() {
            _impl_read::create_array(m_impl, 0);
        }

        array::array(size_t reserve_size) {
		    _impl_read::create_array(m_impl, reserve_size);
        }

        array::array(array &&ar) {
            m_impl = ar.m_impl;
            ar.m_impl = nullptr;
        }
        array::~array() {
            _impl_read::destroy_array(m_impl);
        }
        array &array::operator=(array &&ar) {
            _impl_read::destroy_array(m_impl);
            m_impl = ar.m_impl;
            ar.m_impl = nullptr;
            return *this;
        }
       
        size_t array::size() const noexcept {
            return _impl_read::get_array_size(m_impl);
        }

        std::string array::GetStringUTF8(size_t i) const noexcept {
            std::string sValue;
            _impl_read::get_array_string(m_impl, i, sValue);
            return sValue;
        }
        std::wstring array::GetString(size_t i) const noexcept {
            const std::string sValue = GetStringUTF8(i);
            return ds_str_conv::ConvertFromUTF8(sValue.c_str());
        }
        int64_t array::GetInt64(size_t i) const noexcept {
            return _impl_read::get_array_int64(m_impl, i);
        }
        int32_t array::GetInt32(size_t i) const noexcept {
            return _impl_read::get_array_int32(m_impl, i);
        }
        double array::GetDouble(size_t i) const noexcept {
            return _impl_read::get_array_double(m_impl, i);
        }
        // array iterator **********************************************
        array::iterator::iterator(const array &arr, const size_t nPos) : m_array_object(arr, nPos) { }
        array::iterator::iterator(const iterator &x) : m_array_object(x.m_array_object) { }
        array::iterator &array::iterator::operator++()   { ++m_array_object.m_nPos; return *this; } // prefix++
        array::iterator array::iterator::operator++(int) { ++m_array_object.m_nPos; return iterator(*this); } // postfix++ 
        bool array::iterator::operator!=(iterator &rhs) const { 
            return m_array_object.m_nPos != rhs.m_array_object.m_nPos; 
        }

        array::iterator::reference array::iterator::operator*() const  { 
            m_array_object.m_pArr->GetJsonObject(m_array_object.m_nPos, m_array_object.m_object);
            return m_array_object;
        }

        array::iterator::pointer array::iterator::operator->() const { 
            return &(operator*()); 
        }
        // ********************************************************************************************

        void array::GetJsonObject(size_t i, object &obj) const noexcept {
            //_impl_read::destroy(obj.m_impl);
            //obj.m_impl = nullptr;
            //_impl_read::create(obj.m_impl);
            _impl_read::get_array_object(m_impl, i, obj.m_impl);
        }

        void array::GetArray(size_t i, array &array) const noexcept {
            _impl_read::get_array_object(m_impl, i, array.m_impl);
        }

        void str2obj(const char *sJson, std::vector<int32_t> &v) noexcept 
        {
            ds_json::read::array arr;
            ds_json::str2obj(sJson, arr);
            size_t nSize = arr.size();
            v.reserve(nSize);
            for (size_t i1 = 0; i1 < nSize; ++i1) {
                v.push_back(arr.GetInt32(i1));
            }
        }

        void str2obj(const char *sJson, std::vector<std::vector<int32_t>> &v) noexcept 
        {
            ds_json::read::array arr;
            ds_json::str2obj(sJson, arr);
            size_t nSize = arr.size();
            v.resize(nSize);
            for (size_t i1 = 0; i1 < nSize; ++i1) {
                std::string sJson2 = arr.GetStringUTF8(i1);
                str2obj(sJson2.c_str(), v[i1]);
            }
        }

        void str2obj(const char *sJson, std::unordered_set<std::string> &v) noexcept 
        {
            ds_json::read::array arr;
            ds_json::str2obj(sJson, arr);
            size_t nSize = arr.size();
            v.reserve(nSize);
            for (size_t i1 = 0; i1 < nSize; ++i1) {
                v.emplace(arr.GetStringUTF8(i1));
            }
        }

        void str2obj(const char *sJson, std::vector<std::string> &v) noexcept 
        {
            ds_json::read::array arr;
            ds_json::str2obj(sJson, arr);
            size_t nSize = arr.size();
            v.reserve(nSize);
            for (size_t i1 = 0; i1 < nSize; ++i1) {
                v.push_back(arr.GetStringUTF8(i1));
            }
        }

        array::array_object::operator int32_t() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetInt32(m_nPos);
        }
        array::array_object::operator int64_t() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetInt64(m_nPos);
        }
        array::array_object::operator reference() const noexcept {
            return m_object;
        }
        array::array_object::operator bool() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetInt32(m_nPos) != 0;
        }
        array::array_object::operator double() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetDouble(m_nPos) != 0;
        }
        array::array_object::operator std::string() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetStringUTF8(m_nPos);
        }
        array::array_object::operator std::wstring() const noexcept {
            ASSERT(FALSE); // please check -> do you get the correct output -> call count should be same as json array count
            return m_pArr->GetString(m_nPos);
        }
        /*
        array::array_object::operator ds_json::read::array() const noexcept {
            ds_json::read::array arr;
            arr.m_impl = m_object.m_impl; // explicit conversion object into array
            m_object.m_impl = nullptr;
            return arr;
        }
        */
    };

    // object
    void str2obj(const char *sJson, read::object &obj) noexcept {
        _impl_read::str2obj(sJson, obj.m_impl);
    }
    void str2obj(std::string &&sJson, read::object &obj) noexcept {
        _impl_read::str2obj(std::move(sJson), obj.m_impl);
    }
    // TODO: remove this function
    void str2obj(const wchar_t *sJson, read::object &obj) noexcept {
        std::string json_utf8 = ds_str_conv::ConvertToUTF8(sJson);
        _impl_read::str2obj(std::move(json_utf8), obj.m_impl);
    }

    // array
    void str2obj(const char *sJson, read::array &obj) noexcept {
        _impl_read::str2array(sJson, obj.m_impl);
    }
    void str2obj(std::string &&sJson, read::array &obj) noexcept {
        _impl_read::str2array(std::move(sJson), obj.m_impl);
    }
};