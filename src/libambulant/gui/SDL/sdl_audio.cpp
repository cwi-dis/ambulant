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
#include <stdlib.h>

using namespace ambulant;
using namespace gui::sdl;

typedef lib::no_arg_callback<sdl_active_audio_renderer> readdone_callback;
	
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
	int err;
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
        sdl_active_audio_renderer* object;
        object = (sdl_active_audio_renderer*) get_ptr(channel);
		if (object == NULL) {
			lib::logger::get_logger()->error("sdl_audio:channel_done(%d): get_ptr returned NULL, no object!", channel);
			return;
		}
		// XXX playdone is the wrong call : more data may be coming.
        object->playdone();
}

} //end extern "C"
	
bool sdl_active_audio_renderer::m_sdl_init = false;
int	 sdl_active_audio_renderer::m_mixed_channels = 0;



	
sdl_active_audio_renderer::sdl_active_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	net::passive_datasource *src)
:	common::active_renderer(context, cookie, node, evp, src, NULL),
    m_rate(44100),
    m_bits(16),
    m_channels(1),
	m_buffer_size(4096),
	m_channel_used(-1),
	m_audio_format(AUDIO_S16SYS)
{
	AM_DBG lib::logger::get_logger()->trace("****** sdl_active_audio_renderer::sdl_active_audio_renderer() this=(x%x)",  this);
	if (m_src) {
#ifdef WITH_FFMPEG
		std::string url = src->get_url();
		
		AM_DBG lib::logger::get_logger()->trace("sdl_audio_renderer: url.rfind(\".mp3\") %d, url.size()-4 %d", url.rfind(".mp3"),  url.size()-4);
		if (url.rfind(".mp3") == url.size()-4 ) {
			AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::sdl_active_audio_renderer: using ffmpeg audio reader");
			m_audio_src = new net::ffmpeg_audio_datasource(m_src, evp);
		} else
#endif
		{
			AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::sdl_active_audio_renderer: using raw audio reader");
			m_audio_src = new net::raw_audio_datasource(m_src);
		}
	} else {
		lib::logger::get_logger()->error("sdl_active_audio_renderer: m_src=NULLL, datasource not created");
		m_audio_src = NULL;
	}
}

sdl_active_audio_renderer::~sdl_active_audio_renderer()
{
	AM_DBG lib::logger::get_logger()->trace("****** sdl_active_audio_renderer::~sdl_active_audio_renderer() this=(x%x)",  this);		
}

int
sdl_active_audio_renderer::init(int rate, int bits, int channels)
{
    int err = 0;
	if (m_sdl_init) return 0; // XXX try by Jack
    if (!m_sdl_init) {	
  		err = SDL_Init(SDL_INIT_AUDIO| SDL_INIT_NOPARACHUTE);
		if (err < 0) {
        	lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): SDL init failed", (void *)this);
       		return err;
    		} 
		m_sdl_init = true;
		m_mixed_channels=16;
		err = Mix_AllocateChannels(m_mixed_channels);
		Mix_ChannelFinished(channel_done);
		init_channel_list(m_mixed_channels);
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
sdl_active_audio_renderer::inc_channels()
{
	m_mixed_channels += 16;
	int err;
	err = Mix_AllocateChannels(m_mixed_channels);
	err = resize_channel_list(m_mixed_channels);
	return err;
}




void
sdl_active_audio_renderer::playdone()
{
	m_audio_src->readdone(m_audio_chunck.alen);
	assert(m_channel_used >= 0);
	if (m_audio_src->end_of_file()) {
		AM_DBG lib::logger::get_logger()->trace("Unlocking channel %d", m_channel_used);
		unlock_channel(m_channel_used);
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::playdone: calling stopped_callback() this = (x%x)",this);
		stopped_callback();
	} else {
		lib::event *e = new readdone_callback(this, &common::active_renderer::readdone);
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::playdone(): m_audio_src->start(0x%x, 0x%x) this = (x%x)", (void*)m_event_processor, (void*)e, this);
		m_audio_src->start(m_event_processor, e);
	}
}



void
sdl_active_audio_renderer::readdone()
{
	int result;
	assert(m_audio_src);
	m_audio_chunck.allocated = 0;
	m_audio_chunck.volume = 128;
	m_audio_chunck.abuf = (Uint8*) m_audio_src->get_read_ptr();
	
	m_audio_chunck.alen = m_audio_src->size();
	m_rate = m_audio_src->get_samplerate();
		m_bits = m_audio_src->get_nbits();
		m_channels = m_audio_src->get_nchannels();
		AM_DBG lib::logger::get_logger()->trace("sr=%d, bits=%d, channels=%d ", m_rate, m_bits, m_channels);

	if (!m_sdl_init)
		{
#ifdef WITH_FFMPEG
		AM_DBG lib::logger::get_logger()->trace("Using ffmpeg MP3 support");
#else
		AM_DBG lib::logger::get_logger()->trace("Not using ffmpeg MP3 support, only raw audio !");
#endif
		
		init(m_rate, m_bits, m_channels);	
		}
	if (m_channel_used < 0) {
		m_channel_used = free_channel();
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone: STARTING TO PLAY");	
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone: free_channel() returned  : %d", m_channel_used);
		if (m_channel_used < 0) {
			result = inc_channels();
			if( result < 0) 
			{	
				lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): failed memory allocation ", (void *)this);	
			}
			m_channel_used = free_channel();
			assert(m_channel_used >= 0);
		}	
		lock_channel((void*) this, m_channel_used);	
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone: New Channel : %d", m_channel_used);
		result = Mix_PlayChannel(m_channel_used, &m_audio_chunck, 0);
//		m_audio_src->readdone(m_audio_chunck.alen);
	} else {
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone: PLAYING USING CHANNEL : %d", m_channel_used);	
	
		result = Mix_PlayChannel(m_channel_used, &m_audio_chunck, 0);
//		m_audio_src->readdone(m_audio_chunck.alen);
	}

	if (result < 0) {
		lib::logger::get_logger()->error("sdl_active_renderer.init(0x%x): Failed to play sound", (void *)this);	
		AM_DBG printf("Mix_PlayChannel: %s\n",Mix_GetError());
//	} else {
//		if(m_channel_used < 0) {
//			m_channel_used = result;
//		}
	}
	
#if 0
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone(): %d bytes still in buffer, EOF : %d",m_audio_src->size(), m_audio_src->end_of_file());
	if ( !m_audio_src->end_of_file() ) {
		lib::event *e = new readdone_callback(this, &lib::active_renderer::readdone);
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::readdone(): m_audio_src->start(0x%x, 0x%x) this = (x%x)", (void*)m_event_processor, (void*)e, this);
		m_audio_src->start(m_event_processor, e);
	}
#endif

}	



bool
sdl_active_audio_renderer::is_paused()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_paused(): channel not in use");
		return false;
	}
	if( Mix_Paused(m_channel_used) == 1) {
		return true;
	} else {
		return false;
	}
}

bool
sdl_active_audio_renderer::is_stopped()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_stopped(): channel not in use");
		return true;
	}
	if( Mix_Playing(m_channel_used) == 0 ) {
		return true;
	} else {
		return false;
	}
}

bool
sdl_active_audio_renderer::is_playing()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::is_playing(): channel not in use");
		return false;
	}
	if( Mix_Playing(m_channel_used) == 1 ) {
		return true;
	} else {
		return false;
	}
}


void
sdl_active_audio_renderer::stop()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::stop(): channel not in use");
		return;
	}
	Mix_HaltChannel(m_channel_used);
}

void
sdl_active_audio_renderer::pause()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::pause(): channel not in use");
		return;
	}
	Mix_Pause(m_channel_used);
}

void
sdl_active_audio_renderer::resume()
{
	if (m_channel_used < 0) {
		lib::logger::get_logger()->trace("sdl_active_audio_renderer::resume(): channel not in use");
		return;
	}
	Mix_Resume(m_channel_used);
}


void
sdl_active_audio_renderer::start(double where)
{

    if (!m_node) abort();
	
	std::ostringstream os;
	os << *m_node;
	
	AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_audio_src) {
		init(m_rate, m_bits, m_channels);
		lib::event *e = new readdone_callback(this, &common::active_renderer::readdone);
		AM_DBG lib::logger::get_logger()->trace("sdl_active_audio_renderer::start(): m_audio_src->start(0x%x, 0x%x) this = (x%x)", (void*)m_event_processor, (void*)e, this);
		m_audio_src->start(m_event_processor, e);
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		stopped_callback();
	}
}
