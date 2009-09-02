

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  HP aCC C++ compiler setup:

#if (__HP_aCC <= 33100)
#error Compiler too old for Ambulant
#endif

#if (__HP_aCC <= 33300)
// member templates are sufficiently broken that we disable them for now
#    define AMBULANT_NO_MEMBER_TEMPLATES
#    define AMBULANT_NO_DEPENDENT_NESTED_DERIVATIONS
#endif

#if (__HP_aCC <= 33900) || !defined(AMBULANT_STRICT_CONFIG)
#    define AMBULANT_NO_UNREACHABLE_RETURN_DETECTION
#    define AMBULANT_NO_TEMPLATE_TEMPLATES
#    define AMBULANT_NO_SWPRINTF
#    define AMBULANT_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS
//     std lib config should set this one already:
//#    define AMBULANT_NO_STD_ALLOCATOR
#endif 

// optional features rather than defects:
#if (__HP_aCC >= 33900)
#    define AMBULANT_HAS_LONG_LONG
#    define AMBULANT_HAS_PARTIAL_STD_ALLOCATOR
#endif

#define AMBULANT_COMPILER "HP aCC version " AMBULANT_STRINGIZE(__HP_aCC)

//
// versions check:
// we don't support HP aCC prior to version 0:
#if __HP_aCC < 33000
#  error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version is 0:
#if (__HP_aCC > 33900)
#  if defined(AMBULANT_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif



