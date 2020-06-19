#include "stdafx.h"
#include "rapid_impl.h"

#include "../dsStrConv.h"

#ifdef max
    #undef max
#endif

#ifdef min
    #undef min
#endif

#ifdef __x86_64__ 
    #define RAPIDJSON_NO_SIZETYPEDEFINE
    namespace rapidjson { typedef ::std::size_t SizeType; }
#endif

#ifndef RAPIDJSON_DOCUMENT_H_
    #include "Rapidjson/document.h"
    #include "Rapidjson/stringbuffer.h"
    #include "Rapidjson/writer.h"
#endif

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace ds_json
{
    namespace _impl_rapid
    {
        namespace internal
        {
            static inline void value2str(const rapidjson::Value &value, std::string &value_str)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                value.Accept(writer);
                value_str = buffer.GetString();
            }

            static inline void str2obj(const char *sJson, rapidjson::Document *doc) {
                if ( !::strlen(sJson) ) {
                    return;
                }
                doc->Parse<rapidjson::kParseStopWhenDoneFlag>(sJson);
                if ( !doc->IsObject() && !doc->IsArray() ) {
                    TRACE(L"RapidJson: PARSE ERROR\n");
                    return;
                }
            }

            static inline void obj2str(const rapidjson::Document *doc, std::string &sJson) {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc->Accept(writer);
                sJson = buffer.GetString();
            }
        };

        void str2obj(const char *sJson, void *pImpl) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)pImpl;
            internal::str2obj(sJson, doc);
        }

        void obj2str(const void *pImpl, std::string &sJson)
        {
            const rapidjson::Document *doc = (const rapidjson::Document *)pImpl;
            internal::obj2str(doc, sJson);
        }

        //////////////////////////////////////////////////////////
        // object based specific
        //////////////////////////////////////////////////////////
        void create(void *&impl) 
        {
            ASSERT(!impl);
            impl = new rapidjson::Document;
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            doc->SetObject();
        }

        void destroy(void *impl) 
        {
            delete (rapidjson::Document *)impl;
        }

        void set_field_string(void *impl, const char *sField, const char *value_str) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();

            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value;
                value.SetString(value_str, strlen(value_str), allocator);
                doc->AddMember(rapidjson::StringRef(sField), value, allocator);
            }
            else {
                const auto nLen = ::strlen(value_str);
                //if (nLen == 0) {
                // currently we can not do this: as it can break 3'rd party applications:
                // MatrixProject and WebKozijn (php, js layers) -> both applications don't check does value exists. 
                //    doc->EraseMember(found);
                //}
                //else {
                    rapidjson::Value &value = found->value;
                    value.SetString(value_str, nLen, allocator);
                //}
            }
        }

        void set_field_date_time(void *impl, const char *sField, time_t nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value;
                value.SetInt64(nValue);
                doc->AddMember(rapidjson::StringRef(sField), value, doc->GetAllocator());
            }
            else {
                rapidjson::Value &value = found->value;
                value.SetInt64(nValue);
            }
        }

        void set_field_int64(void *impl, const char *sField, const int64_t nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value;
                value.SetInt64(nValue);
                doc->AddMember(rapidjson::StringRef(sField), value, doc->GetAllocator());
            }
            else {
                rapidjson::Value &value = found->value;
                value.SetInt64(nValue);
            }
        }

        void set_field_int32(void *impl, const char *sField, const int32_t nValue) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value;
                value.SetInt(nValue);
                doc->AddMember(rapidjson::StringRef(sField), value, doc->GetAllocator());
            }
            else {
                rapidjson::Value &value = found->value;
                value.SetInt(nValue);
            }
        }

        void set_field_double(void *impl, const char *sField, const double dValue) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value;
                value.SetDouble(dValue);
                doc->AddMember(rapidjson::StringRef(sField), value, doc->GetAllocator());
            }
            else {
                rapidjson::Value &value = found->value;
                value.SetDouble(dValue);
            }
        }

        void set_field_object(void *impl, const char *sField, void *obj) 
        {
            // Alternative:
            //std::string value_str;
            //obj2str(obj, value_str);
            //set_field_string(impl, sField, value_str.c_str());

            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Document *doc_obj = (rapidjson::Document *)obj;

            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                rapidjson::Value value(doc_obj->GetType());
                value.CopyFrom(*doc_obj, allocator);
                doc->AddMember(rapidjson::StringRef(sField), value, allocator);
            }
            else {
                rapidjson::Value &value = found->value;
                value.CopyFrom(*doc_obj, allocator);
                ASSERT(value.GetType() == doc_obj->GetType());
            }
        }

        bool get_field_object(void *impl, const char *sField, void *obj)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }

            const rapidjson::Value &value = found->value;
            if ( value.IsObject() ) {
                rapidjson::Document *doc_obj = (rapidjson::Document *)obj;
                doc_obj->CopyFrom(value, doc_obj->GetAllocator());
                return true;
            }
            else if ( value.IsString() ) {
                std::string sJson;
                get_field_string(impl, sField, sJson);
                str2obj(sJson.c_str(), obj);
                return true;
            }
            return false;
        }

        void set_field_array(void *impl, const char *sField, void *obj) 
        {
            // Alternative:
            //std::string value_str;
            //obj2str(obj, value_str);
            //set_field_string(impl, sField, value_str.c_str());
            set_field_object(impl, sField, obj);
        }

        bool get_field_array(void *impl, const char *sField, void *obj)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }

            const rapidjson::Value &value = found->value;
            if ( value.IsArray() ) {
                rapidjson::Document *doc_obj = (rapidjson::Document *)obj;
                doc_obj->CopyFrom(value, doc_obj->GetAllocator());
                return true;
            }
            else if ( value.IsString() ) {
                std::string sJson;
                get_field_string(impl, sField, sJson);
                str2obj(sJson.c_str(), obj);
                return true;
            }
            return false;

            // Alternative:
            //std::string sJson;
            //get_field_string(impl, sField, sJson);
            //str2obj(sJson.c_str(), obj);
            //return true;
        }

        bool get_field_date_time(void *impl, const char *sField, time_t &nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }

            const rapidjson::Value &value = found->value;

            if ( value.IsInt64() ) {
                nValue = value.GetInt64();
                return true;
            }
            else if ( value.IsUint64() ) {
                nValue = value.GetInt64();
                return true;
            }
            else if ( value.IsInt() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsUint() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsString() ) {
                nValue = ds_str_conv::string_to_long(value.GetString()); // ~= atoi(value_str.c_str());
                return true;
            }
            
            ASSERT(FALSE);
            return false;
        }

        void set_field_null(void *impl, const char *sField)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return;
            }

            rapidjson::Value &value = found->value;
            value.SetNull();
        }

        bool get_field_null(void *impl, const char *sField)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return true;
            }

            const rapidjson::Value &value = found->value;
            if ( value.IsNull() ) {
                return true;
            }

            return false;
        }

        static inline bool rapid_value_to_int64(const rapidjson::Value &value, int64_t &nValue)
        {
            if ( value.IsInt() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsUint() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsInt64() ) {
                nValue = value.GetInt64();
                return true;
            }
            else if ( value.IsUint64() ) {
                nValue = value.GetInt64();
                return true;
            }
            else if ( value.IsString() ) {
                nValue = ds_str_conv::string_to_long(value.GetString()); // ~= atoi(value_str.c_str());
                return true;
            }
            else if ( value.IsNull() ) {
                nValue = 0;
                return true;
            }
            else if ( value.IsNumber() ) { // do catch all other numeric types: double, float
                nValue = (int32_t)value.GetDouble();
                return true;
            }

            ASSERT(FALSE);
            return false;
        }

        bool get_field_int64(void *impl, const char *sField, int64_t &nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }
            const rapidjson::Value &value = found->value;
            return rapid_value_to_int64(value, nValue);
        }

        static inline bool rapid_value_to_int32(const rapidjson::Value &value, int32_t &nValue)
        {
            if ( value.IsInt() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsUint() ) {
                nValue = value.GetInt();
                return true;
            }
            else if ( value.IsInt64() ) {
                nValue = (int32_t)value.GetInt64();
                return true;
            }
            else if ( value.IsUint64() ) {
                nValue = (int32_t)value.GetInt64();
                return true;
            }
            else if ( value.IsString() ) {
                nValue = ds_str_conv::string_to_long(value.GetString()); // ~= atoi(value_str.c_str());
                return true;
            }
            else if ( value.IsNull() ) {
                nValue = 0;
                return true;
            }
            else if ( value.IsNumber() ) { // do catch all other numeric types: double, float
                nValue = (int32_t)value.GetDouble();
                return true;
            }
            else if ( value.IsBool() ) {
                nValue = value.GetBool() == true ? 1 : 0;
                return true;
            }
            else if ( value.IsFalse() ) {
                nValue = 0;
                return true;
            }
            else if ( value.IsTrue() ) {
                nValue = 1;
                return true;
            }

            ASSERT(FALSE);
            return false;
        }

        bool get_field_int32(void *impl, const char *sField, int32_t &nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);

            if ( found == doc->MemberEnd() ) {
                return false;
            }
            const rapidjson::Value &value = found->value;
            return rapid_value_to_int32(value, nValue);
        }

        bool get_field_double(void *impl, const char *sField, double &dValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }

            const rapidjson::Value &value = found->value;
            
            if ( value.IsNumber() ) {
                dValue = value.GetDouble();
                return true;
            }
            else if ( value.IsString() ) {
                dValue = ds_str_conv::string_to_double(value.GetString()); 
                return true;
            }
            else if ( value.IsNull() ) {
                dValue = 0.0;
                return true;
            }
            // TODO IsObject -> to string and string_to_double also do check out IsLosslessDouble()

            ASSERT(FALSE);
            return false;
        }

        bool get_field_string(void *impl, const char *sField, std::string &value_str) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            rapidjson::Document::MemberIterator found = doc->FindMember(sField);
            if ( found == doc->MemberEnd() ) {
                return false;
            }

            const rapidjson::Value &value = found->value;    
            if ( value.IsString() ) {
                value_str = value.GetString();
                return true;
            }
            
            internal::value2str(value, value_str);
            return true;
        }

        bool remove_field(void *impl, const char *sField)
        {
             rapidjson::Document *doc = (rapidjson::Document *)impl;
             if (!doc->RemoveMember(sField)) {
                 return false;
             }

             return true;
        }
        
        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        void create_array(void *&impl) 
        {
            ASSERT(!impl);
            impl = new rapidjson::Document;
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            doc->SetArray();
        }

        void add_array_string(void *impl, const char *str) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetString(str, strlen(str), doc->GetAllocator());
            doc->PushBack(value, allocator);
        }

        void add_array_int64(void *impl, int64_t nValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetInt64(nValue);
            doc->PushBack(value, allocator);
        }

        void add_array_int32(void *impl, int32_t nValue) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetInt(nValue);
            doc->PushBack(value, allocator);
        }

        void add_array_double(void *impl, double dValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetDouble(dValue);
            doc->PushBack(value, allocator);
        }

        void add_array_float(void *impl, float fValue)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());

            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Value value;
            value.SetFloat(fValue);
            doc->PushBack(value, allocator);
        }
        /*
        void add_array_move_object(void *impl, const void *obj) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();
            rapidjson::Document *doc_obj = (rapidjson::Document *)obj;
            rapidjson::Value value(doc_obj->GetType());
            value.Swap(*doc_obj);
            doc->PushBack(value, allocator);
            //doc->PushBack(rapidjson::Document(std::move(*doc_obj)), allocator);
            ASSERT(doc_obj->IsNull());
            destroy(doc_obj);
        }
        */
        void add_array_object(void *impl, const void *obj) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();

            rapidjson::Document *doc_obj = (rapidjson::Document *)obj;
            rapidjson::Value value(doc_obj->GetType());
            value.CopyFrom(*doc_obj, allocator);
            doc->PushBack(value, allocator);
            ASSERT(value.IsNull());
        }

        size_t get_array_size(const void *impl) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());
            return doc->Size();
        }
        
        void get_array_string(const void *impl, size_t i, std::string &sValue) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            const rapidjson::Value &value = (*doc)[i];
            if ( value.IsString() ) {
                sValue = value.GetString();
                return;
            }
            internal::value2str(value, sValue);
        }

        int32_t get_array_int32(const void *impl, size_t i)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            const rapidjson::Value &value = (*doc)[i];
            int32_t nValue = 0;
            if ( rapid_value_to_int32(value, nValue) ) {
                return nValue;
            }
            return 0;
        }

        int64_t get_array_int64(const void *impl, size_t i)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            const rapidjson::Value &value = (*doc)[i];
            int64_t nValue = 0;
            if ( rapid_value_to_int64(value, nValue) ) {
                return nValue;
            }
            return 0;
        }

        void set_array_object(void *impl, size_t i, const void *obj)
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;
            ASSERT(doc->IsArray());
            rapidjson::Document::AllocatorType &allocator = doc->GetAllocator();

            rapidjson::Value &value = (*doc)[i];    
            rapidjson::Document *doc_obj = (rapidjson::Document *)obj;
            value.CopyFrom(*doc_obj, allocator);
        }
        
        void get_array_object(const void *impl, size_t i, void *obj) 
        {
            rapidjson::Document *doc = (rapidjson::Document *)impl;

            const rapidjson::Value &value = (*doc)[i];           
            if ( value.IsString() ) {
                str2obj(value.GetString(), obj);
            }
            else if ( value.IsObject() ) 
            {
                rapidjson::Document *obj_doc = (rapidjson::Document *)obj;
                obj_doc->CopyFrom(value, obj_doc->GetAllocator());
            }
            else if ( value.IsArray() ) {
                if ( value.Size() == 0 ) { // special case to support "[]", otherwise rapid asserts in the debug mode
                    return;// nothing to fill
                }
                else {
                    std::string sJson;
                    internal::value2str(value, sJson);
                    str2obj(sJson.c_str(), obj);       
                }
            }
            else if ( value.IsNull() ) {
                return;// nothing to fill
            }
            else {
                std::string sJson;
                internal::value2str(value, sJson);
                str2obj(sJson.c_str(), obj);       
            }
        }
    }
};
