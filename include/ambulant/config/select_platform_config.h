

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

// locate which platform we are on and define AMBULANT_PLATFORM_CONFIG as needed.
// Note that we define the headers to include using "header_name" not
// <header_name> in order to prevent macro expansion within the header
// name (for example "linux" is a macro on linux systems).

/* 
 * @$Id$ 
 */

#if defined(linux) || defined(__linux) || defined(__linux__)
// linux:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/linux.h"

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// BSD:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/bsd.h"

#elif defined(sun) || defined(__sun)
// solaris:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/solaris.h"

#elif defined(__sgi)
// SGI Irix:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/irix.h"

#elif defined(__hpux)
// hp unix:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/hpux.h"

#elif defined(__CYGWIN__)
// cygwin is not win32:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/cygwin.h"

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN32_WCE)
// win32:
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/win32.h"

#elif defined(__BEOS__)
// BeOS
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/beos.h"

#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
// MacOS
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/macos.h"

#elif defined(__IBMCPP__)
// IBM
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/aix.h"

#elif defined(__amigaos__)
// AmigaOS
#  define AMBULANT_PLATFORM_CONFIG "ambulant/config/platform/amigaos.h"

#else

#  if defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE)

   // generic unix platform:

#  ifndef AMBULANT_HAS_UNISTD_H
#     define AMBULANT_HAS_UNISTD_H
#  endif

#  include "ambulant/config/posix_features.h"

#  endif

#  if defined (AMBULANT_ASSERT_CONFIG)
      // this must come last - generate an error if we don't
      // recognise the platform:
#     error "Unknown platform - please configure and report the results to boost.org"
#  endif

#endif


