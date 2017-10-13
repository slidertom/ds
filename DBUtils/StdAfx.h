#ifndef __STDAFX_H__
#define __STDAFX_H__
#pragma once

// PragmaSet defines
#pragma warning (disable : 4995) //disable depreciation of 'CDaoDatabase'
#pragma warning (disable : 4996) //disable depreciation of 'stricmp'

#define SS_ANSI
#define VC_EXTRALEAN // Exclude rarely-used stuff from Windows headers

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

#ifndef __x86_64__ 
	#include "afx.h" // Dao include
#else

#ifdef _DEBUG

#include <crtdbg.h>
inline void* _malloc_dbg2(
	size_t      _Size,
	int         _BlockUse,
	char const* _FileName,
	int         _LineNumber
)
{
	return _malloc_dbg(_Size, _BlockUse, _FileName, _LineNumber);
}

inline void* _realloc_dbg2(
	void*       _Block,
	size_t      _Size,
	int         _BlockUse,
	char const* _FileName,
	int         _LineNumber
)
{
	return _realloc_dbg(_Block, _Size, _BlockUse, _FileName, _LineNumber);
}

inline void _free_dbg2(
	void* _Block,
	int   _BlockUse
)
{
	return _free_dbg(_Block, _BlockUse);
}

namespace std 
{   // rapid json uses: std::malloc 
	inline void* _malloc_dbg(
		size_t      _Size,
		int         _BlockUse,
		char const* _FileName,
		int         _LineNumber
	)
	{
		return _malloc_dbg2(_Size, _BlockUse, _FileName, _LineNumber);
	}

	inline void _free_dbg(
		void* _Block,
		int   _BlockUse
	)
	{
		_free_dbg2(_Block, _BlockUse);
	}

	inline void* _realloc_dbg(
		void*       _Block,
		size_t      _Size,
		int         _BlockUse,
		char const* _FileName,
		int         _LineNumber
	)
	{
		return _realloc_dbg2(_Block, _Size, _BlockUse, _FileName, _LineNumber);
	}
};
#endif
	#include "Windows.h"
	#include "Collections/debug_utils.h"
#endif

#define _BUILD_DB_UTILS_IMPL
#include "DBUtilsImpl.h"

#pragma warning (default: 4146)

#endif
