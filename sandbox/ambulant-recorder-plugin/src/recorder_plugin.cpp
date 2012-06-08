// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

/*
 * recorder_plugin.cpp -- implements an Ambulant recorder to produce audio/video streams
 *                        from audio packets/video frames recieved from clients
 *
 * The client initilizes the plugin appropriately using initialize* methods
 * and subsequently repeatedly calls new_packet(...) and/or new_frame(...)
 * to produce the desired output streams(s)
 */


#include "recorder_plugin.h"
#include "ambulant/gui/SDL/sdl_video.h" // for SDL_BPP

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant::common;
using namespace ambulant::lib;

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif

void initialize(int api_version, factories* factories, gui_player *player)
{
	if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
		logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"xerces_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"xerces_plugin", AMBULANT_VERSION);
	}
	AM_DBG logger::get_logger()->debug("recorder_plugin::initialize registering factory function");
	recorder_plugin_factory* rpf = new recorder_plugin_factory(factories);
        factories->set_recorder_factory(rpf);
	logger::get_logger()->trace("recorder_plugin: registered");
}

// class recorder_plugin_factory impl.

recorder_plugin_factory::recorder_plugin_factory (common::factories* factories)
  :	recorder_factory(),
	m_factories(factories)
{
}

recorder*
recorder_plugin_factory::new_recorder(net::pixel_order pixel_order, lib::size window_size)
{
	recorder_plugin* result = NULL;

	switch (pixel_order) {
	case net::pixel_argb:
		result = new recorder_plugin(pixel_order, window_size);
		break;
	default:
		break;
	}
	return (recorder*) result;
}

// class recorder_plugin impl.
	
// Construct a new recorder to accept pixels of the given 'pixel_order'
recorder_plugin::recorder_plugin (net::pixel_order pixel_order, lib::size window_size)
{
	assert (pixel_order == net::pixel_argb);
	switch (pixel_order) {
	case net::pixel_argb:
		m_amask = 0xFF000000;
		m_rmask = 0x00FF0000;
		m_gmask = 0x0000FF00;
		m_rmask = 0x000000FF;
		m_surface = SDL_CreateRGBSurface (0, window_size.w, window_size.h, 32,
						  m_rmask, m_bmask, m_gmask, m_amask);
		break;
	default:
		break;
	}
}

recorder_plugin::~recorder_plugin ()
{
	if (m_surface != NULL) {
		SDL_FreeSurface(m_surface);
	}
}
	
// Record new video data with timestamp (ms) in document time
void
recorder_plugin::new_video_data (void* data, size_t datasize, lib::timer::time_type documenttimestamp)
{
	if (data == NULL || datasize == 0 || m_surface == NULL) {
		return;
	}
	char filename[256];
	sprintf(filename,"%%%0.8lu.bmp", documenttimestamp);
	SDL_SaveBMP(m_surface, filename);
}
