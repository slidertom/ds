#ifndef __RAPID_IMPL_H__
#define __RAPID_IMPL_H__
#pragma once

namespace ds_jsonparser
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
        void set_field_int(void *impl, const char *sField, const int nValue);
        void set_field_double(void *impl, const char *sField, const double dValue);
        void set_field_object(void *impl, const char *sField, void *obj);
        void set_field_date_time(void *impl, const char *sField, time_t nValue);
        void set_field_array(void *impl, const char *sField, void *obj);
        bool get_field_int(void *impl, const char *sField, int &nValue);
        bool get_field_double(void *impl, const char *sField, double &dValue);
        bool get_field_string(void *impl, const char *sField, std::string &value_str);
        bool get_field_date_time(void *impl, const char *sField, time_t &nValue);
        bool get_field_array(void *impl, const char *sField, void *obj);

        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        void create_array(void *&impl);
        void add_array_string(void *impl, const char *str);
        void add_array_object(void *impl, const void *obj);
        int  get_array_size(const void *impl);
        void get_array_string(const void *impl, int i, std::string &sValue);
        void get_array_object(const void *impl, int i, void *obj);
    }
}

#endif