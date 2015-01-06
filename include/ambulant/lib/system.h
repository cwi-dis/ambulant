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

#ifndef AMBULANT_LIB_SYSTEM_H
#define AMBULANT_LIB_SYSTEM_H

#include "ambulant/config/config.h"
#include "ambulant/net/url.h"
#include <string>

namespace ambulant {

namespace lib {

/// Baseclass for embedder that will implement external commands.
class AMBULANTAPI system_embedder {
  public:
	virtual ~system_embedder() {}

	/// Open the given URL with an external program (such as a web browser).
	virtual void show_file(const net::url& href) = 0;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_SYSTEM_H
