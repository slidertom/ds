#ifndef __PICO_IMPL_H__
#define __PICO_IMPL_H__
#pragma once

#ifndef picojson_h
    #include "PicoJson/picojson.h"
#endif


namespace ds_jsonparser
{
    namespace _impl_pico
    {
        static inline void create(void *&impl) {
            ASSERT(!impl);
            impl = new picojson::value(picojson::object_type, true);
        }
        static inline void destroy(void *impl) {
            delete (picojson::value*) impl;
        }
        static inline void set_field(void *impl, const char* sField, const std::string &value_str) {
            picojson::value* pico_val = (picojson::value*) impl;
            ASSERT(pico_val->is<picojson::object>());
            picojson::value::object &pico_obj = pico_val->get<picojson::object>();
            pico_obj[sField] = picojson::value(value_str);
        }
        static inline bool get_field(void *impl, const char* sField, std::string &value_str) {
            picojson::value* pico_val = (picojson::value*) impl;
            ASSERT(pico_val->is<picojson::object>());
            if ( !pico_val->contains(sField) ) {
                value_str.clear();
                return false;
            }
            ASSERT(pico_val->get(sField).is<std::string>());
            value_str = pico_val->get(sField).get<std::string>();
            return true;
        }
        static inline void str2obj(const char* sJson, void *pImpl) {
            if ( !strlen(sJson) ) {
                return;
            }
            picojson::value *pico_value = (picojson::value *)pImpl;
            std::string err = picojson::parse(*pico_value, sJson, sJson + strlen(sJson));
            if ( err.length() ) {
                TRACE("PicoJson: PARSE ERROR: %hs\n", err.c_str());
            }
        }
        static inline void obj2str(const void *pImpl, std::string &sJson) {
            picojson::value *pico_value = (picojson::value*)pImpl;
            sJson = picojson::value(*pico_value).serialize();
        }
    };
}

#endif