

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  Mac OS specific config options:

#define AMBULANT_PLATFORM "Mac OS"

#if __MACH__ && !defined(_MSL_USING_MSL_C)

// Using the Mac OS X system BSD-style C library.
#define AMBULANT_PLATFORM_MACOS
#define AMBULANT_PLATFORM_UNIX

#  define AMBULANT_NO_CTYPE_FUNCTIONS
#  define AMBULANT_NO_CWCHAR
#  ifndef AMBULANT_HAS_UNISTD_H
#    define AMBULANT_HAS_UNISTD_H
#  endif
// boilerplate code:
#  ifndef TARGET_CARBON
#     include "ambulant/config/posix_features.h"
#  endif
#  ifndef AMBULANT_HAS_STDINT_H
#     define AMBULANT_HAS_STDINT_H
#  endif

//
// BSD runtime has pthreads, sigaction, sched_yield and gettimeofday,
// of these only pthreads are advertised in <unistd.h>, so set the 
// other options explicitly:
//
#  define AMBULANT_HAS_SCHED_YIELD
#  define AMBULANT_HAS_GETTIMEOFDAY
#  define AMBULANT_HAS_SIGACTION

#  ifndef __APPLE_CC__

// GCC strange "ignore std" mode works better if you pretend everything
// is in the std namespace, for the most part.

#    define AMBULANT_NO_STDC_NAMESPACE
#  endif

#else

// Using the MSL C library.

// We will eventually support threads in non-Carbon builds, but we do
// not support this yet.
#  if TARGET_CARBON

#    define AMBULANT_HAS_MPTASKS

// The MP task implementation of Boost Threads aims to replace MP-unsafe
// parts of the MSL, so we turn on threads unconditionally.
#    define AMBULANT_HAS_THREADS

// The remote call manager depends on this.
#    define AMBULANT_BIND_ENABLE_PASCAL

#  endif

#endif

