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

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/SDL/sdl_audio.h"
#include "malloc.h"

namespace ambulant {

using namespace lib;
	
extern "C" {
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
	if(channel_nr < m_channel_list.m_channels_open) {
		if (m_channel_list.m_list[channel_nr].m_free == false) {
				return -1;
		}
		m_channel_list.m_list[channel_nr].m_free = false;
		m_channel_list.m_list[channel_nr].m_ptr = ptr;
	} else {
		return -2;
	}
	return 0;
}

int unlock_channel(int channel_nr)
{
	if(channel_nr < m_channel_list.m_channels_open) {
		m_channel_list.m_list[channel_nr].m_free = true;
		m_channel_list.m_list[channel_nr].m_ptr = NULL;
	} else {
		return -2;
	}
	return 0;
}

// Returns pointer to an sdl_active_audio_renderer, or NULL incase the channel is not in use or does not exist.
void* get_ptr(int channel_nr)
{
	if(channel_nr < m_channel_list.m_channels_open) {
		if (m_channel_list.m_list[channel_nr].m_free == true) {
				return NULL;
		}
		return m_channel_list.m_list[channel_nr].m_ptr;
	}
	return NULL;
}

// returns a channel number and locks it or returns -1 if there are no free channels or the channel does not exist.
int free_channel()
{
	int i;
	int err;
	for(i=0;i < m_channel_list.m_channels_open; i++) {
		if(m_channel_list.m_list[i].m_free == true) {
			return m_channel_list.m_list[i].m_channel_nr;
		}
	}
	return -1;
}

 

void channel_done(int channel)
{
        gui::sdl::sdl_active_audio_renderer* dummy;
        dummy = (gui::sdl::sdl_active_audio_renderer*) get_ptr(channel);
        dummy->playdone();
}

} //end extern "C"
	
bool gui::sdl::sdl_active_audio_renderer::m_sdl_init = false;
int	 gui::sdl::sdl_active_audio_renderer::m_mixed_channels = 0;



	
gui::sdl::sdl_active_audio_renderer::sdl_active_audio_renderer(
	active_playable_events *context,
	active_playable_events::cookie_type cookie,
	const node *node,
	event_processor *const evp,
	net::passive_datasource *src)
:	active_renderer(context, cookie, node, evp, src, NULL)
{
    m_rate = 44100;
    m_channels = 1;
    m_bits=16;
	m_audio_format = AUDIO_S16;
	m_buffer_size = 4096;
	m_channel_used = 0;	
}

int
gui::sdl::sdl_active_audio_renderer::init(int rate, int bits, int channels)
{
    int err = 0;
    if (!m_sdl_init) {	
  		err = SDL_Init(SDL_INIT_AUDIO /*| SDL_INIT_NOPARACHUTE*/);
		if (err < 0) {
        	lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): SDL init failed", (void *)this);
       		return err;
    		} 
		m_sdl_init = true;
		m_mixed_channels=16;
		err = Mix_AllocateChannels(m_mixed_channels);
		Mix_ChannelFinished(channel_done);
		init_channel_list(m_mixed_channels);
			dump_channel_list();
		AM_DBG lib::logger::get_logger()->trace("sdl_active_renderer.init(0x%x): SDL init succes", (void *)this);			
    } else {
        err = 0;
    }
	m_rate = rate;
    m_channels = channels;
    m_bits = bits;
	err = Mix_OpenAudio(m_rate, m_audio_format, m_channels, m_buffer_size);
	if (err < 0) {
		lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): SDL open failed", (void *)this);
    	return err;
	}
		
   return err;
}

int
gui::sdl::sdl_active_audio_renderer::inc_channels()
{
	m_mixed_channels += 16;
	int err;
	err = Mix_AllocateChannels(m_mixed_channels);
	err = resize_channel_list(m_mixed_channels);
	return err;
}


gui::sdl::sdl_active_audio_renderer::~sdl_active_audio_renderer()
{
	
	
}

void
gui::sdl::sdl_active_audio_renderer::playdone()
{
	AM_DBG lib::logger::get_logger()->trace("Unlocking channel %d", m_channel_used);
	unlock_channel(m_channel_used);
}



void
gui::sdl::sdl_active_audio_renderer::readdone()
{
	int result;
	
	m_audio_chunck.allocated = 0;
	m_audio_chunck.volume = 128;
	m_audio_chunck.abuf = (Uint8*) m_src->read_ptr();
	
	m_audio_chunck.alen = m_src->size();
	AM_DBG lib::logger::get_logger()->trace("STARTING TO PLAY");	

	if (m_channel_used == 0) {
		m_channel_used = free_channel();
		AM_DBG lib::logger::get_logger()->trace("free_channel() returned  : %d", m_channel_used);
		if (m_channel_used < 0) {
			result = inc_channels();
			if( result < 0) 
			{	
				lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): failed memory allocation ", (void *)this);	
			}
			m_channel_used = free_channel();
		}	
		lock_channel((void*) this, m_channel_used);	
		AM_DBG lib::logger::get_logger()->trace("New Channel : %d, %d", m_channel_used);
		result = Mix_PlayChannel(m_channel_used,&m_audio_chunck, 0);
	} else {
		AM_DBG lib::logger::get_logger()->trace("PLAYING USING CHANNEL : %d", m_channel_used);	
		result = Mix_PlayChannel(m_channel_used, &m_audio_chunck, 0);
	}
	
	if (result < 0) {
		lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): Failed to play sound", (void *)this);	
		AM_DBG printf("Mix_PlayChannel: %s\n",Mix_GetError());
	} else {
		if(m_channel_used == 0) {
			m_channel_used = result;
		}
	}
	
    if (!m_src->end_of_file()) {
		m_src->start(m_event_processor, m_readdone);
	} else {
		stopped_callback();
	}
}


void 
gui::sdl::sdl_active_audio_renderer::callback(void *userdata, Uint8 *stream, int len)
{
	Uint8 *in_ptr;
	int size;
	size = m_src->size();
	if (size > 0) {
		if (size > len) {
			in_ptr = (Uint8*) m_src->read_ptr();
			memcpy(stream, in_ptr, len);
			m_src->readdone(len);
		} else {
			in_ptr = (Uint8*) m_src->read_ptr();
		memcpy(stream, in_ptr, size);
			m_src->readdone(size);
		}	
	} 
}

bool
gui::sdl::sdl_active_audio_renderer::is_paused()
{
	if( Mix_Paused(m_channel_used) == 1) {
		return true;
	} else {
		return false;
	}
}

bool
gui::sdl::sdl_active_audio_renderer::is_stopped()
{
	if( Mix_Playing(m_channel_used) == 0 ) {
		return true;
	} else {
		return false;
	}
}

bool
gui::sdl::sdl_active_audio_renderer::is_playing()
{
	if( Mix_Playing(m_channel_used) == 1 ) {
		return true;
	} else {
		return false;
	}
}


void
gui::sdl::sdl_active_audio_renderer::stop()
{
	Mix_HaltChannel(m_channel_used);
}

void
gui::sdl::sdl_active_audio_renderer::pause()
{
	Mix_Pause(m_channel_used);
}

void
gui::sdl::sdl_active_audio_renderer::resume()
{
	Mix_Resume(m_channel_used);
}


void
gui::sdl::sdl_active_audio_renderer::start(double where)
{

    if (!m_node) abort();
	
	std::ostringstream os;
	os << *m_node;
	
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_src) {
		init(m_rate, m_bits, m_channels);
		m_src->start(m_event_processor, m_readdone);
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		if (m_playdone) {
            stopped_callback();
        }
	}
}


}// end namespace ambulant
