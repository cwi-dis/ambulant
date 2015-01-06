// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/smil_alignment.h"

using namespace ambulant;
using namespace common;

lib::point
smil_alignment::get_image_fixpoint(lib::size image_size) const
{
	int x = m_image_fixpoint.left.get(image_size.w);
	int y = m_image_fixpoint.top.get(image_size.h);
	return lib::point(x, y);
}

lib::point
smil_alignment::get_surface_fixpoint(lib::size surface_size) const
{
	int x = m_surface_fixpoint.left.get(surface_size.w);
	int y = m_surface_fixpoint.top.get(surface_size.h);
	return lib::point(x, y);
}


