#ifndef __DS_TYPES_H__
#define __DS_TYPES_H__
#pragma once

enum class dsDBType
{
    Dao    = 0,
    MsSQL  = 1,
    SqLite = 2
};

enum class dsFieldType
{
    Undefined = -1,
    Text      = 0,
    Integer   = 2,
    Double    = 3,
    Blob      = 4,
    DateTime  = 5, // sqlite defines Integer
};

#endif