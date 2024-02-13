#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__
#pragma once

// MIN MAX 
// ASSERT VERIFY
// LPCSTR and other stuff define

// If they have not #included W32Base.h (part of my W32 utility library) then
// we need to define some stuff.  Otherwise, this is all defined there.

// MACRO: SS_WIN32
// ---------------
//      When this flag is set, we are building code for the Win32 platform and
//      may use Win32 specific functions (such as LoadString).  This gives us
//      a couple of nice extras for the code.
//
//      Obviously, Microsoft's is not the only compiler available for Win32 out
//      there.  So I can't just check to see if _MSC_VER is defined to detect
//      if I'm building on Win32.  So for now, if you use MS Visual C++ or
//      Borland's compiler, I turn this on.  Otherwise you may turn it on
//      yourself, if you prefer

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(_WIN32)
    #define SS_WIN32
#endif

// MACRO: SS_ANSI
// --------------
//      When this macro is defined, the code attempts only to use ANSI/ISO
//      standard library functions to do it's work.  It will NOT attempt to use
//      any Win32 of Visual C++ specific functions -- even if they are
//      available.  You may define this flag yourself to prevent any Win32
//      of VC++ specific functions from being called. 

// If we're not on Win32, we MUST use an ANSI build

#ifndef SS_WIN32
    #if !defined(SS_NO_ANSI)
        #define SS_ANSI
    #endif
#endif

#if !defined(W32BASE_H)
    // If they want us to use only standard C++ stuff (no Win32 stuff)
    #ifdef SS_ANSI
        // On Win32 we have TCHAR.H so just include it.  This is NOT violating
        // the spirit of SS_ANSI as we are not calling any Win32 functions here.
        #ifdef SS_WIN32
            #include <TCHAR.H>
            #include <WTYPES.H>
            #ifndef STRICT
                #define STRICT
            #endif
        // ... but on non-Win32 platforms, we must #define the types we need.
        #else
            
        #endif  // #ifndef _WIN32


        // Make sure ASSERT and verify are defined using only ANSI stuff
        #ifndef DEBUG_ONLY
            #ifdef _DEBUG
                #define DEBUG_ONLY(f)      (f)
            #else
                #define DEBUG_ONLY(f)      ((void)0)
            #endif
        #endif

        #ifndef ASSERT
            #ifdef _DEBUG
                #include "crtdbg.h" // msw specific
                #include <assert.h>
                inline BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine) // MSW specific do stop on line
                {
                    // we remove WM_QUIT because if it is in the queue then the message box
                    // won't display
                    MSG msg;
                    BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
                    BOOL bResult = _CrtDbgReport(_CRT_ASSERT, lpszFileName, nLine, NULL, NULL);
                    if (bQuit)
                        PostQuitMessage((int)msg.wParam);
                    return bResult;
                }
                #define ASSERT(f)          DEBUG_ONLY((void) ((f) || !::AssertFailedLine(__FILE__, __LINE__) || (__debugbreak(), 0)))
                //#define ASSERT(f) assert((f)) // non msw impl
            #else
                #define ASSERT(f) 
            #endif
        #endif
        #ifndef VERIFY
            #ifdef _DEBUG
                #define VERIFY(x) ASSERT((x))
            #else
                #define VERIFY(x) x
            #endif
        #endif

    #else // ...else SS_ANSI is NOT defined

        #include <TCHAR.H>
        #include <WTYPES.H>
        #ifndef STRICT
            #define STRICT
        #endif

        // Make sure ASSERT and verify are defined

        #ifndef ASSERT
            #include <crtdbg.h>
            #define ASSERT(f) _ASSERTE((f))
        #endif
        #ifndef VERIFY
            #ifdef _DEBUG
                #define VERIFY(x) ASSERT((x))
            #else
                #define VERIFY(x) x
            #endif
        #endif

    #endif // #ifdef SS_ANSI

    #ifndef UNUSED
        #define UNUSED(x) x
    #endif

#endif // #ifndef W32BASE_H

#ifdef _DEBUG
    #if defined (_MSC_VER)
        #include "crtdbg.h" // msw specific
        #include <assert.h>
        #include <comdef.h>
    #endif

// Should be in the separe file
// Trace functionality
inline void TraceDump(const wchar_t *lpsz)
{
    if (!lpsz) {
        OutputDebugString(L"(NULL)");
    }

#ifndef _countof
    #define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

    wchar_t szBuffer[512];
    wchar_t *lpBuf = szBuffer;
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

inline void Trace(const wchar_t *format, ... )
{
    va_list args;
    wchar_t buf[1500]; 

    va_start(args, format);
    vswprintf(buf, format, args);
    va_end(args);

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

inline void *_malloc_dbg2(size_t _Size, int _BlockUse, char const *_FileName, int _LineNumber) {
    return _malloc_dbg(_Size, _BlockUse, _FileName, _LineNumber);
}

inline void *_realloc_dbg2(void *_Block, size_t _Size, int _BlockUse, char const *_FileName, int _LineNumber) {
    return _realloc_dbg(_Block, _Size, _BlockUse, _FileName, _LineNumber);
}

inline void _free_dbg2(void *_Block, int _BlockUse) {
    return _free_dbg(_Block, _BlockUse);
}

namespace std {
    
    inline void *_malloc_dbg(size_t _Size, int _BlockUse, char const *_FileName, int _LineNumber) {
        return _malloc_dbg2(_Size, _BlockUse, _FileName, _LineNumber);
    }

    inline void _free_dbg(void *_Block, int _BlockUse) {
        _free_dbg2(_Block, _BlockUse);
    }

    inline void *_realloc_dbg(void *_Block, size_t _Size, int _BlockUse, char const *_FileName, int _LineNumber) {
        return _realloc_dbg2(_Block, _Size, _BlockUse, _FileName, _LineNumber);
    }
};

#endif // _DEBUG

#ifndef DEBUG_ONLY
    #ifdef _DEBUG
        #define DEBUG_ONLY(f)      (f)
    #else
        #define DEBUG_ONLY(f)      ((void)0)
    #endif
#endif

#ifndef ASSERT
    #ifdef _DEBUG
        #include "crtdbg.h" // msw specific
        #include <assert.h>
        inline BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine) // MSW specific do stop on line
        {
            // we remove WM_QUIT because if it is in the queue then the message box
            // won't display
            MSG msg;
            BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
            BOOL bResult = _CrtDbgReport(_CRT_ASSERT, lpszFileName, nLine, NULL, NULL);
            if (bQuit)
                PostQuitMessage((int)msg.wParam);
            return bResult;
        }
        #define ASSERT(f)          DEBUG_ONLY((void) ((f) || !::AssertFailedLine(__FILE__, __LINE__) || (__debugbreak(), 0)))
        //#define ASSERT(f) assert((f)) // non msw impl
    #else
        #define ASSERT(f) 
    #endif
#endif

#endif