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

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/none/none_area.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;
using namespace lib;
using namespace common;

common::window_factory *
ambulant::gui::none::create_none_window_factory()
{
	return new none_window_factory();
}

gui::none::none_playable::none_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	common::playable_imp(context, cookie, node, evp, NULL, NULL)
{
	lib::xml_string tag = node->get_local_name();
	std::string url = repr(node->get_url("src"));
	lib::logger::get_logger()->warn("No renderer found for <%s src=\"%s\">, using none_playable", tag.c_str(), url.c_str());
}

void
gui::none::none_playable::start(double where)
{
	lib::logger::get_logger()->trace("none_playable.start(0x%x)", m_node);
	m_context->stopped(m_cookie, 0);
}

bool
gui::none::none_playable::stop()
{
	lib::logger::get_logger()->trace("none_playable.stop(0x%x)", (void *)this);
	return true; // Don't re-use this renderer
}

void
gui::none::none_background_renderer::redraw(const rect &dirty, gui_window *window)
{
	lib::logger::get_logger()->trace("none_background_renderer.redraw(0x%x) from 0x%x to 0x%x", (void *)this, (void*)m_src, (void*)m_dst);
}

void
gui::none::none_background_renderer::highlight(gui_window *window)
{
	lib::logger::get_logger()->trace("none_background_renderer.highlight(0x%x) from 0x%x to 0x%x", (void *)this, (void*)m_src, (void*)m_dst);
}

void
gui::none::none_background_renderer::keep_as_background()
{
	lib::logger::get_logger()->trace("none_background_renderer.keep_as_background(0x%x)", (void *)this);
}

playable *
gui::none::none_playable_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	lib::xml_string tag = node->get_local_name();
	if(tag == "area" || tag == "a")
		return new none_area_renderer(context, cookie, node, evp, NULL, NULL);
	return new none_playable(context, cookie, node, evp, NULL, NULL);
}

common::playable *
gui::none::none_playable_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}

bgrenderer *
gui::none::none_window_factory::new_background_renderer(const region_info *src)
{
	return new none_background_renderer(src);
}

gui_window *
gui::none::none_window_factory::new_window(const std::string &name, size bounds, gui_events *handler)
{
	return new none_window(name, bounds, handler);
}
