#ifndef __SA_IMPL_H__
#define __SA_IMPL_H__
#pragma once

#include "sajson/sajson.h"

namespace ds_jsonparser
{
    namespace _impl_sa
    {
        static inline void create(void *&impl) {
            ASSERT(!impl);
            //impl = new sajson::Document;
        }
        static inline void destroy(void *impl) {
            //delete (sajson::Document*) impl;
        }
        static inline void set_field(void *impl, const char* sField, const std::string &value_str) {
            //Not implemented
        }
        static inline bool get_field(void *impl, const char* sField, std::string &value_str) {
            //sajson::Document *doc = (sajson::Document*) impl;
            //sajson::Value &value = (*doc)[sField];  value;
            //value_str = value.GetString();
            return true;
        }
        static inline void str2obj(const char* sJson, object &obj) {
            if ( !strlen(sJson) ) {
                return;
            }
            //sajson::Document *doc = (sajson::Document*) obj.m_impl;
            //if ( !doc ) {
            //    TRACE("saJson: PARSE ERROR\n");
            //    return;
            //}
            //doc->Parse(sJson);
            
            //sajson::document *doc = (sajson::document*) obj.m_impl;
            //sajson::document doc = sajson::parse(sJson);
            //obj.m_impl = (void*) &doc;
        }
        static inline void obj2str(const object &obj, std::string &sJson) {
            //Not implemented
        }
    }
}

#endif