// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/SDL/sdl_audio.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
//using namespace gui::sdl;

extern "C" {

void sdl_C_callback(void *userdata, Uint8 *stream, int len)
{
	gui::sdl::sdl_audio_renderer::sdl_callback(stream, len);
}

}

static void
add_samples(short *outbuf, short *inbuf, int size, float *volumes, int volcount)
{
	int i;
	int vol_index = 0;
	for(i=0; i<size; i++) {
		long value = (long)inbuf[i];
		if (volcount) {
			value = (long)(value * volumes[vol_index]);
			if (++vol_index >= volcount) vol_index = 0;
		}
		value += (long)outbuf[i];
		if (value > 0x7fff) value = 0x7fff;
		else if (value < -0x7fff) value = -0x7fff;
		outbuf[i] = (short)value;
	}
}

typedef lib::no_arg_callback<gui::sdl::sdl_audio_renderer> readdone_callback;
	
// ************************************************************

bool gui::sdl::sdl_audio_renderer::s_sdl_init = false;
Uint16 gui::sdl::sdl_audio_renderer::s_sdl_format = AUDIO_S16SYS;
net::audio_format gui::sdl::sdl_audio_renderer::s_ambulant_format = net::audio_format(44100, 2, 16);
int gui::sdl::sdl_audio_renderer::s_buffer_size = 4096;
int gui::sdl::sdl_audio_renderer::s_min_buffer_size_bytes = 2 * 4096 * 2 * 2;  
lib::critical_section gui::sdl::sdl_audio_renderer::s_static_lock;    
std::list<gui::sdl::sdl_audio_renderer *> gui::sdl::sdl_audio_renderer::s_renderers;

int
gui::sdl::sdl_audio_renderer::init()
{
	s_static_lock.enter();
	if (s_sdl_init) {
		s_static_lock.leave();
		return 0;
	}
    int err = 0;
	
	// XXXX Should check that s_ambulant_format and s_sdl_format match!
	
	// Step one - initialize the SDL library
	err = SDL_Init(SDL_INIT_AUDIO| SDL_INIT_NOPARACHUTE);
	if (err < 0) {
		lib::logger::get_logger()->trace("sdl_audio_renderer.init: SDL_Init failed: error %d", err);
		lib::logger::get_logger()->error(gettext("Cannot initialize SDL audio library"));
		s_static_lock.leave();
		return err;
	}
	
	// Step three - open the mixer
	SDL_AudioSpec desired, obtained;
	(void) memset(&desired, 0, sizeof(SDL_AudioSpec));
	(void) memset(&obtained, 0, sizeof(SDL_AudioSpec));
	desired.freq = s_ambulant_format.samplerate;
	desired.format = s_sdl_format;
	desired.channels = s_ambulant_format.channels;
	desired.samples = s_buffer_size;
	desired.callback = sdl_C_callback;
	desired.userdata = NULL;
	err = SDL_OpenAudio(&desired, &obtained);
	if (err < 0) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio failed: error %d", err);
		lib::logger::get_logger()->error(gettext("Cannot open SDL audio output stream"));
		s_static_lock.leave();
    	return err;
	}
	s_ambulant_format.samplerate = obtained.freq;
	s_ambulant_format.channels = obtained.channels;
	if (obtained.format != s_sdl_format) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio could not support format 0x%x, returned 0x%x",
			s_sdl_format, obtained.format);
		lib::logger::get_logger()->error(gettext("Cannot open SDL audio output stream with required characteristics"));
		s_static_lock.leave();
		return -1;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.init: SDL init succes");			
	s_sdl_init = true;
	s_static_lock.leave();
	return err;
}

void
gui::sdl::sdl_audio_renderer::register_renderer(sdl_audio_renderer *rnd)
{
	s_static_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer(0x%x)", rnd);
	std::list<sdl_audio_renderer *>::iterator i;
	for( i=s_renderers.begin(); i != s_renderers.end(); i++) {
		if ((*i) == rnd) {
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer() already exists !");
			s_static_lock.leave();
			return;
		}
	}
	s_renderers.push_back(rnd);
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer: unpause SDL");
	SDL_PauseAudio(0);
	s_static_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::unregister_renderer(sdl_audio_renderer *rnd)
{
	s_static_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::unregister_renderer(0x%x)", rnd);
	std::list<sdl_audio_renderer *>::iterator i;
	for( i=s_renderers.begin(); i != s_renderers.end(); i++) {
		if ((*i) == rnd) {
			s_renderers.erase(i);
			break;
		}
	}
	if (s_renderers.size() == 0) {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer: pause SDL");
		SDL_PauseAudio(1);
	}
	s_static_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::sdl_callback(Uint8 *stream, int len)
{
	s_static_lock.enter();
	std::list<sdl_audio_renderer *>::iterator first = s_renderers.begin();
	if (s_renderers.size() == 1 && (*first)->m_volcount == 0
#ifdef USE_SMIL21
	    && ! ((*first)->m_intransition || (*first)->m_outtransition)
#endif
	    ) {
		// Exactly one active stream, no volume/pan processing,
		// no transitions: use simple copy
		Uint8 *single_data;

		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d) [one stream] calling get_data()", (void*) stream, len);

		int single_len = (*first)->get_data(len, &single_data);
		assert(single_len <= len);
		if (single_len != 0) {
			assert(single_data);
			memcpy(stream, single_data, std::min(len, single_len));
		}

		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d) [one stream] calling get_data_done(%d)", (void*) stream, len, single_len);

		(*first)->get_data_done(single_len);
		if (single_len < len)
			memset(stream+single_len, 0, (len-single_len));
	} else {
		// No streams, or more than one: use an accumulation buffer
		memset(stream, 0, len);
		std::list<sdl_audio_renderer *>::iterator i;
		for (i=first; i != s_renderers.end(); i++) {
			Uint8 *next_data;
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d))calling get_data() ", (void*) stream, len);
			int next_len = (*i)->get_data(len, &next_data);
			if (next_len)
				add_samples((short*)stream, (short*)next_data, std::min(len/2, next_len/2), (*i)->m_volumes, (*i)->m_volcount);

			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d))calling get_data_done(%d) ", (void*) stream, len, next_len);
			(*i)->get_data_done(next_len);
		}
	}
	s_static_lock.leave();
}

// ************************************************************

gui::sdl::sdl_audio_renderer::sdl_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	common::renderer_playable(context, cookie, node, evp),
	m_audio_src(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_audio_renderer() -> 0x%x",  this);
	if (init() != 0)
		return;
		
	net::audio_format_choices supported(s_ambulant_format);
	net::url url = node->get_url("src");
	
	_init_clip_begin_end();
	
	m_audio_src = factory->get_datasource_factory()->new_audio_datasource(url, supported, m_clip_begin, m_clip_end);
	if (!m_audio_src)
		lib::logger::get_logger()->error(gettext("%s: cannot open audio file"), repr(url).c_str());
	else if (!supported.contains(m_audio_src->get_audio_format())) {
		lib::logger::get_logger()->error(gettext("%s: audio format not supported"), repr(url).c_str());
		m_audio_src->release();
		m_audio_src = NULL;
	}
}

gui::sdl::sdl_audio_renderer::sdl_audio_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories* factory,
	net::audio_datasource *ds)
:	common::renderer_playable(context, cookie, node, evp),
	m_audio_src(ds),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
	net::audio_format_choices supported(s_ambulant_format);
	net::url url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_audio_renderer() this=(x%x), ds = 0x%x",  (void*) this, (void*) ds);
	if (init() != 0)
		return;
		
	if (!m_audio_src)
		lib::logger::get_logger()->error(gettext("%s: cannot open"), repr(url).c_str());
	
	// Ugly hack to get the resampler.
	if (m_audio_src) {
		net::audio_datasource *resample_ds = factory->get_datasource_factory()->new_filter_datasource(url, supported, ds);
		AM_DBG lib::logger::get_logger ()->debug("sdl_audio_renderer::sdl_audio_renderer() (this =0x%x) got resample datasource 0x%x", (void *) this, resample_ds);
		if (resample_ds) {
			m_audio_src = resample_ds;
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer: opened resample datasource !");
		}
	}
}

gui::sdl::sdl_audio_renderer::~sdl_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::~sdl_audio_renderer(0x%x) m_audio_src=0x%x",  this, m_audio_src);		
	if (m_is_playing) {
		m_lock.leave();
		unregister_renderer(this);
		m_lock.enter();
	}
	
	if (m_audio_src) {
		m_audio_src->release();
		m_audio_src = NULL;
	}
	
#ifdef USE_SMIL21
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
#endif                                   
	m_is_playing = false;
	m_lock.leave();
}
#ifdef USE_SMIL21

void
gui::sdl::sdl_audio_renderer::set_intransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gui::sdl::sdl_audio_renderer::start_outtransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}
#endif

int
gui::sdl::sdl_audio_renderer::get_data(int bytes_wanted, Uint8 **ptr)
{
	m_lock.enter();
	
	// turned this of because I think here also happends a get_read_ptr when it should not
	//XXXX sometimes we get this one in News when changing video itmes
	
	assert(m_is_playing);
	int rv;
	*ptr = NULL;
	if (m_is_paused||!m_audio_src) { 
		rv = 0;
		m_read_ptr_called = false;
	} else {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data: m_audio_src->get_read_ptr(), m_audio_src=0x%x, this=0x%x", (void*) m_audio_src, (void*) this);
		m_read_ptr_called = true;
		rv = m_audio_src->size();
		*ptr = (Uint8 *) m_audio_src->get_read_ptr();
		if (rv) assert(*ptr);
		if (rv > bytes_wanted)
			rv = bytes_wanted;
		// Also set volume(s)
		m_volcount = 0;
		if (m_dest) {
			const common::region_info *info = m_dest->get_info();
			double level = info ? info->get_soundlevel() : 1.0;
#ifdef USE_SMIL21
			if (m_intransition || m_outtransition) {
				level = m_transition_engine->get_volume(level);
			}
#endif                                   
			double leftlevel, rightlevel;
			leftlevel = rightlevel = level;
#ifdef USE_SMIL21
			if (info) {
				common::sound_alignment align = info->get_soundalign();
				if (align == common::sa_left) {
					rightlevel = 0.0;
				}
				if (align == common::sa_right) {
					leftlevel = 0.0;
				}
			}
#endif
			if (leftlevel == 1.0 && rightlevel == 1.0)
				m_volcount = 0;
			else if (leftlevel == rightlevel) {
				m_volcount = 1;
				m_volumes[0] = leftlevel;
			} else {
				m_volcount = 2;
				m_volumes[0] = leftlevel;
				m_volumes[1] = rightlevel;
			}
		}
	}
	m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data(0x%x) return rv=%d, m_volcount=%d, m_volumes=[%f,%f]",this,rv,m_volcount,m_volcount>=1?m_volumes[0]:1,m_volcount==2?m_volumes[1]:m_volcount==1?0:1);
	return rv;
}

void
gui::sdl::sdl_audio_renderer::get_data_done(int size)
{
	m_lock.enter();
	// Acknowledge that we are ready with the data provided to us
	// at the previous callback time
	//AM_DBG if (m_audio_src) lib::logger::get_logger()->debug("sdl_audio_renderer::get_data_done: m_src->readdone(%d), %d more", size, m_audio_src->size()-size);
	//if (size) {
	if (m_audio_src) {

		if (m_read_ptr_called) {
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data_done: calling m_audio_src->readdone(%d) m_audio_src=0x%x, this = (x%x)", size, (void*) m_audio_src, (void*) this);
			m_audio_src->readdone(size);
			m_read_ptr_called = false;
		}

	}
	bool still_busy;
	still_busy = (size != 0);
	still_busy |= restart_audio_input();
	if (!still_busy) {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::playdone: calling m_context->stopped() this = (x%x)",this);
		// We cannot call unregister_renderer from here, because we are called from the
		// SDL callback and already holding the m_global_lock. So, in stead
		// we use the event processor to unregister ourselves later.
		lib::event *e = new readdone_callback(this, &sdl_audio_renderer::stop);
		m_event_processor->add_event(e, 0, ambulant::lib::ep_med);
		if (m_audio_src) {
			m_audio_src->stop();
			m_audio_src->release();
			m_audio_src = NULL;
		}
		m_lock.leave();
		if (m_context) {
			m_context->stopped(m_cookie, 0);
		} else {
			AM_DBG lib::logger::get_logger()->trace("sdl_audio_renderer(0x%x): m_context is  NULL", (void*)this);
		}
		return;
	}
	m_lock.leave();
}

bool
gui::sdl::sdl_audio_renderer::restart_audio_input()
{
	// private method - no need to lock.
	if (!m_audio_src || m_audio_src->end_of_file() || !m_is_playing) {
		// No more data.
		return false;
	}
	if (m_audio_src->size() < s_min_buffer_size_bytes ) {
		// Start reading 
		lib::event *e = new readdone_callback(this, &sdl_audio_renderer::data_avail);
		m_audio_src->start(m_event_processor, e);
	}
	return true;
}

void
gui::sdl::sdl_audio_renderer::data_avail()
{
	m_lock.enter();
	//assert(m_audio_src);
	if (!m_audio_src) {				
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail:m_audio_src does not exist");
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail: %d bytes available", m_audio_src->size());
	
	restart_audio_input();
	
	m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail: done");
}	

bool
gui::sdl::sdl_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	rv = m_is_paused;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	rv = !m_is_playing;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	rv = m_is_playing;
	m_lock.leave();
	return rv;
}


void
gui::sdl::sdl_audio_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::stop(0x%x)",(void*)this);
	if (m_is_playing) {
		m_lock.leave();
		unregister_renderer(this);
		// XXX Should we call stopped_callback?
		m_context->stopped(m_cookie, 0);
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
gui::sdl::sdl_audio_renderer::pause()
{
	m_lock.enter();
	m_is_paused = true;
	m_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::resume()
{
	m_lock.enter();
	m_is_paused = false;
	m_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::start(double where)
{
	m_lock.enter();
    if (!m_node) abort();
		
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.start(0x%x)", (void *)this);
	if (m_audio_src) {
	
		if (m_audio_src->get_start_time() != m_audio_src->get_clip_begin())
			lib::logger::get_logger()->trace("sdl_audio_renderer: warning: datasource does not support clipBegin");
			
		lib::event *e = new readdone_callback(this, &sdl_audio_renderer::data_avail);
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::start(): m_audio_src->start(0x%x, 0x%x) this = (x%x)m_audio_src=0x%x", (void*)m_event_processor, (void*)e, this, (void*)m_audio_src);
		m_audio_src->start(m_event_processor, e);
		m_is_playing = true;
		m_is_paused = false;
		m_lock.leave();
		register_renderer(this);
#ifdef USE_SMIL21
		if (m_intransition && ! m_transition_engine) {
			m_transition_engine = new smil2::audio_transition_engine();
			m_transition_engine->init(m_event_processor, false, m_intransition);
		}
#endif                                   
	} else {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.start: no datasource");
		m_lock.leave();
		m_context->stopped(m_cookie, 0);
	}
}

void
gui::sdl::sdl_audio_renderer::seek(double where)
{
	lib::logger::get_logger()->trace("sdl_audio_renderer: seek(%f) not implemented", where);
}

common::duration 
gui::sdl::sdl_audio_renderer::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	if (m_audio_src)
		rv = m_audio_src->get_dur();
	m_lock.leave();
	return rv;
}
