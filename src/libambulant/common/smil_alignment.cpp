/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/logger.h"
#include "ambulant/common/smil_alignment.h"

using namespace ambulant;
using namespace common;

// Helper function: decode pre-defined repoint names
bool
decode_regpoint(regpoint_spec &pt, const char *name)
{
	if (strcmp(name, "topLeft") == 0) pt = regpoint_spec(0.0, 0.0);
	else if (strcmp(name, "topMid") == 0) pt = regpoint_spec(0.5, 0.0);
	else if (strcmp(name, "topRight") == 0) pt = regpoint_spec(1.0, 0.0);
	else if (strcmp(name, "midLeft") == 0) pt = regpoint_spec(0.0, 0.5);
	else if (strcmp(name, "center") == 0) pt = regpoint_spec(0.5, 0.5);
	else if (strcmp(name, "midRight") == 0) pt = regpoint_spec(1.0, 0.5);
	else if (strcmp(name, "bottomLeft") == 0) pt = regpoint_spec(0.0, 1.0);
	else if (strcmp(name, "bottomMid") == 0) pt = regpoint_spec(0.5, 1.0);
	else if (strcmp(name, "bottomRight") == 0) pt = regpoint_spec(1.0, 1.0);
	else
		return false;
	return true;
}

alignment *
smil_alignment::create_for_dom_node(const lib::node *n)
{
	const char *regPoint = n->get_attribute("regPoint");
	const char *regAlign = n->get_attribute("regAlign");
	if (regPoint == NULL && regAlign == NULL) return NULL;
	return new smil_alignment(n, regPoint, regAlign);
}

smil_alignment::smil_alignment(const lib::node *n, const char *regPoint, const char *regAlign)
{
	if (!decode_regpoint(m_image_fixpoint, regAlign))
		lib::logger::get_logger()->error("smil_alignment: unknown regAlign value: %s", regAlign);
	if (!decode_regpoint(m_surface_fixpoint, regPoint))
		lib::logger::get_logger()->error("smil_alignment: only standard regPoint values implemented", regAlign);
}

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
