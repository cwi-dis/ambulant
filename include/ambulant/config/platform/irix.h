

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  SGI Irix specific config options:

#define AMBULANT_PLATFORM "SGI Irix"

#define AMBULANT_NO_SWPRINTF 
//
// these are not auto detected by POSIX feature tests:
//
#define AMBULANT_HAS_GETTIMEOFDAY
#define AMBULANT_HAS_PTHREAD_MUTEXATTR_SETTYPE


// boilerplate code:
#define AMBULANT_HAS_UNISTD_H
#include <ambulant/config/posix_features.h>


