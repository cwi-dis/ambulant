/*
 *
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003 Stiching CWI,
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 *
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 *
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception.
 *
 */

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/SDL/sdl_audio.h"
#include "ambulant/net/posix_datasource.h"
#include <stdlib.h>

using namespace ambulant;
//using namespace gui::sdl;

typedef lib::no_arg_callback<gui::sdl::sdl_active_audio_renderer> readdone_callback;
	
extern "C" {

// XXXX Need to lock the channel list with a mutex, probably...

struct channel {
	int m_channel_nr;
	bool m_free;
	void* m_ptr;
 };
 
 struct channel_list {
	 int m_channels_open;
	 channel* m_list;
 };
 
channel_list m_channel_list;

void dump_channel_list() {
int i; 
for(i=0;i < m_channel_list.m_channels_open; i++) {
		std::cout << m_channel_list.m_list[i].m_channel_nr  << " , " <<
		m_channel_list.m_list[i].m_free << " , " <<
		m_channel_list.m_list[i].m_ptr << std::endl;
	}
}
 
void init_channel_list(int nr_channels)
{
	int i;
	
	m_channel_list.m_channels_open = nr_channels;
	m_channel_list.m_list = (channel*) malloc(nr_channels*sizeof(channel)); 
	for(i=0;i < m_channel_list.m_channels_open; i++) {
		m_channel_list.m_list[i].m_channel_nr = i;
		m_channel_list.m_list[i].m_free = true;
		m_channel_list.m_list[i].m_ptr = NULL;
	}
}


// returns 0 on succes and -1 on a memory allocation error.
int resize_channel_list(int nr_channels)
{
	channel* ptr;
	int i;
	
	ptr = (channel*) realloc(m_channel_list.m_list, nr_channels*sizeof(channel));
	if (ptr) {
		m_channel_list.m_list = ptr;
		if (nr_channels > m_channel_list.m_channels_open) {
			for(i = m_channel_list.m_channels_open; i < nr_channels; i++) {
				m_channel_list.m_list[i].m_channel_nr = i;
				m_channel_list.m_list[i].m_free = true;
				m_channel_list.m_list[i].m_ptr = NULL;
			}
		}
		m_channel_list.m_channels_open = nr_channels;
		return 0;
	}
	return -1;
}

void free_channel_list()
{
	free(m_channel_list.m_list);
	m_channel_list.m_list = NULL;
	m_channel_list.m_channels_open = 0;
}

// returns zero if succesfull, -1 if the channel is alreay locked, -2 if channel_nr is to big.
int lock_channel(void* ptr, int channel_nr)
{
	AM_DBG lib::logger::get_logger()->trace("sdl_audio:lock_channel(0x%x, %d)", ptr, channel_nr);
	if(channel_nr < m_channel_list.m_channels_open) {
		if (m_channel_list.m_list[channel_nr].m_free == false) {
				lib::logger::get_logger()->error("sdl_audio:lock_channel(0x%x, %d): not free, m_ptr=0x%x", 
					ptr, channel_nr, m_channel_list.m_list[channel_nr].m_ptr);
				return -1;
		}
		m_channel_list.m_list[channel_nr].m_free = false;
		m_channel_list.m_list[channel_nr].m_ptr = ptr;
	} else {
		lib::logger::get_logger()->error("sdl_audio:lock_channel(0x%x, %d): channel list full");
		return -2;
	}
	return 0;
}

int unlock_channel(int channel_nr)
{
	AM_DBG lib::logger::get_logger()->trace("sdl_audio:unlock_channel(%d)", channel_nr);
	if(channel_nr < m_channel_list.m_channels_open) {
		m_channel_list.m_list[channel_nr].m_free = true;
		m_channel_list.m_list[channel_nr].m_ptr = NULL;
	} else {
		lib::logger::get_logger()->error("sdl_audio:unlock_channel(%d): only %d channels open",
			channel_nr, m_channel_list.m_channels_open);
		return -2;
	}
	return 0;
}

// Returns pointer to an sdl_active_audio_renderer, or NULL incase the channel is not in use or does not exist.
void* get_ptr(int channel_nr)
{
	if(channel_nr < m_channel_list.m_channels_open) {
		if (m_channel_list.m_list[channel_nr].m_free == true) {
				AM_DBG lib::logger::get_logger()->trace("sdl_audio:get_ptr(%d): channel not in use!", channel_nr);
				return NULL;
		}
		return m_channel_list.m_list[channel_nr].m_ptr;
	}
	lib::logger::get_logger()->error("sdl_audio:get_ptr(%d): only %d channels!", channel_nr, m_channel_list.m_channels_open);
	return NULL;
}

// returns a channel number and locks it or returns -1 if there are no free channels or the channel does not exist.
int free_channel()
{
	int i;
	for(i=0;i < m_channel_list.m_channels_open; i++) {
		if(m_channel_list.m_list[i].m_free == true) {
			AM_DBG lib::logger::get_logger()->trace("sdl_audio:free_channel(): returning %d", m_channel_list.m_list[i].m_channel_nr);
			return m_channel_list.m_list[i].m_channel_nr;
		}
	}
	AM_DBG lib::logger::get_logger()->trace("sdl_audio:free_channel(): all %d channels in use", m_channel_list.m_channels_open);
	return -1;
}

 

void channel_done(int channel)
{
        gui::sdl::sdl_active_audio_renderer* object;
        object = (gui::sdl::sdl_active_audio_renderer*) get_ptr(channel);
		if (object == NULL) {
			lib::logger::get_logger()->error("sdl_audio:channel_done(%d): get_ptr returned NULL, no object!", channel);
			return;
		}
        object->playdone();
}

} //end extern "C"
	
// ************************************************************

bool gui::sdl::sdl_active_audio_renderer::m_sdl_init = false;
int	 gui::sdl::sdl_active_audio_renderer::m_mixed_channels = 0;
Uint16 gui::sdl::sdl_active_audio_renderer::m_sdl_format = AUDIO_S16SYS;
net::audio_format gui::sdl::sdl_active_audio_renderer::m_ambulant_format = net::audio_format(44100, 2, 16);
int gui::sdl::sdl_active_audio_renderer::m_buffer_size = 4096;    
lib::critical_section gui::sdl::sdl_active_audio_renderer::m_static_lock;    

int
gui::sdl::sdl_active_audio_renderer::init()
{
	m_static_lock.enter();
	if (m_sdl_init) {
		m_static_lock.leave();
		return 0;
	}
    int err = 0;
	
	// XXXX Should check that m_ambulant_format and m_sdl_format match!
	
	// Step one - initialize the SDL library
	err = SDL_Init(SDL_INIT_AUDIO| SDL_INIT_NOPARACHUTE);
	if (err < 0) {
		lib::logger::get_logger()->error("sdl_active_audio_renderer.init: SDL_Init failed: error %d", err);
		m_static_lock.leave();
		return err;
	}
	
	// Step two - initialize the SDL Mixer library and our datastructures to
	// interface to it
	m_mixed_channels=16;
	err = Mix_AllocateChannels(m_mixed_channels);
	Mix_ChannelFinished(channel_done);
	init_channel_list(m_mixed_channels);

	// Step three - open the mixer
	err = Mix_OpenAudio(m_ambulant_format.samplerate, m_sdl_format, m_ambulant_format.channels, m_buffer_size);
	if (err < 0) {
		lib::logger::get_logger()->error("sdl_active_renderer.init: Mix_OpenAudio failed: error %d", err);
		m_static_lock.leave();
    	return err;
	}
	
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer.init: SDL init succes");			
	m_sdl_init = true;
	m_static_lock.leave();
	return err;
}

// ************************************************************

gui::sdl::sdl_active_audio_renderer::sdl_active_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::datasource_factory *df)
:	common::active_basic_renderer(context, cookie, node, evp),
	m_audio_src(NULL),
	m_audio_chunk_busy(false),
	m_channel_used(-1)
{
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::sdl_active_audio_renderer() this=(x%x)",  this);
	if (init() != 0)
		return;
		
	net::audio_format_choices supported = net::audio_format_choices(m_ambulant_format);
	std::string url = node->get_url("src");
	m_audio_src = df->new_audio_datasource(url, supported);
	if (!m_audio_src)
		lib::logger::get_logger()->error("sdl_active_audio_renderer: cannot open %s", url.c_str());
	if (!supported.contains(m_audio_src->get_audio_format())) {
		lib::logger::get_logger()->error("sdl_active_audio_renderer: %s: unsupported format", url.c_str());
		m_audio_src->release();
		m_audio_src = NULL;
	}
}

gui::sdl::sdl_active_audio_renderer::~sdl_active_audio_renderer()
{
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::~sdl_active_audio_renderer() this=(x%x)",  this);		
	if (m_audio_src) m_audio_src->release();
	m_audio_src = NULL;
}

int
gui::sdl::sdl_active_audio_renderer::inc_channels()
{
	m_static_lock.enter();
	m_mixed_channels += 16;
	int err;
	err = Mix_AllocateChannels(m_mixed_channels);
	err = resize_channel_list(m_mixed_channels); // XXXX Jack thinks the treatment of err is suspect
	m_static_lock.leave();
	return err;
}


void
gui::sdl::sdl_active_audio_renderer::playdone()
{
	// XXXX Jack is not sure about this m_lock.enter(): playdone() is called by the SDL layer, is there
	// a chance that this happens synchronously from an SDL call that we are making while
	// already holding the lock?
	m_lock.enter();
	// Acknowledge that we are ready with the data provided to us
	// at the previous callback time
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::playdone: m_src->readdone(%d)", m_audio_chunk.alen);
	m_audio_src->readdone(m_audio_chunk.alen);
	m_audio_chunk_busy = false;
	bool still_busy;
	still_busy = restart_audio_output();
	still_busy |= restart_audio_input();
	if (!still_busy) {
		assert(m_channel_used >= 0);
		AM_DBG lib::logger::get_logger()->trace("Unlocking channel %d", m_channel_used);
		// XXX Need to delete reference on this created when we did lock_channel().
		m_static_lock.enter();
		unlock_channel(m_channel_used);
		m_static_lock.leave();
		release(); // XXXX Jack thinks this is suspect here...
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::playdone: calling stopped_callback() this = (x%x)",this);
		m_lock.leave();
		stopped_callback();
		return;
	}
	m_lock.leave();
}

bool
gui::sdl::sdl_active_audio_renderer::restart_audio_output()
{
	// private method - no need to lock.
	assert(m_audio_src);
	if (m_audio_chunk_busy) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::restart_audio_output: already playing");
		return true;
	}
	if (m_audio_src->size() == 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::restart_audio_output: no more data");
		return false;
	}
	m_audio_chunk.allocated = 0;
	m_audio_chunk.volume = 128;
	m_audio_chunk.abuf = (Uint8*) m_audio_src->get_read_ptr();
	m_audio_chunk.alen = m_audio_src->size();
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::restart_audio_output: Mixing %d bytes", m_audio_chunk.alen);
	
	if (m_channel_used < 0) {
		m_static_lock.enter();
		new_channel();
		m_static_lock.leave();
	}
	
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::restart_audio_output: STARTING TO PLAY");
	int result;
	result = Mix_PlayChannel(m_channel_used, &m_audio_chunk, 0);
	if (result < 0) {
		lib::logger::get_logger()->error("sdl_active_renderer.restart_audio_output(0x%x): Failed to play sound", (void *)this);	
		AM_DBG printf("Mix_PlayChannel: %s\n",Mix_GetError());
	}
	m_audio_chunk_busy = true;
	return true;
}

bool
gui::sdl::sdl_active_audio_renderer::restart_audio_input()
{
	// private method - no need to lock.
	if (m_audio_src->end_of_file()) {
		// No more data.
		return false;
	}
	if (m_audio_src->size() == 0) {
		// Start reading 
		lib::event *e = new readdone_callback(this, &sdl_active_audio_renderer::data_avail);
		m_audio_src->start(m_event_processor, e);
	}
	return true;
}

void
gui::sdl::sdl_active_audio_renderer::data_avail()
{
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::data_avail: about to acquire lock");
	m_lock.enter();
	assert(m_audio_src);
	restart_audio_output();
	restart_audio_input();
	m_lock.leave();
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::data_avail: done");
}	

void
gui::sdl::sdl_active_audio_renderer::new_channel()
{
	// private method - no need to lock. But do make sure you hold m_static_lock too!
	int result;
	m_channel_used = free_channel();
			
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer:: free_channel() returned  : %d", m_channel_used);
		if (m_channel_used < 0) {
			result = inc_channels();
			if( result < 0) 
			{	
				lib::logger::get_logger()->error("sdl_active_renderer.new_channel(0x%x): failed to increase channels ", (void *)this);	
			}
			m_channel_used = free_channel();
			assert(m_channel_used >= 0);
		}
		// XXX Need to addref() this!!
		add_ref();
		lock_channel((void*) this, m_channel_used);	
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer:: New Channel : %d", m_channel_used);
}

bool
gui::sdl::sdl_active_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_paused(): channel not in use");
		rv = false;
	} else if( Mix_Paused(m_channel_used) == 1) {
		rv = true;
	} else {
		rv = false;
	}
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_active_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_stopped(): channel not in use");
		rv = true;
	} else if( Mix_Playing(m_channel_used) == 0 ) {
		rv = true;
	} else {
		rv = false;
	}
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_active_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_playing(): channel not in use");
		rv = false;
	} else if( Mix_Playing(m_channel_used) == 1 ) {
		rv = true;
	} else {
		rv = false;
	}
	m_lock.leave();
	return rv;
}


void
gui::sdl::sdl_active_audio_renderer::stop()
{
	m_lock.enter();
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::stop(): channel not in use");
		return;
	}
	// Release the lock before halting the channel, so a possible callback will work.
	int channel = m_channel_used;
	m_lock.leave();
	Mix_HaltChannel(channel);
}

void
gui::sdl::sdl_active_audio_renderer::pause()
{
	m_lock.enter();
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::pause(): channel not in use");
		return;
	}
	// Release the lock before halting the channel, so a possible callback will work.
	int channel = m_channel_used;
	m_lock.leave();
	Mix_Pause(channel);
}

void
gui::sdl::sdl_active_audio_renderer::resume()
{
	m_lock.enter();
	if (m_channel_used < 0) {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::resume(): channel not in use");
		return;
	}
	// Release the lock before halting the channel, so a possible callback will work.
	int channel = m_channel_used;
	m_lock.leave();
	Mix_Resume(channel);
}


void
gui::sdl::sdl_active_audio_renderer::start(double where)
{
	m_lock.enter();
    if (!m_node) abort();
	
	std::ostringstream os;
	os << *m_node;
	
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_audio_src) {
		lib::event *e = new readdone_callback(this, &sdl_active_audio_renderer::data_avail);
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::start(): m_audio_src->start(0x%x, 0x%x) this = (x%x)m_audio_src=0x%x", (void*)m_event_processor, (void*)e, this, (void*)m_audio_src);
		m_audio_src->start(m_event_processor, e);
		m_lock.leave();
	} else {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer.start: no datasource");
		m_lock.leave();
		stopped_callback();
	}
}
