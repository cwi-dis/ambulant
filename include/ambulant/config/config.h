

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org/libs/config for most recent version.

//  Boost config.h policy and rationale documentation has been moved to
//  http://www.boost.org/libs/config
//
//  CAUTION: This file is intended to be completely stable -
//           DO NOT MODIFY THIS FILE!
//

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_CONFIG_H
#define AMBULANT_CONFIG_H

// if we don't have a user config, then use the default location:
#if !defined(AMBULANT_USER_CONFIG) && !defined(AMBULANT_NO_USER_CONFIG)
#  define AMBULANT_USER_CONFIG "ambulant/config/user.h"
#endif
// include it first:
#ifdef AMBULANT_USER_CONFIG
#  include AMBULANT_USER_CONFIG
#endif

// if we don't have a compiler config set, try and find one:
#if !defined(AMBULANT_COMPILER_CONFIG) && !defined(AMBULANT_NO_COMPILER_CONFIG) && !defined(AMBULANT_NO_CONFIG)
#  include "ambulant/config/select_compiler_config.h"
#endif
// if we have a compiler config, include it now:
#ifdef AMBULANT_COMPILER_CONFIG
#  include AMBULANT_COMPILER_CONFIG
#endif

// if we don't have a std library config set, try and find one:
#if !defined(AMBULANT_STDLIB_CONFIG) && !defined(AMBULANT_NO_STDLIB_CONFIG) && !defined(AMBULANT_NO_CONFIG)
#  include "ambulant/config/select_stdlib_config.h"
#endif
// if we have a std library config, include it now:
#ifdef AMBULANT_STDLIB_CONFIG
#  include AMBULANT_STDLIB_CONFIG
#endif

// if we don't have a platform config set, try and find one:
#if !defined(AMBULANT_PLATFORM_CONFIG) && !defined(AMBULANT_NO_PLATFORM_CONFIG) && !defined(AMBULANT_NO_CONFIG)
#  include "ambulant/config/select_platform_config.h"
#endif
// if we have a platform config, include it now:
#ifdef AMBULANT_PLATFORM_CONFIG
#  include AMBULANT_PLATFORM_CONFIG
#endif

// get config suffix code:
#include "ambulant/config/suffix.h"

// Also here, include gettext.h (for optional internationalization)
#include "ambulant/config/gettext.h"

// Set the default for the macro that flags our
// API to empty. (This macro will have been set to the dllexport
// mumbo-jumbo for Windows).
#ifndef AMBULANTAPI
#define AMBULANTAPI
#endif
#endif  // AMBULANT_CONFIG_H










