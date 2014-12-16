

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  GNU C++ compiler setup:

#   if __GNUC__ < 4
#error Only GCC 4 and greater supported.
#endif

//
// Threading support: Turn this on unconditionally here (except for
// MinGW, where we can know for sure). It will get turned off again
// later if no threading API is detected.
//
#if !defined(__MINGW32__) || defined(_MT)
# define AMBULANT_HAS_THREADS
#endif 

//
// gcc has "long long"
//
#define AMBULANT_HAS_LONG_LONG

//
// gcc implements the named return value optimization since version 3.1
//
#if __GNUC__ > 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ >= 1 )
#define AMBULANT_HAS_NRVO
#endif

#define AMBULANT_COMPILER "GNU C++ version " __VERSION__

//
// versions check:
// we don't know gcc prior to version 2.90:
#if (__GNUC__ == 2) && (__GNUC_MINOR__ < 90)
#  error "Compiler not configured - please reconfigure"
#endif
//
// Jack checked 4.4 with boost configure and it seems fine:
// Jack: Ambulant builds fine with 4.7.3
#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 9))
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  else
#     warning "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif
