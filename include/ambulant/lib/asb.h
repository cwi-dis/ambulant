
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

// This header defines and exports  
// a set of handy not std functions.
// All functions defined here are accessible
// through the lib namespace.
// For example to invoke on unix or windows 
// sleep(secs) use:
// lib::sleep(secs); // within ambulant ns

#ifndef AMBULANT_LIB_ASB_H
#define AMBULANT_LIB_ASB_H

#ifdef WIN32
#include "ambulant/lib/win32/win32_asb.h"
#else
#include <unistd.h>
#endif

namespace ambulant {

namespace lib {

#ifdef WIN32

using ambulant::lib::win32::sleep;
using ambulant::lib::win32::sleep_msec;
const std::string file_separator = "\\/";

#else

using ::sleep;
const std::string file_separator = "/";

#endif

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_ASB_H
