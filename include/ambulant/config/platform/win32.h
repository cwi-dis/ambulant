

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  Win32 specific config options:

#define AMBULANT_PLATFORM "Win32"

#define AMBULANT_PLATFORM_WIN32

#if defined(__GNUC__) && !defined(AMBULANT_NO_SWPRINTF)
#  define AMBULANT_NO_SWPRINTF
#endif

#ifndef AMBULANT_DISABLE_WIN32
//
// Win32 will normally be using native Win32 threads,
// but there is a pthread library avaliable as an option:
//
#ifndef AMBULANT_HAS_PTHREADS
#  define AMBULANT_HAS_WINTHREADS
#endif

// WEK: Added
#define AMBULANT_HAS_FTIME

#endif

//
// DLL cruft section.
// Define AMBULANT_BUILD_DLL when creating the DLL,
// Define AMBULANT_USE_DLL when using the DLL.
//
#ifdef AMBULANT_BUILD_DLL
#define AMBULANTAPI __declspec(dllexport)
#endif
#ifdef AMBULANT_USE_DLL
#define AMBULANTAPI __declspec(dllimport)
#endif

/////////////////////////////
// BEGIN WIN32 WINCE SECTION
#if defined(_WIN32_WCE)

#define AMBULANT_PLATFORM_WIN32_WCE

#if _WIN32_WCE < 5
#define AMBULANT_PLATFORM_WIN32_WCE_3

// The following symbol has been used with a different 
// meaning by other platforms
#define AMBULANT_NO_IOSTREAMS
#define AMBULANT_NO_OSTREAM
#define AMBULANT_NO_STRINGSTREAM
// Define the following symbol as a replacement for AMBULANT_NO_IOSTREAMS
// When the following symbol is defined
// no headers related to streams should be included.
// Not only the std ones but also ostream.h, istream.h etc
#define AMBULANT_NO_IOSTREAMS_HEADERS

// implied: #define AMBULANT_NO_STRINGSTREAM
#define AMBULANT_NO_ABORT


#ifdef AMBULANT_PLATFORM_WIN32_WCE_3

// signed/unsigned mismatch
#pragma warning( disable : 4018)

#include <wce_defs.h>
typedef unsigned short wchar_t;
namespace std {
  using ::ptrdiff_t;
}

#include <string>
inline bool operator != (const std::string& s, const char *p) {
	return !(s == p);
}

// Actually compiler property
#define AMBULANT_NO_MEMBER_TEMPLATES
#define AMBULANT_NO_TIME_H

#elif defined(AMBULANT_PLATFORM_WIN32_WCE_4)

#define abort()

#endif

#else
// Windows Mobile 5
#define AMBULANT_PLATFORM_WIN32_WCE_5
#define AMBULANT_NO_TIME_H
#define abort() exit(1)
#endif // _win32_WCE < 5
#endif // defined(_WIN32_WCE)
// END WIN32 WINCE SECTION

/////////////////////////////

///////////////////////////
// The char type and the string routines are used in many places

#ifdef UNICODE
// UNICODE
typedef wchar_t text_char;
#define text_str(quote) L##quote
#define text_strchr wcschr
#define text_strrchr wcsrchr
#define text_strtok wcstok
#define text_strlen wcslen
#define text_vscprintf _vscwprintf
#define text_strcat wcscat
#else
// MB (not UNICODE)
typedef char text_char;
#define text_str(quote) quote
#define text_strchr strchr
#define text_strrchr strrchr
#define text_strtok strtok
#define text_strlen strlen
#define text_vscprintf _vscprintf
#define text_strcat strcat
#endif //UNICODE (#ifdef ... #else ... $endif)

///////////////////////////

//
// disable min/max macros:
//
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#ifdef AMBULANT_MSVC
namespace std{
  // Apparently, something in the Microsoft libraries requires the "long"
  // overload, because it calls the min/max functions with arguments of
  // slightly different type.  (If this proves to be incorrect, this
  // whole "AMBULANT_MSVC" section can be removed.)
  inline long min(long __a, long __b) {
    return __b < __a ? __b : __a;
  }
  inline long max(long __a, long __b) {
    return  __a < __b ? __b : __a;
  }
  // The "long double" overload is required, otherwise user code calling
  // min/max for floating-point numbers will use the "long" overload.
  // (SourceForge bug #495495)
  inline long double min(long double __a, long double __b) {
    return __b < __a ? __b : __a;
  }
  inline long double max(long double __a, long double __b) {
    return  __a < __b ? __b : __a;
  }
}
using std::min;
using std::max;
#     endif
