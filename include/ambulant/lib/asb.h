/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "ambulant/config/config.h"

#ifdef AMBULANT_PLATFORM_WIN32
#include "ambulant/lib/win32/win32_asb.h"
#else
#include <unistd.h>
#endif

namespace ambulant {

namespace lib {

#ifdef AMBULANT_PLATFORM_WIN32

using ambulant::lib::win32::sleep;
using ambulant::lib::win32::sleep_msec;

#else

using ::sleep;
inline void sleep_msec(unsigned long msecs) { (void)usleep(msecs*1000); }
#endif

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_ASB_H
