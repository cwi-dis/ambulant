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
recorder_plugin::recorder_plugin (net::pixel_order pixel_order, lib::size& window_size)
  : m_pipe(NULL),
    m_surface(NULL),
    m_window_size(window_size)
{
	const char* fun = __PRETTY_FUNCTION__;

	switch (pixel_order) {
	case net::pixel_argb:
		m_amask = 0xFF000000;
		m_rmask = 0x00FF0000;
		m_gmask = 0x0000FF00;
		m_bmask = 0x000000FF;
		break;
	case net::pixel_rgba:
		m_rmask = 0xFF000000;
		m_gmask = 0x00FF0000;
		m_bmask = 0x0000FF00;
		m_amask = 0x000000FF;
		break;
	default:
		logger::get_logger()->trace("%s: unimplemented 'pixel_order' %d)", fun, pixel_order);
		break;
	}
	char* command = getenv("AMBULANT_RECORDER_PIPE");
	if (command != NULL) {
		m_pipe = popen(command, "w");
		if (m_pipe == NULL) {
			logger::get_logger()->trace("%s: pipe failed: %s)", fun,strerror(errno));
		}
	}
}

recorder_plugin::~recorder_plugin ()
{
	const char* fun = __PRETTY_FUNCTION__;

	if (m_pipe != NULL) {
		pclose (m_pipe);
	}
	if (m_surface != NULL) {
		SDL_FreeSurface(m_surface);
	}
}

void*
convert_bgra_to_rgb(void* data, size_t datasize, size_t* new_datasize)
{
	int length = ( datasize/4 ) * 3;
	int rgb_idx = length - 1;
	int bgra_idx = datasize - 1;
	char* rgb_buffer = (char*) malloc(length);
	char* bgra_buffer = (char*) data;
	if (data != NULL) {
	  for (; bgra_idx >= 0; bgra_idx -= 4) {
	    rgb_buffer[rgb_idx--] = bgra_buffer[bgra_idx - 3];
	    rgb_buffer[rgb_idx--] = bgra_buffer[bgra_idx - 2];
	    rgb_buffer[rgb_idx--] = bgra_buffer[bgra_idx - 1];
	  }
	}
	if (new_datasize != NULL) {
	  *new_datasize = length;
	}
	return rgb_buffer;
}
	
// Record new video data with timestamp (ms) in document time
void
recorder_plugin::new_video_data (void* data, size_t datasize, lib::timer::time_type documenttimestamp)
{
	const char* fun = __PRETTY_FUNCTION__;

	if (data == NULL || datasize == 0) {
		return;
	}
	if (m_pipe != NULL) {
		unsigned int new_datasize;
		void* new_data = convert_bgra_to_rgb (data, datasize, &new_datasize);
		fprintf(m_pipe, "Time: %0.8u\nSize: %.8u\nW: %5u\nH: %5u\n", documenttimestamp, new_datasize, m_window_size.w, m_window_size.h);
		fwrite (new_data, 1, new_datasize, m_pipe);
		free (new_data);
	} else {
		if (m_surface) {
			SDL_FreeSurface(m_surface);
		}
		m_surface = SDL_CreateRGBSurface (0, m_window_size.w, m_window_size.h, 32,
						  m_rmask, m_gmask, m_bmask, m_amask);
		if (m_surface == NULL) {
			logger::get_logger()->trace("%s: SDL_CreateSurface failed: %s)", fun, SDL_GetError());
		}
	  
		char filename[256];
		sprintf(filename,"%%%0.8lu.bmp", documenttimestamp);
		SDL_SaveBMP(m_surface, filename);
	}
}
