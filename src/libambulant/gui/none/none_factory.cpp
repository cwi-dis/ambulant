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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/config/config.h"
#include "ambulant/gui/none/none_factory.h"
#include "ambulant/gui/none/none_video_renderer.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"


using namespace ambulant;
using namespace gui::none;

common::playable_factory *
ambulant::gui::none::create_none_video_factory(common::factories *factory)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererNone"), true);
	return new none_video_factory(factory);
}

none_video_factory::~none_video_factory()
{
}

bool
none_video_factory::supports(common::renderer_select *rs)
{
	const lib::xml_string& tag = rs->get_tag();
	if (tag != "" && tag != "ref" && tag != "video") return false;
	const char *renderer_uri = rs->get_renderer_uri();
	if (renderer_uri && strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererNone")) == 0)
		return true;
	return false;
}

common::playable *
none_video_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	lib::xml_string tag = node->get_local_name();
	AM_DBG lib::logger::get_logger()->debug("none_video_factory: node 0x%x:	  inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "video") {
		rv = new gui::none::none_video_renderer(context, cookie, node, evp, m_factory, NULL);
		AM_DBG lib::logger::get_logger()->debug("none_video_factory: node 0x%x: returning none_video_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("none_video_factory: no renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}

common::playable *
none_video_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}

