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
 e
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

#include "ambulant/gui/arts/arts_audio.h"

using namespace ambulant;
using namespace gui::arts;

typedef lib::no_arg_callback<gui::arts::arts_active_audio_renderer> readdone_callback;
net::audio_format gui::arts::arts_active_audio_renderer::m_ambulant_format = net::audio_format(44100, 2, 16);

bool arts_active_audio_renderer::m_arts_init = false;
 
arts_active_audio_renderer::arts_active_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory)
:	common::playable_imp(context, cookie, node, evp),
	m_rate(44100),
	m_channels(2),
	m_bits(16),
	m_stream(NULL),
	m_audio_src(NULL),
	m_is_paused(false),
	m_is_playing(false)
{
	m_lock.enter();
	//init();
	arts_setup(m_rate,m_bits,m_channels,"arts_audio");
	net::audio_format_choices supported = net::audio_format_choices(m_ambulant_format);
	net::url url = node->get_url("src");
	m_audio_src = factory->df->new_audio_datasource(url, supported, 0, -1);
	if (!m_audio_src)
		lib::logger::get_logger()->error("arts_active_audio_renderer: cannot open %s", repr(url).c_str());
	else if (!supported.contains(m_audio_src->get_audio_format())) {
		lib::logger::get_logger()->error("arts_active_audio_renderer: %s: unsupported format", repr(url).c_str());
		m_audio_src->release();
		m_audio_src = NULL;
	}
	m_lock.leave();
}

arts_active_audio_renderer::arts_active_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	net::audio_datasource *ds)
:	common::playable_imp(context, cookie, node, evp),
	m_rate(44100),
	m_channels(2),
	m_bits(16),
	m_stream(NULL),
	m_audio_src(ds),
	m_is_paused(false),
	m_is_playing(false)
{
	m_lock.enter();
	//init();
	arts_setup(m_rate,m_bits,m_channels,"arts_audio");
	net::audio_format_choices supported = net::audio_format_choices(m_ambulant_format);
	net::url url = node->get_url("src");
	
	if (!m_audio_src)
		lib::logger::get_logger()->error("arts_active_audio_renderer: cannot open %s", repr(url).c_str());
	
	// Ugly hack to get the resampler.
	if (m_audio_src) {
		net::audio_datasource *resample_ds = factory->df->new_filter_datasource(url, supported, ds);
		AM_DBG lib::logger::get_logger ()->debug("arts_active_audio_renderer::arts_active_audio_renderer() (this =0x%x) got resample datasource 0x%x", (void *) this, resample_ds);
		if (resample_ds) {
			m_audio_src = resample_ds;
			AM_DBG lib::logger::get_logger()->debug("arts_active_audio_renderer: opened resample datasource !");
		}
	}
	
	m_lock.leave();
}


int
arts_active_audio_renderer::init()
{
    int err;
    if (!m_arts_init) {
        err  = arts_init();
        m_arts_init  = true;
        AM_DBG lib::logger::get_logger()->debug("active_renderer.arts_setup(0x%x): initialising aRts", (void *)this);
    } else {
        err = 0;
    }
    return err;
}

int
arts_active_audio_renderer::arts_setup(int rate, int bits, int channels, char *name)
{
    int err;
    if (!m_stream) {
        //err = arts_init();
        err = init();
    	if (err < 0) {
        	AM_DBG lib::logger::get_logger()->error("active_renderer.arts_setup(0x%x): %s", (void *)this, arts_error_text(err));
        	return err;
    	}
    	m_stream = arts_play_stream(rate, bits, channels, name);
		if (!m_stream) {
			AM_DBG lib::logger::get_logger()->error("active_renderer.arts_setup(0x%x): m_stream == NULL");
		}
    	return 0;
    }
}

arts_active_audio_renderer::~arts_active_audio_renderer()
{
    arts_close_stream(m_stream);
}


bool
arts_active_audio_renderer::restart_audio_input()
{	
	// private method - no need to lock.
	if (!m_audio_src || m_audio_src->end_of_file()) {
		// No more data.
		AM_DBG lib::logger::get_logger()->debug("arts_plugin::restart_audio_input(0x%x): no more data",(void*) this);
		m_is_playing=false;
		if(m_audio_src) {
			m_audio_src->stop();
			m_audio_src->release();
			m_audio_src=NULL;
		}
		if (m_context) 
			m_context->stopped(m_cookie, 0);
		return false;
	}
	if (m_audio_src->size() == 0) {
		// Start reading 
		lib::event *e = new readdone_callback(this, &arts_active_audio_renderer::data_avail);
		m_audio_src->start(m_event_processor, e);
	}
	return true;
}

int
arts_active_audio_renderer::arts_play(char *data, int size)
{
    int err;
    if (m_stream) {
        err = arts_write(m_stream, data, size);
        if (err < 0) {
            AM_DBG lib::logger::get_logger()->error("arts_plugin::arts_play(0x%x): %s", (void *)this, arts_error_text(err));
			return 0;
		}
		if (err < size) {
			//int delay = arts_stream_get (m_stream, ARTS_P_TOTAL_LATENCY);
			int delay = 15;
			AM_DBG lib::logger::get_logger()->debug("arts_plugin::arts_play(0x%x): aRts buffer full, delaying %dms", (void *)this,delay);
			lib::event *e = new readdone_callback(this, &arts_active_audio_renderer::data_avail);
			m_event_processor->add_event(e,delay,ambulant::lib::event_processor::med);
		}
     } else {
        AM_DBG lib::logger::get_logger()->error("arts_plugin::arts_play(0x%x): No aRts stream opened", (void *)this);        
		return 0;	
     }
        
    return err;
}

void
arts_active_audio_renderer::data_avail()
{
	
	m_lock.enter();
    char *data;
    int size;
    int played;
    int err;
    
    AM_DBG lib::logger::get_logger()->debug("arts_plugin::data_avail(): (this=0x%x)", (void *)this);
    AM_DBG lib::logger::get_logger()->debug("arts_plugin::data_avail(): m_audio_src->get_read_ptr() m_audio_src=0x%x, this=0x%x", (void*) m_audio_src, (void *)this);
	data = m_audio_src->get_read_ptr();
	size = m_audio_src->size();
    AM_DBG lib::logger::get_logger()->debug("arts_plugin::data_avail(): (this=0x%x) strarting to play %d bytes", (void *)this, size);
	
	if (!m_is_paused || m_audio_src) {
		played=arts_play(data,size);
		AM_DBG lib::logger::get_logger()->debug("arts_plugin::data_avail():(this=0x%x)  played %d bytes", (void *)this, played);
    	AM_DBG lib::logger::get_logger()->debug("arts_plugin::data_avail(): m_audio_src->readdone(%d) called", size);
		m_audio_src->readdone(played);
	} else {
		if (m_audio_src) m_audio_src->readdone(0);
	}
	restart_audio_input();
    //m_context->stopped(m_cookie, 0);
	m_lock.leave();

}



void
arts_active_audio_renderer::start(double where)
{
	m_lock.enter();
    if (!m_node) abort();

	std::ostringstream os;
	os << *m_node;

	
	AM_DBG lib::logger::get_logger()->debug("arts_plugin::start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_audio_src) {
		m_is_playing = true;
		lib::event *e = new readdone_callback(this, &arts_active_audio_renderer::data_avail);
		m_audio_src->start(m_event_processor, e);

	} else {
		lib::logger::get_logger()->error("arts_plugin::start(0x%x): no datasource", (void *)this);
		if (m_playdone) {
			m_context->stopped(m_cookie, 0);
            //stopped_callback();
        }
	}
	m_lock.leave();
}

void
gui::arts::arts_active_audio_renderer::pause()
{
	m_lock.enter();
	m_is_paused = true;
	m_lock.leave();
}

void
gui::arts::arts_active_audio_renderer::stop()
{
	m_lock.enter();
	m_is_playing=false;
	if(m_audio_src) {
		m_audio_src->stop();
		m_audio_src->release();
		m_audio_src=NULL;
	}
	if (m_context) 
		m_context->stopped(m_cookie, 0);
	
	m_lock.leave();
}

void
gui::arts::arts_active_audio_renderer::resume()
{
	m_lock.enter();
	m_is_paused = false;
	m_lock.leave();
}

std::pair<bool, double> 
gui::arts::arts_active_audio_renderer::get_dur()
{
	//DBG return std::pair<bool, double>(true, 7);
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_audio_src)
		rv = m_audio_src->get_dur();
	m_lock.leave();
	return rv;
}
