

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  Do not check in modified versions of this file,
//  This file may be customized by the end user, but not by boost.

/* 
 * @$Id$ 
 */

//
//  Use this file to define a site and compiler specific
//  configuration policy:
//

// define this to locate a compiler config file:
// #define AMBULANT_COMPILER_CONFIG <myheader>

// define this to locate a stdlib config file:
// #define AMBULANT_STDLIB_CONFIG   <myheader>

// define this to locate a platform config file:
// #define AMBULANT_PLATFORM_CONFIG <myheader>

// define this to disable compiler config,
// use if your compiler config has nothing to set:
// #define AMBULANT_NO_COMPILER_CONFIG

// define this to disable stdlib config,
// use if your stdlib config has nothing to set:
// #define AMBULANT_NO_STDLIB_CONFIG

// define this to disable platform config,
// use if your platform config has nothing to set:
// #define AMBULANT_NO_PLATFORM_CONFIG

// define this to disable all config options,
// excluding the user config.  Use if your
// setup is fully ISO compliant, and has no
// useful extensions, or for autoconf generated
// setups:
// #define AMBULANT_NO_CONFIG

// define this to make the config "optimistic"
// about unknown compiler versions.  Normally
// unknown compiler versions are assumed to have
// all the defects of the last known version, however
// setting this flag, causes the config to assume
// that unknown compiler versions are fully conformant
// with the standard:
// #define AMBULANT_STRICT_CONFIG

// define this to cause the config to halt compilation
// with an #error if it encounters anything unknown --
// either an unknown compiler version or an unknown
// compiler/platform/library:
// #define AMBULANT_ASSERT_CONFIG


// define if you want to disable threading support, even
// when available:
// #define AMBULANT_DISABLE_THREADS

// define when you want to disable Win32 specific features
// even when available:
// #define AMBULANT_DISABLE_WIN32


