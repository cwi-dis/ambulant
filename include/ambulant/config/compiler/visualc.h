

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

#if _MSC_VER <= 1200  // 1200 == VC++ 6.0
#pragma warning( disable : 4786 ) // ident trunc to '255' chars in debug info
#  define AMBULANT_NO_EXPLICIT_FUNCTION_TEMPLATE_ARGUMENTS
#  define AMBULANT_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#  define AMBULANT_NO_VOID_RETURNS
#  define AMBULANT_NO_EXCEPTION_STD_NAMESPACE
#  define AMBULANT_NO_DEDUCED_TYPENAME
#  define AMBULANT_DDRAW_EX
   // disable min/max macro defines on vc6:
   //
#endif

#if (_MSC_VER <= 1300)  // 1300 == VC++ 7.0

#if !defined(_MSC_EXTENSIONS) && !defined(AMBULANT_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS)      // VC7 bug with /Za
#  define AMBULANT_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#endif

#  define AMBULANT_NO_INCLASS_MEMBER_INITIALIZATION
#  define AMBULANT_NO_PRIVATE_IN_AGGREGATE
#  define AMBULANT_NO_ARGUMENT_DEPENDENT_LOOKUP
#  define AMBULANT_NO_INTEGRAL_INT64_T

//    VC++ 6/7 has member templates but they have numerous problems including
//    cases of silent failure, so for safety we define:
#  define AMBULANT_NO_MEMBER_TEMPLATES
//    For VC++ experts wishing to attempt workarounds, we define:
#  define AMBULANT_MSVC6_MEMBER_TEMPLATES

#  define AMBULANT_NO_MEMBER_TEMPLATE_FRIENDS
#  define AMBULANT_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#  define AMBULANT_NO_CV_VOID_SPECIALIZATIONS
#  define AMBULANT_NO_FUNCTION_TEMPLATE_ORDERING
#  define AMBULANT_NO_USING_TEMPLATE
#  define AMBULANT_NO_SWPRINTF
#  define AMBULANT_NO_TEMPLATE_TEMPLATES
#  if (_MSC_VER > 1200)
#     define AMBULANT_NO_MEMBER_FUNCTION_SPECIALIZATIONS
#  endif
#  define AMBULANT_DDRAW_EX

#endif

#if _MSC_VER < 1310 // 1310 == VC++ 7.1
#  define AMBULANT_NO_SWPRINTF
#  define AMBULANT_DDRAW_EX
#endif

#if _MSC_VER > 1310
// XXXJACK: Unsure for VC8. Will add definitions as we find things.
// These unsafe warnings are very obnoxious:
#define _CRT_SECURE_NO_WARNINGS
#endif

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
#if (_MSC_VER >= 1200) && defined(_MSC_EXTENSIONS)
#   define AMBULANT_HAS_MS_INT64
#endif
#if (_MSC_VER >= 1310) && defined(_MSC_EXTENSIONS)
#   define AMBULANT_HAS_LONG_LONG
#endif
//
// disable Win32 API's if compiler extentions are
// turned off:
//
#ifndef _MSC_EXTENSIONS
#  define AMBULANT_DISABLE_WIN32
#endif

# if _MSC_VER == 1200
#   define AMBULANT_COMPILER_VERSION 6.0
# elif _MSC_VER == 1300
#   define AMBULANT_COMPILER_VERSION 7.0
# elif _MSC_VER == 1310
#   define AMBULANT_COMPILER_VERSION 7.1
# elif _MSC_VER == 1400
#   define AMBULANT_COMPILER_VERSION 8.0
# else
#   define AMBULANT_COMPILER_VERSION _MSC_VER
# endif

#define AMBULANT_COMPILER "Microsoft Visual C++ version " AMBULANT_STRINGIZE(AMBULANT_COMPILER_VERSION)

//
// versions check:
// we don't support Visual C++ prior to version 6:
#if _MSC_VER < 1200
#error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 1400:
#if (_MSC_VER > 1400)
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  else
#     pragma message("Unknown compiler version - please run the configure tests and report the results")
#  endif
#endif







