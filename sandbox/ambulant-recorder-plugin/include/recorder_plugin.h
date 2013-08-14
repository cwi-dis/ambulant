/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_RECORDER_RECORDER_PLUGIN_H
#define AMBULANT_RECORDER_RECORDER_PLUGIN_H

#include "ambulant/common/factory.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "version.h"

#include "ambulant/lib/mtsync.h"	 // critical_section
#include "ambulant/net/datasource.h" // #define BASE_THREAD
#include <queue>
#include "SDL.h"
#include <stdio.h>
#include <string.h>

// This version only works with sdl_renderer and dumps BMP files for each video frame
// When AMBULANT_RECORDER_PIPE is defined, each video frame is written on stdout
// preceded by Time: (in ms), Size: (in bytes), W: and H: (both in pixels).

namespace ambulant {

namespace common {

class recorder_queue_element {

  public:
  recorder_queue_element (void* data, size_t datasize, lib::timer::time_type timestamp, lib::size window_size, const char* type) {
		m_data = data;
		m_datasize = datasize;
		m_timestamp = timestamp;
		m_window_size = window_size;
		strncpy(m_type, type, 4);
	}
	~recorder_queue_element() {
	    free(m_data);
	}

	void* m_data;
	size_t m_datasize;
	lib::timer::time_type m_timestamp;
	lib::size m_window_size;
	char m_type[4];
};

class recorder_writer : public BASE_THREAD {

  public:
	recorder_writer(FILE* pipe);
	~recorder_writer();

	void push_data (recorder_queue_element* qe);

  protected:
	long unsigned int run ();

  private:
	int _write_data (recorder_queue_element* qe);
	std::queue<recorder_queue_element*> m_queue;
	ambulant::lib::critical_section_cv m_lock;
	FILE* m_pipe;
};

class recorder_plugin : recorder {

public:
	/// Construct a new recorder to accept pixels of the given 'pixel_order'
	recorder_plugin (net::pixel_order pixel_order, lib::size& window_size);
	~recorder_plugin ();
	
	/// Record new video data with timestamp (ms) in document time
	void new_video_data (const char* data, size_t datasize, lib::timer::time_type documenttimestamp);
	/// Record new audio data with timestamp (ms) in document time
	void new_audio_data(const char* data, size_t datasize, lib::timer::timer::time_type _documentimestamp);

private:
	void new_data(const char* data, size_t datasize, lib::timer::timer::time_type _documentimestamp, const char* type);

	SDL_Surface* m_surface;
	Uint32 m_amask, m_rmask, m_gmask, m_bmask;
	bool m_dumpflag;
	FILE* m_pipe;
	lib::size m_window_size;
	recorder_writer* m_writer;
}; // class recorder_plugin

class recorder_plugin_factory : public recorder_factory {
  public:

	recorder_plugin_factory(common::factories* factories);

	~recorder_plugin_factory() {};

	recorder* new_recorder(net::pixel_order, lib::size window_size);

  private:
	factories* m_factories;

}; // class recorder_plugin_factory 

}; // namespace common

}; // namespace ambulant
#endif // AMBULANT_RECORDER_RECORDER_PLUGIN_H
