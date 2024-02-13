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

#pragma warning (disable: 4857) // c1xx : warning C4857: C++/CLI mode does not support C++ versions newer than C++17; setting language to /std:c++17

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
    #include "Windows.h"
    #include "Collections/debug_utils.h"
#endif

#define _BUILD_DB_UTILS_IMPL
#include "DBUtilsImpl.h"

#pragma warning (default: 4146)

// C++20 can be enabled -> enable /Zc:twoPhase- flag and disable warning
// also about CLR compatibility:
// https://developercommunity.visualstudio.com/t/vs-2019-preview-2-toolset-v142-clr-and-stdclatest/435513
//
// Daniel Griffing [MSFT]  
//
// Hello All,
// 
// During the increased pace of the ISO C++ standard over the past decade and efforts in the MSVC toolset to catch up to standards, 
// the team’s priorities have shifted in that direction and we’ve doubled down on our focus on native targeting. 
// This culminated in MSVC catching up to the C++17 during VS 2017 updates and extensive work in VS 2019 to support C++20 
// as it evolved in the ISO C++ committee. 
// As part of the prioritization of those efforts, some rough edges 
// and differences in behavior in C++11 through C++17 and C++/CLI mappings 
// to the .NET Framework have emerged. Bridging the gap between the behaviors 
// would require a significant design effort that likely includes changes 
// to the MSVC compiler, libraries, and even the .NET Framework. C++/CLI design has not greatly evolved since ECMA-372, 
// which is still C++03-based, and much of the subsequent feature support for C++/CLI has been implemented 
// as point fixes to address issues on an individual basis. 
// Unfortunately, an effort to fully resolve these differences would be much larger than what our team capacity and priorities allow.
//
// Based upon these factors and the fact that C++20 has built-in assumptions/dependencies on ISO C++ behavior from C++11/14/17 
// and avoid confusion and clearly communicate expectations with respect to compiler support, 
// a decision was made to limit the use of C++20 features (under /std:c++latest) and C++/CLI.
//
// VS 2019 supports “/std:c++17 and /clr” for VS 2017 project compatibility. 
// In VS 2017 updates, /std:c++latest was tracking the Preview C++17 standard implementation. Any code that successfully 
// built using that switch is supported in VS 2019 under the /std:c++17 mode.
// In VS 2019, /std:c++latest shifted its target to track Preview implementation of C++20 (draft) features.
// In VS 2019 v16.0, the /std:c++latest /clr mode combination resulted in a compile-time error. 
// Based upon feedback through Developer Community, we relaxed this restriction in the VS 2019 v16.2 update 
// to implicitly convert /std:c++latest to /std:c++17 
// with a warning to enable projects upgrading from VS 2017 which may be configured to use that switch combination.
//
// C++/CLI Investments in VS 2019
//
// In VS 2019 updates, the C++ team has continued to invest in C++/CLI with a focus on enabling .NET Core targeting on Windows. 
// The goal is to enable continued use of investments in C++/CLI for interop between native C++ code and .NET.
//
// In VS 2019 v16.6, C++/CLI support (/clr:netcore) was announced supporting targeting of .NET Core 3.1 
// and we’re continuing to work with .NET to ensure support for .NET 5 
// and any subsequent releases. One piece of this effort, separating the metadata merger from clr.dll, 
// has made it possible to address some aspects of the C++17 to C++/CLI language feature differences for .NET Core.
//
// In VS 2019 v16.6, support for C++ inline namespaces was added under /clr:netcore.
//  In VS 2019 v16.6, support for default arguments was added under /clr:netcore. 
//
// An interesting aspect of this is that ISO C++ and .NET have different specs on how default args are to be resolved, 
// where ISO C++ explicitly disallows default argument resolution in cases where all arguments to a function have default values; 
// this is allowed in .NET. This is something discovered after adding support 
// and we have a bug fix to address this issue coming in an upcoming VS 2019 servicing update.
//
// Guidance
//
// As you’re working with or evaluating native code that is used in managed interop scenarios, we recommend the following steps be taken.
//
// Ensure code that needs to be used in managed to native interoperability via C++/CLI successfully builds in /std:c++17 mode.
// Use P/Invoke in your managed code to call into native code, wherever possible.
// Use C++/CLI wrapper projects building in /std:c++17 /clr mode encapsulating code, via static linking 
// or as a DLL shim, that requires C++20 (/std:c++latest) support.
// For code that is not performance-critical, consider rewriting the code in a managed language such as C#.
// Request specific features that block use of /clr for managed to native interoperability scenarios via VS Developer Community.
//
// Looking forward
//
// Looking forward, the C++ team is committed to continuing support for C++/CLI allowing existing codebases continue support 
// for investments in this space and there are no plans for removal of /clr from the MSVC toolset. However, at this point in time, 
// we do not have plans for full support for C++20 code under /clr mode 
// and the compiler will continue to implicitly change (with warning emitted) to /std:c++17 
// when the C++20 mode is seen in the compiler arguments.
//
// We’re definitely interested in hearing about the feature differences 
// that are most impacting your ability to build interop wrappers and interfaces using C++/CLI. 
// Also, we’re interested in learning about feature areas in C++20 and later standards that would be most helpful and impactful 
// in further leveraging existing investments and will consider support for C++20 features 
// on an individual basis, for both customer value and technical feasibility, 
// to ensure we are maximizing the value of investments in this area.
//
// Regard,
//
// Daniel Griffing, Visual C++ Compiler Front-End

#endif