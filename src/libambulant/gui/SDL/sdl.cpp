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

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// SDL.h is needed here to solve a compile problem with gcc 4.0
#undef HAVE_ICONV
//#include "SDL.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_gui.h"
#include "ambulant/gui/SDL/sdl_audio.h"
#include "ambulant/gui/SDL/sdl_fill.h"
#include "ambulant/gui/SDL/sdl_video.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

using namespace ambulant;
using namespace gui::sdl;

common::playable_factory *
ambulant::gui::sdl::create_sdl_playable_factory(common::factories *factory)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererAudio"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new sdl_renderer_factory(factory);
}

sdl_renderer_factory::~sdl_renderer_factory()
{
	sdl_audio_renderer::quit();
}

bool
sdl_renderer_factory::supports(common::renderer_select *rs)
{
	const lib::xml_string& tag = rs->get_tag();
	if (tag != "" && tag != "ref" && tag != "audio" && tag != "brush" && tag != "video" && tag != "prefetch") return false;
	const char *renderer_uri = rs->get_renderer_uri();
#if 1
	// Stopgap for MyVideos: We don't want the SDL renderer to
	// grab prefetches, they should go to the video renderer
	if (renderer_uri == NULL && tag == "prefetch")
		return false;
#endif
	if (renderer_uri != NULL &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererSdl")) != 0 &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererOpen")) != 0 &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererAudio")) != 0  &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererVideo")) != 0 &&
		strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererFill")) != 0 )
	{
		return false;
	}
	return true;
}

common::playable *
sdl_renderer_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv = NULL;
	lib::xml_string tag = node->get_local_name();
	AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory: node 0x%x:	inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio" || tag == "prefetch") {
		rv = new gui::sdl::sdl_audio_renderer(context, cookie, node, evp, m_factory, (common::playable_factory_machdep*)NULL);
		AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory: node 0x%x: %s returning sdl_audio_renderer 0x%x", (void *)node, node->get_sig().c_str(), (void *)rv);
#ifdef WITH_SDL_IMAGE
	} else if (tag == "video") {
		rv = new gui::sdl::sdl_video_renderer(context, cookie, node, evp, m_factory, (common::playable_factory_machdep*)NULL);
		AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory: node 0x%x: %s returning sdl_video_renderer 0x%x", (void *)node, node->get_sig().c_str(), (void *)rv);
#endif // WITH_SDL_IMAGE
	}
	return rv;
}

common::playable*
sdl_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	common::playable *rv;
	lib::xml_string tag = node->get_local_name();
	AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory:new_aux_audio_playable: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	rv = new gui::sdl::sdl_audio_renderer(context, cookie, node, evp, m_factory, src);
	AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory: node 0x%x: returning sdl_audio_renderer 0x%x", (void *)node, (void *)rv);
	return rv;
}
