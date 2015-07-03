#ifndef __PRAGMA_SET_H__
#define __PRAGMA_SET_H__
#pragma once

#pragma message("Pragma warnings and messages included")

#define SS_ANSI // StdString definition, we use only ANSI mode

// Disabling
// The given formal parameter was never referenced in the body of the function 
// for which it was declared
#pragma warning  (disable : 4100)
// nonstandard extension used : class rvalue used as lvalue
#pragma warning (disable : 4238)
// signed/unsigned mismatch
#pragma warning (disable : 4018)
// disables warning: identifier was truncated to '255' characters in the browser information
#pragma warning (disable : 4786)

//local variable may be used without having been initialized
#pragma warning (disable : 4701)

// warning C++ language change
#pragma warning (disable : 4663)

// cast truncates constant value
#pragma warning (disable : 4310)

// nonstandard extension used: 'argument' : conversion
#pragma warning (disable : 4239)

// nonstandard extension used : nameless struct/union
#pragma warning (disable : 4201)

// warning C4018: '<' : signed/unsigned mismatch
#pragma warning (disable : 4018)

//'function' : This function or variable may be unsafe.
#pragma warning(disable: 4996)


#ifndef _AFXDLL
    // warning C4710: 'std::_Exception_ptr std::_Exception_ptr::_Current_exception(void)' : function not inlined
    #pragma warning(disable: 4710)
    // warning C4512: 'stlp_std::pair<_T1,_T2>' : assignment operator could not be generated
    #pragma warning(disable: 4512)
    // non dll-interface class used as base for dll-interface 
    #pragma warning (disable : 4275)
    // needs to have dll-interface to be used by clients of class
    #pragma warning (disable : 4251)
#else
    //'function' : function not inlined
    #pragma warning(default: 4710)
#endif

//'function' : function marked as __forceinline not inlined
#pragma warning(default: 4714)

//'insert_unique' : decorated name length exceeded, name was truncated
#pragma warning(disable: 4503)

#ifdef _DEBUG
	//identifier : local variable is initialized but not referenced"
	#pragma warning (error : 4189)
	// local variable 'name' used without having been initialized
	#pragma warning(error: 4700)
	//'identifier' : unreferenced local variable
	#pragma warning(error: 4101)
#else
	//identifier : local variable is initialized but not referenced"
	#pragma warning(default: 4189) 
	// local variable 'name' used without having been initialized
	#pragma warning(default: 4700) 
	//'identifier' : unreferenced local variable
	#pragma warning(default: 4101)
#endif

// we do no not flollow MS standart 
// strcpy, fopen and other (ANSI standart) "unsafe" functions are legal.
#define _CRT_SECURE_NO_DEPRECATE

#if _MSC_VER >= 1600 // VC10 or higher
	#pragma warning (disable : 4995) //disable depreciation of 'CDaoDatabase'
	#pragma warning (disable : 4996) //disable depreciation of 'stricmp'
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

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

#endif