

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

/* 
 * @$Id$ 
 */

//  config for libstdc++ v3
//  not much to go in here:

#define AMBULANT_STDLIB "GNU libstdc++ version " AMBULANT_STRINGIZE(__GLIBCPP__)

#ifndef _GLIBCPP_USE_WCHAR_T
#  define AMBULANT_NO_CWCHAR
#  define AMBULANT_NO_CWCTYPE
#  define AMBULANT_NO_STD_WSTRING
#  define AMBULANT_NO_STD_WSTREAMBUF
#endif
 
#ifndef _GLIBCPP_USE_LONG_LONG
// May have been set by compiler/*.h, but "long long" without library
// support is useless.
#  undef AMBULANT_HAS_LONG_LONG
#endif

