#ifndef __RAPID_IMPL_H__
#define __RAPID_IMPL_H__
#pragma once

#include "string"

namespace ds_json
{
    namespace _impl_rapid
    {
        // current interface can be used for the any json backend implementation

        void str2obj(const char *sJson, void *pImpl);
        void obj2str(const void *pImpl, std::string &sJson);

        //////////////////////////////////////////////////////////
        // object based specific
        //////////////////////////////////////////////////////////
        void create(void *&impl);
        void destroy(void *impl);
        void set_field_string(void *impl, const char *sField, const char *value_str);
        void set_field_int32(void *impl, const char *sField, const int32_t nValue);
        void set_field_int64(void *impl, const char *sField, const int64_t nValue);
        void set_field_double(void *impl, const char *sField, const double dValue);
        void set_field_object(void *impl, const char *sField, void *obj);
        void set_field_date_time(void *impl, const char *sField, time_t nValue);
        void set_field_array(void *impl, const char *sField, void *obj);
        void set_field_null(void *impl, const char *sField);
        bool get_field_object(void *impl, const char *sField, void *obj);
        bool get_field_int32(void *impl, const char *sField, int32_t &nValue);
        bool get_field_int64(void *impl, const char *sField, int64_t &nValue);
        bool get_field_double(void *impl, const char *sField, double &dValue);
        bool get_field_string(void *impl, const char *sField, std::string &value_str);
        bool get_field_date_time(void *impl, const char *sField, time_t &nValue);
        bool get_field_array(void *impl, const char *sField, void *obj);
        bool get_field_null(void *impl, const char *sField);

        bool remove_field(void *impl, const char *sField);

        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        void create_array(void *&impl);
        void add_array_string(void *impl, const char *str);
        void add_array_int32(void *impl, int32_t nValue);
        void add_array_int64(void *impl, int64_t nValue);
        void add_array_double(void *impl, double dValue);
        void add_array_float(void *impl, float fValue);
        void add_array_object(void *impl, const void *obj);
        //void add_array_move_object(void *impl, const void *obj);
        size_t get_array_size(const void *impl);
        void get_array_string(const void *impl, size_t i, std::string &sValue);
        int32_t get_array_int32(const void *impl, size_t i);
        int64_t get_array_int64(const void *impl, size_t i);
        double get_array_double(const void *impl, size_t i);
        void get_array_object(const void *impl, size_t i, void *obj);
        void set_array_object(void *impl, size_t i, const void *obj);
    }
}

#endif