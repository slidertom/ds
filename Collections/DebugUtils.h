#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__
#pragma once

// MIN MAX 
// ASSERT VERIFY
// LPCSTR and other stuff define
#ifndef STDSTRING_H
	#include "stdstring.h"
#endif

#ifdef _DEBUG
// Should be in the separe file
// Trace functionality
inline void TraceDump(LPCTSTR lpsz)
{
	if (lpsz == NULL)
	{
		OutputDebugString(_T("(NULL)"));
	}

#ifndef _countof
	#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

	TCHAR szBuffer[512];
	LPTSTR lpBuf = szBuffer;
	while (*lpsz != '\0')
	{
		if (lpBuf > szBuffer + _countof(szBuffer) - 3)
		{
			*lpBuf = '\0';
			OutputDebugString(szBuffer);
			lpBuf = szBuffer;
		}
		if (*lpsz == '\n')
			*lpBuf++ = '\r';
		*lpBuf++ = *lpsz++;
	}

	*lpBuf = '\0';
	OutputDebugString(szBuffer);
}

#include <stdio.h>

inline void Trace(const TCHAR *format, ... )
{
	va_list args;
	TCHAR buf[ 1500 ]; // adjust size as needed
					   // Dainius Ceponis: 1024 -> 1500

	va_start( args, format );
	_vstprintf( buf, format, args );
	va_end( args );

	OutputDebugString(buf);
}
#endif //_DEBUG

// Define TRACE
#ifdef _DEBUG
	#ifndef TRACE 
		#define TRACE Trace
	#endif
#else
	#ifndef TRACE
		#define TRACE 
	#endif
#endif

// TRACE defined


#ifdef _DEBUG

#ifndef __LEAK_WATCH
#define __LEAK_WATCH

#include <crtdbg.h>

__inline void* __cdecl operator new(size_t nSize, const char * lpszFileName, int nLine)
{
    return ::operator new(nSize, 1, lpszFileName, nLine);
}

//Disabling recursive function call warning since it does not cause any crash and
//delete operator required for new to be overrided
#pragma warning(disable: 4717)

__inline void __cdecl operator delete(void * _P, const char * lpszFileName, int nLine)
{
	::operator delete(_P, lpszFileName, nLine);
}

#define DEBUG_NEW new(__FILE__, __LINE__)
#define malloc(s)       _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)    _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)   _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)   _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)         _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)       _msize_dbg(p, _NORMAL_BLOCK)

#endif

#endif // _DEBUG

#endif