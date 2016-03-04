#ifndef __RAPID_IMPL_H__
#define __RAPID_IMPL_H__
#pragma once

#ifndef __DS_STR_CONV_H__
    #include "dsStrConv.h"
#endif

#ifndef RAPIDJSON_DOCUMENT_H_
    #include "Rapidjson/document.h"
    #include "Rapidjson/stringbuffer.h"
    #include "Rapidjson/writer.h"
#endif

namespace ds_jsonparser
{
    namespace _impl_rapid
    {
        static inline void create(void *&impl) {
            ASSERT(!impl);
            impl = new rapidjson::Document;

            rapidjson::Document *doc = (rapidjson::Document *)impl;
            doc->SetObject();
        }
        static inline void destroy(void *impl) {
            delete (rapidjson::Document *)impl;
        }
        static inline void set_field(void *impl, const char *sField, const std::string &value_str) {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            if ( !doc->HasMember(sField) )
            {
                rapidjson::Value value;
                value.SetString(value_str.c_str(), value_str.size(), doc->GetAllocator());
                doc->AddMember(rapidjson::StringRef(sField), value, doc->GetAllocator());
            }
            else
            {
                rapidjson::Value &value = (*doc)[sField];
                value.SetString(value_str.c_str(), value_str.size(), doc->GetAllocator());
            }
        }

        static inline bool get_field_int(void *impl, const char *sField, int &nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            if ( !doc->HasMember(sField) ) {
                return false;
            }
            rapidjson::Value &value = (*doc)[sField];

            if ( value.IsString() ) {
                nValue = ds_str_conv::string_to_long(value.GetString()); // ~= atoi(value_str.c_str());
                return true;
            }
            else if ( value.IsInt() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsInt64() ) {
                nValue = value.GetInt64();
                return true;
            }

            ASSERT(FALSE);
            return false;
        }

        static inline bool get_field(void *impl, const char *sField, std::string &value_str) {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            if ( !doc->HasMember(sField) ) {
                return false;
            }
            rapidjson::Value &value = (*doc)[sField];
            ASSERT(value.IsString());
            value_str = value.GetString();
            return true;
        }
        static inline void str2obj(const char *sJson, void *pImpl) {
            if ( !::strlen(sJson) ) {
                return;
            }
            rapidjson::Document *doc = (rapidjson::Document *)pImpl;
            doc->Parse<rapidjson::kParseStopWhenDoneFlag>(sJson);
            if ( !doc->IsObject() && !doc->IsArray() ) {
                TRACE("RapidJson: PARSE ERROR\n");
                return;
            }
        }
        static inline void obj2str(const void *pImpl, std::string &sJson) {
            rapidjson::Document *doc = (rapidjson::Document *)pImpl;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc->Accept(writer);
            sJson = buffer.GetString();
        }
        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        namespace internal
        {
            // https://code.google.com/p/rapidjson/issues/detail?id=85
            template <class Allocator>
            static inline rapidjson::Value *document_to_value(rapidjson::Document &document, Allocator &alloc) 
            {
                rapidjson::Value *rtn = new rapidjson::Value();
                if ( document.IsObject() ) {
                    rtn->SetObject();
                    auto end_it = document.MemberEnd();
                    for (auto itr = document.MemberBegin(); itr != end_it; ++itr)  {
                        rtn->AddMember(itr->name, itr->value, alloc);
                    }
                    return rtn;
                } 
                else if (document.IsArray()) {
                    rtn->SetArray();
                    auto sz = document.Size();
                    for (int64_t i = 0; i < sz; ++i) {
                        rtn->PushBack(document[i], alloc);
                    }
                    return rtn;
                } 
                else {
                    delete rtn;
                    return nullptr;
                }
            }
        };

        static inline void create_array(void *&impl) {
            ASSERT(!impl);
            impl = new rapidjson::Document;

            rapidjson::Document *doc = (rapidjson::Document *)impl;
            doc->SetArray();
        }
        static inline void add_array_string(void *impl, const char *str) {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            // must pass an allocator when the object may need to allocate memory
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetString(str, strlen(str), doc->GetAllocator());
            doc->PushBack(value, allocator);
        }
        static inline int get_array_size(const void *impl) {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());
            return doc->Size();
        }
        static inline std::string get_value(const void *impl, int i) {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            return (*doc)[i].GetString();
        }
    }
}

#endif