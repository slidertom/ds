#ifndef __SQ_LITE_INCLUDE_H__
#define __SQ_LITE_INCLUDE_H__
#pragma once

#ifndef _SQLITE3_H_
    #include "sqlite3.h"
    // TODO: do add MSVC based defines to support gcc 
    #ifdef __x86_64__ 
        #pragma comment(lib, "sqlite3_x64.lib")  
    #else
    #pragma comment(lib, "sqlite3.lib")  
    #endif
#endif

#endif 