

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  SGI C++ compiler setup:

#define AMBULANT_COMPILER "SGI Irix compiler version " AMBULANT_STRINGIZE(_COMPILER_VERSION)

#include "ambulant/config/compiler/common_edg.h"

//
// Threading support:
// Turn this on unconditionally here, it will get turned off again later
// if no threading API is detected.
//
#define AMBULANT_HAS_THREADS
//
// version check:
// probably nothing to do here?

