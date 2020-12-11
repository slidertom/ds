#ifndef __DS_TABLE_FIELD_INFO_H__
#define __DS_TABLE_FIELD_INFO_H__
#pragma once

#ifndef __DS_TYPES_H__
    #include "dsTypes.h"
#endif

#include "unordered_map"

class dsTableFieldInfo : public std::unordered_map<std::wstring, dsFieldType, std::hash<std::basic_string<wchar_t> > > { };

#endif