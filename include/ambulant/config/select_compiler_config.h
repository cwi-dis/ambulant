

//  (C) Copyright Boost.org 2001. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version.

// locate which compiler we are using and define
// AMBULANT_COMPILER_CONFIG as needed: 

/* 
 * @$Id$ 
 */

#if defined __GNUC__
//  GNU C++:
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/gcc.h"

# elif defined __COMO__
//  Comeau C++
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/comeau.h"

#elif defined __KCC
//  Kai C++
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/kai.h"

#elif defined __sgi
//  SGI MIPSpro C++
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/sgi_mipspro.h"

#elif defined __DECCXX
//  Compaq Tru64 Unix cxx
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/compaq_cxx.h"

#elif defined __ghs
//  Greenhills C++
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/greenhills.h"

#elif defined __BORLANDC__
//  Borland
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/borland.h"

#elif defined(__ICL) || defined(__ICC)
//  Intel
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/intel.h"

#elif defined  __MWERKS__
//  Metrowerks CodeWarrior
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/metrowerks.h"

#elif defined  __SUNPRO_CC
//  Sun Workshop Compiler C++
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/sunpro_cc.h"

#elif defined __HP_aCC
//  HP aCC
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/hp_acc.h"

#elif defined(__MRC__) || defined(__SC__)
//  MPW MrCpp or SCpp
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/mpw.h"

#elif defined(__IBMCPP__)
//  IBM Visual Age
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/vacpp.h"

#elif defined _MSC_VER
//  Microsoft Visual C++
//
//  Must remain the last #elif since some other vendors (Metrowerks, for
//  example) also #define _MSC_VER
#   define AMBULANT_COMPILER_CONFIG "ambulant/config/compiler/visualc.h"

#elif defined (AMBULANT_ASSERT_CONFIG)
// this must come last - generate an error if we don't
// recognise the compiler:
#  error "Unknown compiler - please configure and report the results to boost.org"

#endif
