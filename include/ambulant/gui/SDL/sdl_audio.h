/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#ifndef __SDL_AUDIO__
#define __SDL_AUDIO__

#include <SDL.h>
#include <iostream>

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/event_processor.h"
#ifdef USE_SMIL21
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/transition_info.h"
#endif
#include "ambulant/lib/asb.h"


#define SDL_BUFFER_MAX_BYTES 819200
#define SDL_BUFFER_MIN_BYTES 20480

#define AMBULANT_MAX_CHANNELS 2
 
namespace ambulant {
namespace gui {
namespace sdl {	  

class sdl_audio_renderer : public common::renderer_playable {

  public:
	sdl_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory);

	sdl_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	net::audio_datasource *ds);
  
	~sdl_audio_renderer();

	  
	bool is_paused();
	bool is_stopped();
	bool is_playing();
	
	common::duration get_dur();
	void start(double where);
	void stop();
	void seek(double t);
	void pause();
	void resume();
//	void freeze() {};
//	void speed_changed() {};

//	void set_surface(common::surface *dest) { abort(); }
//	common::surface *get_surface() { abort(); }
//	void set_alignment(common::alignment *align) { /* Ignore, for now */ }
//	void transition_freeze_end(lib::rect area) {}		  
	void redraw(const lib::rect &dirty, common::gui_window *window) {}
	static void sdl_callback(Uint8 *stream, int len);
#ifdef USE_SMIL21
	void set_intransition(const lib::transition_info* info);
	void start_outtransition(const lib::transition_info* info);
#else
	void set_intransition(const lib::transition_info* info) {}
	void start_outtransition(const lib::transition_info* info) {}
#endif
  private:
   	void data_avail();
	bool restart_audio_input();
	int get_data(int bytes_wanted, Uint8 **ptr);
	void get_data_done(int size);
	net::audio_datasource *m_audio_src;
	lib::critical_section m_lock;
	
	bool m_is_playing;
	bool m_is_paused;
  	bool m_read_ptr_called;
	bool m_audio_started;
	int m_volcount;
	float m_volumes[AMBULANT_MAX_CHANNELS];
#ifdef USE_SMIL21
	const lib::transition_info* m_intransition;
	const lib::transition_info* m_outtransition;
	smil2::audio_transition_engine* m_transition_engine;
#endif
	// class methods and attributes:
	static int init();
 	static void register_renderer(sdl_audio_renderer *rnd);
 	static void unregister_renderer(sdl_audio_renderer *rnd);
    static bool s_sdl_init;			// True when SDL has beeen initialized
	static Uint16 s_sdl_format;		// Audio format we want, in SDL terms
	static net::audio_format s_ambulant_format;	// Audio format we want, in Ambulant terms
    static int s_buffer_size;		// Number of samples we want SDL to process at once
	static int s_min_buffer_size_bytes;		// Minimum number of bytes we need for the abobe
	static lib::critical_section s_static_lock;
	static std::list<sdl_audio_renderer *>s_renderers;
};

} // end namespace sdl
} // end namespace gui
} // end namespace ambulant


#endif
