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

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/none/none_area.h"
#include "ambulant/common/renderer.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;
using namespace lib;
using namespace common;

gui::none::none_playable::none_playable(
	common::playable_notification *context,
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	// Workaround for bug in emVC 4.0: it gets confused
	// when getting a subtype from a class within a function
	// signature, or something like that
	int cookie,
#else
	common::playable_notification::cookie_type cookie,
#endif
	const lib::node *node,
	lib::event_processor *evp)
:	common::playable_imp(context, cookie, node, evp)
{
	lib::xml_string tag = node->get_qname().second;
	std::string url = repr(node->get_url("src"));
	lib::logger::get_logger()->warn("No renderer found for <%s src=\"%s\">, using none_playable", tag.c_str(), url.c_str());
}

void
gui::none::none_playable::start(double where)
{
	lib::logger::get_logger()->trace("none_playable.start(0x%x)", m_node);
	m_context->stopped(m_cookie, 0);
}

void
gui::none::none_playable::stop()
{
	lib::logger::get_logger()->trace("none_playable.stop(0x%x)", (void *)this);
}

void
gui::none::none_background_renderer::redraw(const screen_rect<int> &dirty, gui_window *window)
{
	lib::logger::get_logger()->trace("none_background_renderer.redraw(0x%x) from 0x%x to 0x%x", (void *)this, (void*)m_src, (void*)m_dst);
}

playable *
gui::none::none_playable_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	lib::xml_string tag = node->get_qname().second;
	if(tag == "area" || tag == "a")
		return new none_area_renderer(context, cookie, node, evp);
	return new none_playable(context, cookie, node, evp);
}

bgrenderer *
gui::none::none_window_factory::new_background_renderer(region_info *src)
{
	return new none_background_renderer(src);
}

gui_window *
gui::none::none_window_factory::new_window(const std::string &name, size bounds, gui_events *handler)
{
	return new none_window(name, bounds, handler);
}
