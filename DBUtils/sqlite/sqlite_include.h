#ifndef __SQ_LITE_INCLUDE_H__
#define __SQ_LITE_INCLUDE_H__
#pragma once

#ifndef _SQLITE3_H_
    #include "sqlite3.h"
    // TODO: do add MSVC based defines to support gcc 
    #pragma comment(lib, "sqlite3.lib")  
#endif

#endif 
