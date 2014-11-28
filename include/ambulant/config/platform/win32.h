

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
#pragma warning(disable: 4275) //  warning C4275: non dll-interface class 'ambulant::...' used as base for dll-interface class 
#pragma warning(disable: 4251) //  warning C4251: 'ambulant::...' : class 'std::...' needs to have dll-interface to be used by clients
/////////////////////////////
// BEGIN WIN32 WINCE SECTION
#if defined(_WIN32_WCE)
#error WinCE no longer supported by Ambulant
#endif
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

#else
// MB (not UNICODE)
typedef char text_char;
#define text_str(quote) quote
#define text_strchr strchr
#define text_strrchr strrchr
#define text_strtok strtok
#define text_strlen strlen
#define text_vscprintf _vscprintf
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
