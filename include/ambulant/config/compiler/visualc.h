

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  Microsoft Visual C++ compiler setup:

#define AMBULANT_MSVC _MSC_VER

// turn off the warnings before we #include anything
#pragma warning( disable : 4503 ) // warning: decorated name length exceeded

// turn off the warnings for virtuals 
#pragma warning( disable: 4250) // xxx : inherits yyy::function via dominance

#if _MSC_VER <= 1310  // 1200 == VC++ 7.1 (Visual Studio 2003)
#error Compiler too old for Ambulant
#endif

// XXXJACK: Unsure for VC8. Will add definitions as we find things.
// These unsafe warnings are very obnoxious:
#define _CRT_SECURE_NO_WARNINGS

#ifndef _NATIVE_WCHAR_T_DEFINED
#  define AMBULANT_NO_INTRINSIC_WCHAR_T
#endif

//   
// check for exception handling support:   
#ifndef _CPPUNWIND   
#  define AMBULANT_NO_EXCEPTIONS   
#endif 

//
// __int64 support:
//
#   define AMBULANT_HAS_MS_INT64
#   define AMBULANT_HAS_LONG_LONG
//
// disable Win32 API's if compiler extentions are
// turned off:
//
#ifndef _MSC_EXTENSIONS
#  define AMBULANT_DISABLE_WIN32
#endif

# if _MSC_VER == 1400
#   define AMBULANT_COMPILER_VERSION 8.0
# elif _MSC_VER == 1500
#   define AMBULANT_COMPILER_VERSION 9.0
# elif _MSC_VER == 1600
#	define AMBULANT_COMPILER_VERSION 10.0
#else
#   define AMBULANT_COMPILER_VERSION _MSC_VER
# endif

#define AMBULANT_COMPILER "Microsoft Visual C++ version " AMBULANT_STRINGIZE(AMBULANT_COMPILER_VERSION)

//
// last known and checked version is 1600:
#if (_MSC_VER > 1600)
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  else
#     pragma message("Unknown compiler version - please run the configure tests and report the results")
#  endif
#endif







