

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

// locate which std lib we are using and define AMBULANT_STDLIB_CONFIG as needed:

// we need to include a std lib header here in order to detect which
// library is in use, use <utility> as it's about the smallest
// of the std lib headers - do not rely on this header being included -
// users can short-circuit this header if they know whose std lib
// they are using.

/* 
 * @$Id$ 
 */

#include <utility>

#if defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)
// STLPort library; this _must_ come first, otherwise since
// STLport typically sits on top of some other library, we
// can end up detecting that first rather than STLport:
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/stlport.h"

#elif defined(__LIBCOMO__)
// Commeau STL:
#define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/libcomo.h"

#elif defined(__STD_RWCOMPILER_H__) || defined(_RWSTD_VER)
// Rogue Wave library:
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/roguewave.h"

#elif defined(__GLIBCPP__)
// GNU libstdc++ 3
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/libstdcpp3.h"

#elif defined(__STL_CONFIG_H)
// generic SGI STL
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/sgi.h"

#elif defined(__MSL_CPP__)
// MSL standard lib:
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/msl.h"

#elif defined(__IBMCPP__)
// take the default VACPP std lib
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/vacpp.h"

#elif defined(MSIPL_COMPILE_H)
// Modena C++ standard library
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/modena.h"

#elif (defined(_YVALS) && !defined(__IBMCPP__)) || defined(_CPPLIB_VER)
// Dinkumware Library (this has to appear after any possible replacement libraries):
#  define AMBULANT_STDLIB_CONFIG "ambulant/config/stdlib/dinkumware.h"

#elif defined (AMBULANT_ASSERT_CONFIG)
// this must come last - generate an error if we don't
// recognise the library:
#  error "Unknown standard library"

#endif


