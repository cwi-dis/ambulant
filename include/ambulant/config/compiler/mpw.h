

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  MPW C++ compilers setup:

#   if    defined(__SC__)
#     define AMBULANT_COMPILER "MPW SCpp version " AMBULANT_STRINGIZE(__SC__)
#   elif defined(__MRC__)
#     define AMBULANT_COMPILER "MPW MrCpp version " AMBULANT_STRINGIZE(__MRC__)
#   else
#     error "Using MPW compiler configuration by mistake.  Please update."
#   endif

//
// MPW 8.90:
//
#if (MPW_CPLUS <= 0x890) || !defined(AMBULANT_STRICT_CONFIG)
#  define AMBULANT_NO_CV_SPECIALIZATIONS
#  define AMBULANT_NO_DEPENDENT_NESTED_DERIVATIONS
#  define AMBULANT_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
#  define AMBULANT_NO_INCLASS_MEMBER_INITIALIZATION
#  define AMBULANT_NO_INTRINSIC_WCHAR_T
#  define AMBULANT_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#  define AMBULANT_NO_USING_TEMPLATE

#  define AMBULANT_NO_CWCHAR
#  define AMBULANT_NO_LIMITS_COMPILE_TIME_CONSTANTS

#  define AMBULANT_NO_STD_ALLOCATOR /* actually a bug with const reference overloading */
#endif

//
// versions check:
// we don't support MPW prior to version 8.9:
#if MPW_CPLUS < 0x890
#  error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 0x890:
#if (MPW_CPLUS > 0x890)
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif

