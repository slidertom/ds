#ifndef __SIMD_IMPL_H__
#define __SIMD_IMPL_H__
#pragma once

#include <string>

namespace ds_json
{
    namespace _impl_simd
    {
        // current interface can be used for the any json backend implementation
        void str2obj(const char *sJson, void *pImpl);
        void str2obj(std::string &&sJson, void *pImpl);
        void str2array(const char *sJson, void *pImpl);
        void str2array(std::string &&sJson, void *pImpl);
        void obj2str(const void *pImpl, std::string &sJson);

        //////////////////////////////////////////////////////////
        // object based specific
        //////////////////////////////////////////////////////////
        void create(void *&impl);
        void destroy(void *impl);
        
        bool get_field_object(void *impl, const char *sField, void *obj);
        bool get_field_int32(void *impl, const char *sField, int32_t &nValue);
        bool get_field_int64(void *impl, const char *sField, int64_t &nValue);
        bool get_field_double(void *impl, const char *sField, double &dValue);
        bool get_field_string(void *impl, const char *sField, std::string &value_str);
        bool get_field_date_time(void *impl, const char *sField, time_t &nValue);
        bool get_field_array(void *impl, const char *sField, void *obj);
        bool get_field_null(void *impl, const char *sField);

        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        void create_array(void *&impl, size_t reserve_size);
        void destroy_array(void *&impl);
        
        size_t get_array_size(const void *impl);
        void get_array_string(const void *impl, size_t i, std::string &sValue);
        int32_t get_array_int32(const void *impl, size_t i);
        int64_t get_array_int64(const void *impl, size_t i);
        double get_array_double(const void *impl, size_t i);
        void get_array_object(const void *impl, size_t i, void *obj);
    };
};

#endif