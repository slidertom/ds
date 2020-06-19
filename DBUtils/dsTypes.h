#ifndef __DS_TYPES_H__
#define __DS_TYPES_H__
#pragma once

enum class dsDBType
{
    dsType_Dao    = 0,
    dsType_MsSQL  = 1,
    dsType_SqLite = 2
};

enum class dsFieldType
{
    dsFieldType_Undefined = -1,
    dsFieldType_Text      = 0,
    dsFieldType_Integer   = 2,
    dsFieldType_Double    = 3,
    dsFieldType_Blob      = 4,
    dsFieldType_DateTime  = 5, // sqlite defines dsFieldType_Integer
};

#endif