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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gstreamer/gstreamer_renderer_factory.h"
#include "ambulant/gui/gstreamer/gstreamer_audio_renderer.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
using namespace gui::gstreamer;

common::playable_factory *
ambulant::gui::gstreamer::create_gstreamer_renderer_factory(common::factories *factory)
{
	return new gstreamer_renderer_factory(factory);
}

bool
gstreamer_renderer_factory::supports(common::renderer_select *rs)
{
	const lib::xml_string& tag = rs->get_tag();
	if (tag != "" && tag != "ref" && tag != "audio" && tag != "prefetch") return false;
	const char *renderer_uri = rs->get_renderer_uri();
	if (renderer_uri != NULL &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererGstreamer")) != 0 &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererOpen")) != 0 &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererAudio")) != 0 )
	{
		return false;
	}
	return true;
}

// ***************** gstreamer_renderer_factory ************************

gstreamer_renderer_factory::~gstreamer_renderer_factory()
{
}

common::playable *
gstreamer_renderer_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	common::playable *rv = NULL;
	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: node 0x%x:	inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio" && gui::gstreamer::gstreamer_audio_renderer::is_supported(node)) {
		rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory, (common::playable_factory_machdep*)NULL);
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: no GStreamer renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}

common::playable*
gstreamer_renderer_factory::new_aux_audio_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::audio_datasource *src)
{
	common::playable *rv = NULL;
	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: node 0x%x:	inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio" && gui::gstreamer::gstreamer_audio_renderer::is_supported(node)) {
		rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory, src);
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: no GStreamer renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}

