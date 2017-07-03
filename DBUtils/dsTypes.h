#ifndef __DS_TYPES_H__
#define __DS_TYPES_H__
#pragma once

enum dsDBType
{
	dsType_Dao = 0,
	dsType_MsSQL,
	dsType_SqLite
};

enum dsFieldType
{
    dsFieldType_Undefined = -1,
    dsFieldType_Text      = 0,
    dsFieldType_Integer,
    dsFieldType_Double,
    dsFieldType_Blob,
    dsFieldType_DateTime, // sqlite defines dsFieldType_Integer
};

#endif