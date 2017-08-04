#ifndef __STDAFX_H__
#define __STDAFX_H__
#pragma once

#ifndef _WIN32_WINNT
// VC 10
// This file requires _WIN32_WINNT to be #defined at least to 0x0403. Value 0x0501 or higher is recommended
#define _WIN32_WINNT 0x0501 // Windows (VC 10 MFC requires minimal define)
// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#endif

//#ifndef __x86_64__ 
#include "afx.h" // Dao include
//#else
//#include "Windows.h"
//#include "Collections/DebugUtils.h"
//#endif

#define _BUILD_DB_UTILS_IMPL
#include "DBUtilsImpl.h"

#pragma warning (default: 4146)

// PragmaSet defines
#pragma warning (disable : 4995) //disable depreciation of 'CDaoDatabase'
#pragma warning (disable : 4996) //disable depreciation of 'stricmp'

#define VC_EXTRALEAN // Exclude rarely-used stuff from Windows headers

#endif
