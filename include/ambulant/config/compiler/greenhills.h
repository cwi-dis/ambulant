

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  Greenhills C++ compiler setup:

#define AMBULANT_COMPILER "Greenhills C++ version " AMBULANT_STRINGIZE(__ghs)

#include "ambulant/config/compiler/common_edg.h"

//
// versions check:
// we don't support Greenhills prior to version 0:
#if __ghs < 0
#  error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 0:
#if (__ghs > 0)
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif

