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

extern "C" {

void sdl_C_callback(void *userdata, Uint8 *stream, int len)
{
	gui::sdl::sdl_active_audio_renderer::sdl_callback(stream, len);
}

}

static void
add_samples(short *outbuf, short *inbuf, int size)
{
	int i;
	for(i=0; i<size; i++) {
		long value = (long)outbuf[i] + (long)inbuf[i];
		if (value > 0x7fff) value = 0x7fff;
		else if (value < -0x7fff) value = -0x7fff;
		outbuf[i] = (short)value;
	}
}

typedef lib::no_arg_callback<gui::sdl::sdl_active_audio_renderer> readdone_callback;
	
// ************************************************************

bool gui::sdl::sdl_active_audio_renderer::m_sdl_init = false;
Uint16 gui::sdl::sdl_active_audio_renderer::m_sdl_format = AUDIO_S16SYS;
net::audio_format gui::sdl::sdl_active_audio_renderer::m_ambulant_format = net::audio_format(44100, 2, 16);
int gui::sdl::sdl_active_audio_renderer::m_buffer_size = 4096;    
lib::critical_section gui::sdl::sdl_active_audio_renderer::m_static_lock;    
std::list<gui::sdl::sdl_active_audio_renderer *> gui::sdl::sdl_active_audio_renderer::m_renderers;

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
		lib::logger::get_logger()->trace("sdl_active_audio_renderer.init: SDL_Init failed: error %d", err);
		lib::logger::get_logger()->error("Cannot initialize SDL audio library");
		m_static_lock.leave();
		return err;
	}
	
	// Step three - open the mixer
	SDL_AudioSpec desired, obtained;
	(void) memset(&desired, 0, sizeof(SDL_AudioSpec));
	(void) memset(&obtained, 0, sizeof(SDL_AudioSpec));
	desired.freq = m_ambulant_format.samplerate;
	desired.format = m_sdl_format;
	desired.channels = m_ambulant_format.channels;
	desired.samples = m_buffer_size;
	desired.callback = sdl_C_callback;
	desired.userdata = NULL;
	err = SDL_OpenAudio(&desired, &obtained);
	if (err < 0) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio failed: error %d", err);
		lib::logger::get_logger()->error("Cannot open SDL audio output stream");
		m_static_lock.leave();
    	return err;
	}
	m_ambulant_format.samplerate = obtained.freq;
	m_ambulant_format.channels = obtained.channels;
	if (obtained.format != m_sdl_format) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio could not support format 0x%x, returned 0x%x",
			m_sdl_format, obtained.format);
		lib::logger::get_logger()->error("Cannot open SDL audio output stream with required characteristics");
		m_static_lock.leave();
		return -1;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer.init: SDL init succes");			
	m_sdl_init = true;
	m_static_lock.leave();
	return err;
}

void
gui::sdl::sdl_active_audio_renderer::register_renderer(sdl_active_audio_renderer *rnd)
{
	m_static_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::register_renderer(0x%x)", rnd);
	m_renderers.push_back(rnd);
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::register_renderer: unpause SDL");
	SDL_PauseAudio(0);
	m_static_lock.leave();
}

void
gui::sdl::sdl_active_audio_renderer::unregister_renderer(sdl_active_audio_renderer *rnd)
{
	m_static_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::unregister_renderer(0x%x)", rnd);
	std::list<sdl_active_audio_renderer *>::iterator i;
	for( i=m_renderers.begin(); i != m_renderers.end(); i++) {
		if ((*i) == rnd) {
			m_renderers.erase(i);
			break;
		}
	}
	if (m_renderers.size() == 0) {
		AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::register_renderer: pause SDL");
		SDL_PauseAudio(1);
	}
	m_static_lock.leave();
}

void
gui::sdl::sdl_active_audio_renderer::sdl_callback(Uint8 *stream, int len)
{
	m_static_lock.enter();
	std::list<sdl_active_audio_renderer *>::iterator first = m_renderers.begin();
	if (m_renderers.size() == 1) {
		// Exactly one active stream: use simple copy
		Uint8 *single_data;
		int single_len = (*first)->get_data(len, &single_data);
		if (single_len != 0)
			memcpy(stream, single_data, std::min(len, single_len));
		(*first)->get_data_done(single_len);
		if (single_len < len)
			memset(stream+single_len, 0, (len-single_len));
	} else {
		// No streams, or more than one: use an accumulation buffer
		memset(stream, 0, len);
		std::list<sdl_active_audio_renderer *>::iterator i;
		for (i=first; i != m_renderers.end(); i++) {
			Uint8 *next_data;
			int next_len = (*i)->get_data(len, &next_data);
			if (next_len)
				add_samples((short*)stream, (short*)next_data, std::min(len/2, next_len/2));
			(*i)->get_data_done(next_len);
		}
	}
	m_static_lock.leave();
}

// ************************************************************

gui::sdl::sdl_active_audio_renderer::sdl_active_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::datasource_factory *df)
:	common::playable_imp(context, cookie, node, evp),
	m_audio_src(NULL),
	m_is_playing(false),
	m_is_paused(false)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::sdl_active_audio_renderer() -> 0x%x",  this);
	if (init() != 0)
		return;
		
	net::audio_format_choices supported = net::audio_format_choices(m_ambulant_format);
	net::url url = node->get_url("src");
	m_audio_src = df->new_audio_datasource(url, supported);
	if (!m_audio_src)
		lib::logger::get_logger()->error("%s: cannot open audio file", repr(url).c_str());
	else if (!supported.contains(m_audio_src->get_audio_format())) {
		lib::logger::get_logger()->error("%s: audio format not supported", repr(url).c_str());
		m_audio_src->release();
		m_audio_src = NULL;
	}
}

gui::sdl::sdl_active_audio_renderer::sdl_active_audio_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	net::datasource_factory *df,
	net::audio_datasource *ds)
:	common::playable_imp(context, cookie, node, evp),
	m_audio_src(ds),
	m_is_playing(false),
	m_is_paused(false)
{
	net::audio_format_choices supported = net::audio_format_choices(m_ambulant_format);
	net::url url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::sdl_active_audio_renderer() this=(x%x)",  this);
	if (init() != 0)
		return;
		
	if (!m_audio_src)
		lib::logger::get_logger()->error("%s: cannot open", repr(url).c_str());
	
	// Ugly hack to get the resampler.
	if (m_audio_src) {
		net::audio_datasource *resample_ds = df->new_filter_datasource(url, supported, ds);
		AM_DBG lib::logger::get_logger ()->debug("active_video_renderer::active_video_renderer() (this =0x%x) got resample datasource 0x%x", (void *) this, resample_ds);
		if (resample_ds) {
			m_audio_src = resample_ds;
			AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer: opened resample datasource !");
		}
	}
}


gui::sdl::sdl_active_audio_renderer::~sdl_active_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::~sdl_active_audio_renderer(0x%x) m_audio_src=0x%x",  this, m_audio_src);		
	if (m_is_playing) {
		m_lock.leave();
		unregister_renderer(this);
		m_lock.enter();
	}
	if (m_audio_src) m_audio_src->release();
	m_audio_src = NULL;
	m_is_playing = false;
	m_lock.leave();
}

int
gui::sdl::sdl_active_audio_renderer::get_data(int bytes_wanted, Uint8 **ptr)
{
	m_lock.enter();
	assert(m_is_playing);
	int rv;
	if (m_is_paused||!m_audio_src) {
		rv = 0;
	} else {
		*ptr = (Uint8 *)m_audio_src->get_read_ptr();
		rv = m_audio_src->size();
		if (rv > bytes_wanted)
			rv = bytes_wanted;
	}
	m_lock.leave();
	return rv;
}

void
gui::sdl::sdl_active_audio_renderer::get_data_done(int size)
{
	m_lock.enter();
	// Acknowledge that we are ready with the data provided to us
	// at the previous callback time
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::get_data_done: m_src->readdone(%d), %d more", size, m_audio_src->size()-size);
	if (size)
		m_audio_src->readdone(size);
	bool still_busy;
	still_busy = (size != 0);
	still_busy |= restart_audio_input();
	if (!still_busy) {
		AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::playdone: calling m_context->stopped() this = (x%x)",this);
		assert(m_is_playing);
		// We cannot call unregister_renderer from here, because we are called from the
		// SDL callback and already holding the m_global_lock. So, in stead
		// we use the event processor to unregister ourselves later.
		lib::event *e = new readdone_callback(this, &sdl_active_audio_renderer::stop);
		m_event_processor->add_event(e, 0, ambulant::lib::event_processor::high);
		if (m_audio_src) {
			m_audio_src->stop();
			m_audio_src->release();
			m_audio_src = NULL;
		}
		m_lock.leave();
		if (m_context) {
			m_context->stopped(m_cookie, 0);
		} else {
			AM_DBG lib::logger::get_logger()->warn("sdl_active_audio_renderer(0x%x): m_context is  NULL", (void*)this);
		}
		return;
	}
	m_lock.leave();
}

bool
gui::sdl::sdl_active_audio_renderer::restart_audio_input()
{
	// private method - no need to lock.
	if (!m_audio_src || m_audio_src->end_of_file()) {
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
	m_lock.enter();
	//assert(m_audio_src);
	if (!m_audio_src) {				
		AM_DBG lib::logger::get_logger()->warn("sdl_active_audio_renderer::data_avail:m_audio_src does not exist");
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::data_avail: %d bytes available", m_audio_src->size());
	restart_audio_input();
	m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::data_avail: done");
}	

bool
gui::sdl::sdl_active_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	rv = m_is_paused;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_active_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	rv = !m_is_playing;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_active_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	rv = m_is_playing;
	m_lock.leave();
	return rv;
}


void
gui::sdl::sdl_active_audio_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::stop()");
	if (m_is_playing) {
		m_lock.leave();
		unregister_renderer(this);
		// XXX Should we call stopped_callback?
		m_lock.enter();
	}
	m_is_playing = false;
	if (m_audio_src) {
		m_audio_src->stop();
		m_audio_src->release();
		m_audio_src = NULL;
	}
	m_lock.leave();
}

void
gui::sdl::sdl_active_audio_renderer::pause()
{
	m_lock.enter();
	m_is_paused = true;
	m_lock.leave();
}

void
gui::sdl::sdl_active_audio_renderer::resume()
{
	m_lock.enter();
	m_is_paused = false;
	m_lock.leave();
}

void
gui::sdl::sdl_active_audio_renderer::start(double where)
{
	m_lock.enter();
    if (!m_node) abort();
	
	std::ostringstream os;
	os << *m_node;
	
	AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_audio_src) {
		lib::event *e = new readdone_callback(this, &sdl_active_audio_renderer::data_avail);
		AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer::start(): m_audio_src->start(0x%x, 0x%x) this = (x%x)m_audio_src=0x%x", (void*)m_event_processor, (void*)e, this, (void*)m_audio_src);
		m_audio_src->start(m_event_processor, e);
		m_is_playing = true;
		m_is_paused = false;
		m_lock.leave();
		register_renderer(this);
	} else {
		AM_DBG lib::logger::get_logger()->debug("sdl_active_audio_renderer.start: no datasource");
		m_lock.leave();
		m_context->stopped(m_cookie, 0);
	}
}
