

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  BeOS specific config options:

#define AMBULANT_PLATFORM "BeOS"

#define AMBULANT_NO_CWCHAR
#define AMBULANT_NO_CWCTYPE
#define AMBULANT_HAS_UNISTD_H

#define AMBULANT_HAS_BETHREADS

#ifndef AMBULANT_DISABLE_THREADS
#  define AMBULANT_HAS_THREADS
#endif

// boilerplate code:
#include <ambulant/config/posix_features.h>
 

