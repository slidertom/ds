#include "stdafx.h"
#include "simd_impl.h"

#include "simdjson/simdjson.h"
#include "../dsStrConv.h"

#ifdef max
    #undef max
#endif

#ifdef min
    #undef min
#endif

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

namespace ds_json
{
    namespace _impl_simd
    {
        class simd_type
        {
        // Construction
        public:
            simd_type() { }

        // Attributes
        public:
            // https://github.com/simdjson/simdjson/blob/master/doc/basics.md
            // For code safety, you should keep (1) the parser instance, (2) the input string and (3) the document instance alive throughout your parsing. 
            simdjson::ondemand::parser m_parser; // (1)
            std::string m_json; // (2) looks like it's not required
            simdjson::simdjson_result<simdjson::ondemand::document> m_doc; // (3)
            // Additionally, you should follow the following rules:
            // * A parser may have at most one document open at a time, since it holds allocated memory used for the parsing.
            // * By design, you should only have one document instance per JSON document. 
            // Thus, if you must pass a document instance to a function, you should avoid passing it by value: 
            // choose to pass it by reference instance to avoid the copy. (We also provide a document_reference 
            // class if you need to pass by value.)
        };

        class simd_wrapper : public simd_type
        {
        public:
            simd_wrapper() { }

            
            simdjson::ondemand::object m_obj;
            bool m_object_mode {false}; // hack
            bool m_failed {false}; // hack TEMPORAL

            simdjson::simdjson_result<simdjson::ondemand::value> operator[](const char *key) & noexcept 
            {
                if (m_object_mode) 
                {
                    return m_obj[key];
                }

                if (m_failed) {
                    // TODO: do add assert -> it musn't be such situation -> do return before!
                    return simdjson::NO_SUCH_FIELD;
                }

                return m_doc[key];
            }
        };

        class simd_arr_wrapper : public simd_type
        {
        public:
            simd_arr_wrapper() { }

            bool defined() {
                return m_it.error() ? false : true;
            }
            bool m_bFirst {true};
            void Iterate()
            {
                if ( m_bFirst ) {
                    m_bFirst = false;
                    return;
                }
                ++m_it;
            }
            
            simdjson::ondemand::array m_arr;
            simdjson::simdjson_result<simdjson::ondemand::array_iterator> m_it;
        };

        // Or by creating a padded string (for efficiency reasons, simdjson requires a string with SIMDJSON_PADDING bytes at the end) and calling iterate():
        // {c++}
        // ondemand::parser parser;
        // auto json = "[1,2,3]"_padded; // The _padded suffix creates a simdjson::padded_string instance
        // ondemand::document doc = parser.iterate(json); // parse a string

        // If you have a buffer of your own with enough padding already (SIMDJSON_PADDING extra bytes allocated), you can use padded_string_view to pass it in:
        // {c++}
        // ondemand::parser parser;
        // char json[3+SIMDJSON_PADDING];
        // strcpy(json, "[1]");
        // ondemand::document doc = parser.iterate(json, strlen(json), sizeof(json));

        // We recommend against creating many std::string or many std::padding_string instances in your application to store your JSON data. Consider reusing the same buffers and limiting memory allocations.

        // The simdjson library will also accept `std::string` instances, as long as the `capacity()` of
        // the string exceeds the `size()` by at least `SIMDJSON_PADDING`. You can increase the `capacity()` with the `reserve()` function of your strings.

        void str2obj(std::string &&sJson, void *pImpl)
        {
            simd_wrapper *doc = (simd_wrapper *)pImpl;
            if (sJson.empty()) {
                doc->m_json = "{}"_padded;
            }
            else {
                doc->m_json = std::move(sJson); 
                doc->m_json.reserve(doc->m_json.length() + simdjson::SIMDJSON_PADDING);
            }

            const size_t len = doc->m_json.length();
            doc->m_doc = doc->m_parser.iterate(doc->m_json.c_str(), len, len + simdjson::SIMDJSON_PADDING);
        }

        void str2obj(const char *sJson, void *pImpl) 
        {
            simd_wrapper *doc = (simd_wrapper *)pImpl;
            const size_t len = ::strlen(sJson);
            if ( (len == 0) ) {
                doc->m_json = "{}"_padded;
            }
            else {
                doc->m_json = sJson; // required copy as it's given temporal string
                doc->m_json.reserve(doc->m_json.length() + simdjson::SIMDJSON_PADDING);
            }

            doc->m_doc = doc->m_parser.iterate(doc->m_json.c_str(), len, len + simdjson::SIMDJSON_PADDING);
        }

        void str2array(const char *sJson, void *pImpl)
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)pImpl;
            const size_t len = ::strlen(sJson);
            if ( (len == 0) ) {
                doc->m_json = "[]"_padded;
            }
            else {
                doc->m_json = sJson;  // required copy as it's given temporal string
                doc->m_json.reserve(doc->m_json.length() + simdjson::SIMDJSON_PADDING);
            }
            
            doc->m_doc = doc->m_parser.iterate(doc->m_json.c_str(), len, len + simdjson::SIMDJSON_PADDING);
            doc->m_arr = doc->m_doc.get_array();
            doc->m_it  = doc->m_arr.begin();
        }

        void str2array(std::string &&sJson, void *pImpl)
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)pImpl;
            if (sJson.empty()) {
                doc->m_json = "[]"_padded;
            }
            else {
                doc->m_json = std::move(sJson); 
                doc->m_json.reserve(doc->m_json.length() + simdjson::SIMDJSON_PADDING);
            }
            
            const size_t len = doc->m_json.length();
            doc->m_doc = doc->m_parser.iterate(doc->m_json.c_str(), len, len + simdjson::SIMDJSON_PADDING);
            doc->m_arr = doc->m_doc.get_array();
            doc->m_it  = doc->m_arr.begin();
        }

        void obj2str(const void *pImpl, std::string &sJson) 
        {
            simd_arr_wrapper *doc = dynamic_cast<simd_arr_wrapper *>((simd_arr_wrapper *)pImpl);
            if (doc && doc->defined()) {
                std::string_view view = simdjson::to_json_string(doc->m_arr);
                sJson = view;
                return;
            }
         //   ASSERT(FALSE);
        }

        //////////////////////////////////////////////////////////
        // object based specific
        //////////////////////////////////////////////////////////
        void create(void *&impl) 
        {
            ASSERT(!impl);
            impl = new simd_wrapper;
        }

        void destroy(void *impl) 
        {
            delete (simd_type *)impl;
        }

        template <class TValue>
        static bool simd_value_to_object(TValue &value, void *obj)
        {
            switch (value.type())
            {
            case simdjson::ondemand::json_type::object: {
                    simd_wrapper *doc_obj = (simd_wrapper *)obj;
                    auto error = value.get_object().get(doc_obj->m_obj);
                    doc_obj->m_object_mode = true;
                    if (error) {
                        doc_obj->m_failed = true;
                        return false;
                    }
                    return true;
                }
            break;
            }

            return false;
        }

        bool get_field_object(void *impl, const char *sField, void *obj)
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error()) {
                return false;
            }

            return simd_value_to_object(value, obj);
        }

        template <class TValue>
        static bool simd_value_to_array(TValue &value, void *obj)
        {
            switch (value.type())
            {
            case simdjson::ondemand::json_type::array: {
                simd_arr_wrapper *doc_obj = (simd_arr_wrapper *)obj;
                doc_obj->m_arr = value.get_array();
                doc_obj->m_it  = doc_obj->m_arr.begin();
                return true;
            }
            break;
            }
            ASSERT(FALSE);
            return false;
        }

        bool get_field_array(void *impl, const char *sField, void *obj)
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error()) {
                return false;
            }

            switch (value.type())
            {
            case simdjson::ondemand::json_type::array: {
                simd_arr_wrapper *doc_obj = (simd_arr_wrapper *)obj;
                doc_obj->m_arr = value.get_array();
                doc_obj->m_it  = doc_obj->m_arr.begin();
                return true;
            }
            break;
            case simdjson::ondemand::json_type::string: {
                std::string_view view;
                value.get(view);
                std::string value_str;
                value_str = view;
                str2array(std::move(value_str), obj);
                return true;
            }
            break;
            }
            ASSERT(FALSE);
            return false;
        }

        bool get_field_date_time(void *impl, const char *sField, time_t &nValue)
        {
            // TODO specific impl for 32 bits?
            if ( !get_field_int64(impl, sField, nValue) ) {
                return false;
            }
            return true;
        }
        
        bool get_field_null(void *impl, const char *sField)
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error()) {
                return true;
            }

            switch (value.type())
            {
            case simdjson::ondemand::json_type::null:
                return true;
            break;
            }

            return false;
        }

        template <class TValue>
        static bool simd_value_to_int64(TValue &value, int64_t &nValue)
        {
            switch (value.type())
            {
            case simdjson::ondemand::json_type::array:
                return false;
            break;
            case simdjson::ondemand::json_type::object:
                return false;
            break;
            case simdjson::ondemand::json_type::number:
            {
                simdjson::ondemand::number_type t = value.get_number_type();
                switch (t) 
                {
                    case simdjson::ondemand::number_type::signed_integer:
                    case simdjson::ondemand::number_type::unsigned_integer:
                    {
                        nValue = value.get_int64();
                        return true;
                    }
                    break;
                    case simdjson::ondemand::number_type::floating_point_number:
                    {
                        nValue = (int64_t)value.get_double();
                        return true;
                    }
                    break;
                }
                
                return false;
            }
            break;
            case simdjson::ondemand::json_type::string:
                value.get_int64_in_string().get(nValue);
                return true;
            break;
            case simdjson::ondemand::json_type::boolean:
                nValue = (value.get_bool() == true) ? 1 : 0;
                return true;
            break;
            case simdjson::ondemand::json_type::null:
                nValue = 0;
                return true;
            break;
            }

            return false;
        }

        bool get_field_int64(void *impl, const char *sField, int64_t &nValue)
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error()) {
                return false;
            }

            return simd_value_to_int64(value, nValue);
        }

        bool get_field_int32(void *impl, const char *sField, int32_t &nValue)
        {
            // TODO: specific implementation for int32_t
            int64_t nVal64;
            if ( !get_field_int64(impl, sField, nVal64) ) {
                return false;
            }
            nValue = (int32_t)nVal64;
            return true;
        }

        template <class TValue>
        static bool simd_value_to_double(TValue &value, double &dValue)
        {
            switch (value.type())
            {
            case simdjson::ondemand::json_type::array:
                return false;
            break;
            case simdjson::ondemand::json_type::object:
                return false;
            break;
            case simdjson::ondemand::json_type::number:
            {
                simdjson::ondemand::number_type t = value.get_number_type();
                switch(t) 
                {
                    case simdjson::ondemand::number_type::signed_integer:
                    case simdjson::ondemand::number_type::unsigned_integer: {
                        dValue = (double)value.get_int64();
                        return true;
                    }
                    break;
                    case simdjson::ondemand::number_type::floating_point_number: {
                        dValue = value.get_double();
                        return true;
                    }
                    break;
                }
                
                return false;
            }
            break;
            case simdjson::ondemand::json_type::string: 
                value.get_double_in_string().get(dValue);
                return true;   
            break;
            case simdjson::ondemand::json_type::boolean:
                dValue = (value.get_bool() == true) ? 1 : 0;
                return true;
            break;
            case simdjson::ondemand::json_type::null:
                dValue = 0;
                return true;
            break;
            }

            return false;
        }

        bool get_field_double(void *impl, const char *sField, double &dValue)
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error() ) {
                return false; // simdjson::UNINITIALIZED; // simdjson::NO_SUCH_FIELD
            }
            
            return simd_value_to_double(value, dValue);
        }

        template <class TValue>
        static bool simd_value_to_string(TValue &value, std::string &value_str)
        {
            switch (value.type())
            {
            case simdjson::ondemand::json_type::array: {
                    std::string_view str_view = simdjson::to_json_string(value); // ?simdjson::to_json_string (get_raw_json_string crash)
                    value_str = str_view;
                    return true;
                }
            break;
            case simdjson::ondemand::json_type::object: {
                    std::string_view str_view = simdjson::to_json_string(value); // ?simdjson::to_json_string (get_raw_json_string crash)
                    value_str = str_view;
                    return true;
                }
            break;
            case simdjson::ondemand::json_type::number:
            {
                simdjson::ondemand::number_type t = value.get_number_type();
                switch(t) 
                {
                    case simdjson::ondemand::number_type::signed_integer: {
                        value_str = std::to_string(value.get_uint64()); // quick hack TODO: expectation access raw string value?
                        return true;
                    }
                    break;
                    case simdjson::ondemand::number_type::unsigned_integer: {
                        value_str = std::to_string(value.get_int64()); // quick hack TODO: expectation access raw string value?
                        return true;
                    }
                    break;
                    case simdjson::ondemand::number_type::floating_point_number: {
                        value_str = std::to_string(value.get_double()); // quick hack TODO: expectation access raw string value?
                        return true;
                    }
                    break;
                }
                
                return false;
            }
            break;
            case simdjson::ondemand::json_type::string: {
                std::string_view view;
                value.get(view);
                value_str = view;
                return true;
            }
            break;
            case simdjson::ondemand::json_type::boolean:
                value_str = (value.get_bool() == true) ? "1" : "0";
                return true;
            break;
            case simdjson::ondemand::json_type::null:
                value_str = "0";
                return true;
            break;
            }

            return false;
        }

        bool get_field_string(void *impl, const char *sField, std::string &value_str) 
        {
            simd_wrapper *doc = (simd_wrapper *)impl;
            auto value = (*doc)[sField];
            if (value.error()) {
                return false;
            }

            return simd_value_to_string(value, value_str);
        }

        //////////////////////////////////////////////////////////
        // array based specific
        //////////////////////////////////////////////////////////
        void create_array(void *&impl, size_t reserve_size) 
        {
            ASSERT(!impl);
            impl = new simd_arr_wrapper;
        }

        void destroy_array(void *&impl)
        {
            delete (simd_type *)impl; 
        }

        size_t get_array_size(const void *impl) 
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;
            if ( !doc->defined() ) {
                return 0;
            }
            return doc->m_arr.count_elements();
        }

        void get_array_string(const void *impl, size_t i, std::string &sValue) 
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;
            if ( !doc->defined() ) {
                ASSERT(FALSE);
                return;
            }
            doc->Iterate();
            simdjson::ondemand::value value = (*doc->m_it);
            simd_value_to_string(value, sValue);
        }

        double get_array_double(const void *impl, size_t i)
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;
            if ( !doc->defined() ) {
                ASSERT(FALSE);
                return 0.;
            }
            doc->Iterate();
            simdjson::ondemand::value value = (*doc->m_it);
            double dValue;
            if ( !simd_value_to_double(value, dValue) ) {
                dValue = 0.;
            }
            return dValue;
        }

        int32_t get_array_int32(const void *impl, size_t i)
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;
            if ( !doc->defined() ) {
                ASSERT(FALSE);
                return 0;
            }

            doc->Iterate();
            simdjson::ondemand::value value = (*doc->m_it);
            int64_t nValue;
            if ( !simd_value_to_int64(value, nValue) ) {
                nValue = 0;
            }

            return (int32_t)nValue;
        }
        
        int64_t get_array_int64(const void *impl, size_t i)
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;
            if ( !doc->defined() ) {
                ASSERT(FALSE);
                return 0;
            }
            doc->Iterate();
            simdjson::ondemand::value value = (*doc->m_it);
            int64_t nValue = 0;
            if ( !simd_value_to_int64(value, nValue) ) {
                nValue = 0;
            }
            
            return nValue;
        }
        
        void get_array_object(const void *impl, size_t i, void *obj) 
        {
            simd_arr_wrapper *doc = (simd_arr_wrapper *)impl;

            if ( !doc->defined() ) {
                ASSERT(FALSE);
                return;
            }
            doc->Iterate();
            simdjson::ondemand::value value = (*doc->m_it);

            switch (value.type())
            {
            case simdjson::ondemand::json_type::array: {
                    simd_value_to_array(value, obj); 
                    return;
                }
                break;
            case simdjson::ondemand::json_type::object: {
                    simd_value_to_object(value, obj);
                    return;
                }
                break;
            case simdjson::ondemand::json_type::string:
                {
                    std::string_view view;
                    value.get(view);
                    std::string value_str;
                    value_str = view;
                    str2obj(std::move(value_str), obj);
                    return;
                }
                break;
            case simdjson::ondemand::json_type::null: {
                    ASSERT(FALSE); // crash will happen, do filter null values from array before parse FIXME
                                   // e.g.: for (const ds_json::read::object &obj: obj_array)  ? what should happen with object
                    return; // just ignore
                }
                break;
            }

            ASSERT(FALSE);
            return;
        }
    }
};