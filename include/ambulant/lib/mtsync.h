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

#ifndef AMBULANT_LIB_MTSYNC_H
#define AMBULANT_LIB_MTSYNC_H

#if defined(AMBULANT_PLATFORM_WIN32)
#include "ambulant/lib/win32/win32_mtsync.h"
#else
#include "ambulant/lib/unix/unix_mtsync.h"
#endif

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {

/// A mutex.
/// The actual implementation is machine-dependent and
/// implements the base_critical_section interface.
///
/// The reason for the slightly convoluted implementation
/// in stead of a normal pattern is to allow allocation
/// of critical sections without using the new operator.
///
/// Critical sections are not reentrant from the same thread
/// (nor from other threads, obviously), and there is no
/// deadlock detection (so you are responsible that there are
/// no situations where code can sometimes first enter CS 1
/// and then CS 2 and other paths through the code where first
/// CS 2 is entered and then CS 1).
#ifdef AMBULANT_PLATFORM_WIN32
class AMBULANTAPI critical_section : public win32::critical_section {
};
#else
class AMBULANTAPI critical_section : public unix::critical_section {
};
#endif

/// A mutex plus a condition variable.
/// Extends the critical_section class with signal and wait primitives.
#ifdef AMBULANT_PLATFORM_WIN32
class AMBULANTAPI critical_section_cv : public win32::critical_section_cv {
};
#else
class AMBULANTAPI critical_section_cv : public unix::critical_section_cv {
};
#endif

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_MTSYNC_H
