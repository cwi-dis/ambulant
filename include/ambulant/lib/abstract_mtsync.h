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

#ifndef AMBULANT_LIB_ABSTRACT_MTSYNC_H
#define AMBULANT_LIB_ABSTRACT_MTSYNC_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {

/// API for the main synchronisation primitive.
/// Do not use this class directly, in stead allocate
/// objects of class critical section.
class AMBULANTAPI base_critical_section {
  public:
	virtual ~base_critical_section() {}

	/// Enter a critical section.
	virtual void enter() = 0;

	/// Leave a critical section.
	virtual void leave() = 0;
};

class AMBULANTAPI base_critical_section_cv {
  public:
	virtual ~base_critical_section_cv() {};

	virtual void signal() = 0;
	virtual bool wait(int microseconds = -1) = 0;
};

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_ABSTRACT_MTSYNC_H
