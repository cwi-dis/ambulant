/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

 
#ifndef AMBULANT_LIB_PROFILE_H
#define AMBULANT_LIB_PROFILE_H

#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"

//
// To enable profiling of specific parts of an execution, do the following:
// 1. Define one of the WITH_PROFILE_XXX defines (or create a new one, for your platform)
// 2. Make sure profile::initialize() and terminate() are called in the main program
// 3. Add profile::start() and stop() calls around the area you want to profile.
// 4. Rebuild.
//
// MacOSX CHUD profiling, uses Shark. Download the CHUD tools from the Apple
// Developer site. "System" profiling is especially interesting, it shows all thread activity
// and what caused it.

//#define WITH_PROFILE_CHUD

#ifdef WITH_PROFILE_CHUD
#include <CHUD/CHUD.h>
#endif // WITH_PROFILE_CHUD

namespace ambulant {

namespace lib {

namespace profile {

#if defined(WITH_PROFILE_CHUD)
inline void initialize() {
    chudAcquireRemoteAccess();
}

inline void terminate() {
    chudReleaseRemoteAccess();
}

inline void start() {
    chudStartRemotePerfMonitor("ambulant-profile");
}

inline void stop() {
    chudStopRemotePerfMonitor();
}
#else
inline void initialize() {
}

inline void terminate() {
}

inline void start() {
	lib::logger::get_logger()->debug("profile::start() called, but no profiler selected");
}

inline void stop() {
	lib::logger::get_logger()->debug("profile::stop() called, but no profiler selected");
}
#endif


} // end namespace profile

} // end namespace lib

} //end namespace ambulant

#endif  // AMBULANT_LIB_PROFILE_H
