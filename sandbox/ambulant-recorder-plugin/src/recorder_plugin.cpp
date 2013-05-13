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

//#define AM_DBG if(1)
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
  : m_dumpflag(false),
    m_pipe(NULL),
    m_surface(NULL),
    m_window_size(window_size),
    m_writer(NULL)
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
		if (strcmp(command, "dump") == 0) {
			m_dumpflag = true;
		} else {
			m_pipe = popen(command, "w");
			if (m_pipe == NULL) {
				logger::get_logger()->trace("%s: pipe failed: %s)", fun,strerror(errno));
			}
			m_writer = new recorder_writer (m_pipe);
			m_writer->start();
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
	
// Record new video data with timestamp (ms) in document time
void
recorder_plugin::new_video_data (const char* data, size_t datasize, lib::timer::time_type documenttimestamp)
{
	const char* fun = __PRETTY_FUNCTION__;

	if (data == NULL || datasize == 0) {
		return;
	}
	if (m_pipe != NULL) {
		unsigned long int checksum;
		void* new_data = malloc (datasize); memcpy (new_data, data, datasize);
		m_writer->push_data (new recorder_queue_element(new_data, datasize, documenttimestamp, m_window_size, checksum));
	} else if (m_dumpflag) {
		if (m_surface) {
			SDL_FreeSurface(m_surface);
		}
		m_surface = SDL_CreateRGBSurfaceFrom ((void*) data, m_window_size.w, m_window_size.h, 32, m_window_size.w * 4,
						  m_rmask, m_gmask, m_bmask, m_amask);
		if (m_surface == NULL) {
			logger::get_logger()->trace("%s: SDL_CreateSurface failed: %s)", fun, SDL_GetError());
		}
	  
		char filename[256];
		sprintf(filename,"%%%.8lu.bmp", documenttimestamp);
		SDL_SaveBMP(m_surface, filename);
	}
}

// class recorder_writer implementation.

recorder_writer::recorder_writer(FILE* pipe)
{
	const char* fun = __PRETTY_FUNCTION__;
	AM_DBG ambulant::lib::logger::get_logger()->debug("%s(%p)", fun, this);
	m_pipe = pipe;
}

recorder_writer::~recorder_writer() {
	const char* fun = __PRETTY_FUNCTION__;
	AM_DBG ambulant::lib::logger::get_logger()->debug("%s(%p)", fun, this);
	m_lock.enter();
	while (m_queue.size() > 0) {
	  recorder_queue_element* qe = m_queue.front();
	  delete qe;
	  m_queue.pop();
	}
	m_lock.leave();  
}

void
recorder_writer::push_data(recorder_queue_element* qe)
{
	const char* fun = __PRETTY_FUNCTION__;
	static lib::timer::time_type s_old_timestamp = 0;
	lib::timer::time_type diff =  qe->m_timestamp - s_old_timestamp;
	m_lock.enter();
	bool drop_frame = diff < 30;
// add CXXFLAGS=-DFRAME_DELAY_DEBUG in './configue' for frame delay debugging					  
#ifdef FRAME_DELAY_DEBUG
	/*AM_DBG*/
#else
	AM_DBG
#endif//FRAME_DELAY_DEBUG  
	  ambulant::lib::logger::get_logger()->debug("%s%p(qe=%p time=%ld diff=%ld drop_frame=%d data=0x%x)", fun, this, qe, qe->m_timestamp, diff, drop_frame, *(unsigned int**) qe->m_data);
	if ( ! drop_frame) {
		s_old_timestamp = qe->m_timestamp;
		m_queue.push (qe);
	} else {
		delete qe;
	}
	m_lock.leave();
}

// Only for debugging. Code from: unix_timer.cpp
#include <sys/time.h>
// return current time in millisec. since Epoch (unix: man 2 time)
ambulant::lib::timer::time_type clock_time() {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) >= 0) {
		return tv.tv_sec*1000 + tv.tv_usec / 1000;
	}
	return 0;
}

int
recorder_writer::_write_data (recorder_queue_element* qe)
{
	const char* fun = __PRETTY_FUNCTION__;
	size_t result = 0;
	static lib::timer::time_type s_old_timestamp = 0;
	lib::timer::time_type start_time = clock_time();

	AM_DBG ambulant::lib::logger::get_logger()->debug("%s%p timestamp=%ld, (diff=%ld) data=0x%x", fun, this, qe->m_timestamp, qe->m_timestamp - s_old_timestamp, *(void**)qe->m_data); // enable for frame delay debugging
	s_old_timestamp = qe->m_timestamp;
	if (fprintf(m_pipe, "Time: %.8lu\nSize: %.8lu\nW: %5u\nH: %5u\nChksm: %.24lx\n", qe->m_timestamp, qe->m_datasize, qe->m_window_size.w, qe->m_window_size.h, qe->m_checksum) < 0) {
		return -1;
	}
	result = fwrite (qe->m_data, 1, qe->m_datasize, m_pipe);
	fflush(m_pipe);
	size_t datasize = qe->m_datasize;
	delete qe;
	if (result != datasize) {
		return -1;
	}
	AM_DBG ambulant::lib::logger::get_logger()->debug("%s%p wrote: (qe=%p time=%ld in %ld millisec)", fun, this, qe, qe->m_timestamp, clock_time() - start_time);
	return 0;
}

unsigned long int
recorder_writer::run()
{
	const char* fun = __PRETTY_FUNCTION__;
	AM_DBG ambulant::lib::logger::get_logger()->debug("%s(%p)", fun, this);

	while ( ! exit_requested() ) {
		m_lock.enter();
		if (m_queue.size() > 0) {
			recorder_queue_element* qe = m_queue.front();
			m_queue.pop();
			m_lock.leave();
			if (_write_data(qe) < 0) {
				terminate();
			}
		} else m_lock.leave();
		ambulant::lib::sleep_msec(10);
	}

}

