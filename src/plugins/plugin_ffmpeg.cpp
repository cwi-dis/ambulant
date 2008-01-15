// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#ifdef WITH_LIVE
#include "ambulant/net/rtsp_factory.h"
#endif
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#endif
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_factory.h"
#endif
#ifdef WITH_DSVIDEO
#include "ambulant/gui/dx/dx_dsvideo.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#ifdef WITH_DSVIDEO
class dsvideo_renderer_factory : public common::playable_factory {
  public:

	dsvideo_renderer_factory(common::factories *factory)
	:   m_factory(factory) {}
	~dsvideo_renderer_factory() {}
		
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
	{
		lib::xml_string tag = node->get_local_name();
		if (tag == "video")
			return new gui::dx::dx_dsvideo_renderer(context, cookie, node, evp, m_factory);
		return NULL;
	}
		
	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src) { return NULL; }	
		
  private:
	common::factories *m_factory;
	
};
#endif // WITH_DSVIDEO

static ambulant::common::factories * 
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif
void initialize(
    int api_version,
    ambulant::common::factories* factory,
    ambulant::common::gui_player *player)
{
    if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
        lib::logger::get_logger()->warn("ffmpeg_plugin: built for plugin-api version %d, current %d. Skipping.", 
            AMBULANT_PLUGIN_API_VERSION, api_version);
        return;
    }
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("ffmpeg_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    lib::logger::get_logger()->debug("ffmpeg_plugin: loaded.");

	// Add desired playable factories
	common::global_playable_factory *pf = factory->get_playable_factory();
    if (pf) {
#ifdef WITH_SDL
		pf->add_factory(gui::sdl::create_sdl_playable_factory(factory));
    	lib::logger::get_logger()->trace("ffmpeg_plugin: SDL playable factory registered");
#endif
#ifdef WITH_DSVIDEO
		pf->add_factory(new dsvideo_renderer_factory(factory));
    	lib::logger::get_logger()->trace("ffmpeg_plugin: video playable factory registered");
#endif
	}
	// Same for datasource foactories
	net::datasource_factory *df = factory->get_datasource_factory();
	if (df) {
#ifdef WITH_LIVE	
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add live_audio_datasource_factory");
		df->add_video_factory(net::create_live_video_datasource_factory());
		df->add_audio_factory(net::create_live_audio_datasource_factory());
		lib::logger::get_logger()->trace("ffmpeg_plugin: live555 datasource factories registered");
#endif
#ifdef WITH_FFMPEG
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add ffmpeg_video_datasource_factory");
		df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add ffmpeg_audio_datasource_factory");
		df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add ffmpeg_audio_decoder_finder");
		df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add ffmpeg_audio_filter_finder");
		df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_plugin: add ffmpeg_raw_datasource_factory");
		df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
		lib::logger::get_logger()->trace("ffmpeg_plugin: ffmpeg datasource factories registered");
#endif // WITH_FFMPEG
	}
}
