

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  generic BSD config options:

#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__)
#error "This platform is not BSD"
#endif

#ifdef __FreeBSD__
#define AMBULANT_PLATFORM "FreeBSD " AMBULANT_STRINGIZE(__FreeBSD__)
#elif defined(__NetBSD__)
#define AMBULANT_PLATFORM "NetBSD " AMBULANT_STRINGIZE(__NetBSD__)
#elif defined(__OpenBSD__)
#define AMBULANT_PLATFORM "OpenBSD " AMBULANT_STRINGIZE(__OpenBSD__)
#endif

//
// is this the correct version check?
// FreeBSD has <nl_types.h> but does not
// advertise the fact in <unistd.h>:
//
#if defined(__FreeBSD__) && (__FreeBSD__ >= 3)
#  define AMBULANT_HAS_NL_TYPES_H
#endif

//
// FreeBSD 3.x has pthreads support, but defines _POSIX_THREADS in <pthread.h>
// and not in <unistd.h>
//
#if defined(__FreeBSD__) && (__FreeBSD__ <= 3)
#  define AMBULANT_HAS_PTHREADS
#endif

//
// No wide character support in the BSD header files:
//
#define AMBULANT_NO_CWCHAR

//
// The BSD <ctype.h> has macros only, no functions:
//
#define AMBULANT_NO_CTYPE_FUNCTIONS

//
// thread API's not auto detected:
//
#define AMBULANT_HAS_SCHED_YIELD
#define AMBULANT_HAS_NANOSLEEP
#define AMBULANT_HAS_GETTIMEOFDAY
#define AMBULANT_HAS_PTHREAD_MUTEXATTR_SETTYPE
#define AMBULANT_HAS_SIGACTION

// boilerplate code:
#define AMBULANT_HAS_UNISTD_H
#include <ambulant/config/posix_features.hpp>





