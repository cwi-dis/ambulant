

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  IBM/Aix specific config options:

#define AMBULANT_PLATFORM "IBM Aix"

#define AMBULANT_HAS_UNISTD_H
#define AMBULANT_HAS_NL_TYPES_H
#define AMBULANT_HAS_NANOSLEEP
#define AMBULANT_HAS_CLOCK_GETTIME

// This needs support in "ambulant/cstdint.h" exactly like FreeBSD.
// This platform has header named <inttypes.h> which includes all
// the things needed.
#define AMBULANT_HAS_STDINT_H

// Threading API's:
#define AMBULANT_HAS_PTHREADS
#define AMBULANT_HAS_PTHREAD_DELAY_NP
#define AMBULANT_HAS_SCHED_YIELD
//#define AMBULANT_HAS_PTHREAD_YIELD

// boilerplate code:
#include <ambulant/config/posix_features.h>



