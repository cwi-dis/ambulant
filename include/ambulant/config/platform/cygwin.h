

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  cygwin specific config options:

#define AMBULANT_PLATFORM "Cygwin"
#define AMBULANT_NO_CWCTYPE
#define AMBULANT_NO_CWCHAR
#define AMBULANT_NO_SWPRINTF
#define AMBULANT_HAS_DIRENT_H

//
// Threading API:
// See if we have POSIX threads, if we do use them, otherwise
// revert to native Win threads.
#define AMBULANT_HAS_UNISTD_H
#include <unistd.h>
#if defined(_POSIX_THREADS) && (_POSIX_THREADS+0 >= 0) && !defined(AMBULANT_HAS_WINTHREADS)
#  define AMBULANT_HAS_PTHREADS
#  define AMBULANT_HAS_SCHED_YIELD
#  define AMBULANT_HAS_GETTIMEOFDAY
#  define AMBULANT_HAS_PTHREAD_MUTEXATTR_SETTYPE
#  define AMBULANT_HAS_SIGACTION
#else
#  if !defined(AMBULANT_HAS_WINTHREADS)
#     define AMBULANT_HAS_WINTHREADS
#  endif
#  define AMBULANT_HAS_FTIME
#endif

// boilerplate code:
#include <ambulant/config/posix_features.h>
 


