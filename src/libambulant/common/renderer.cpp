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

/* 
 * @$Id$ 
 */

#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"

#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/net/posix_datasource.h"
#else
#include "ambulant/net/stdio_datasource.h"
inline double round(double v) {return floor(v+0.5);}
#endif
#ifndef round
#define round(v) ((int)(v+0.5))
#endif


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

typedef lib::no_arg_callback<renderer_playable_ds> readdone_callback;

renderer_playable_ds::renderer_playable_ds(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	renderer_playable(context, cookie, node, evp),
	m_is_showing(false),
	m_src(NULL)
{
	// XXXX m_src = passive_datasource(node->get_url("src"))->activate()
	net::url url = node->get_url("src");
	m_src = factory->df->new_raw_datasource(url);	
}

renderer_playable_ds::~renderer_playable_ds()
{
	AM_DBG lib::logger::get_logger()->debug("~renderer_playable_ds(0x%x)", (void *)this);
}

void
renderer_playable_ds::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.start(0x%x)", (void *)this);
	if (m_is_showing) {
		lib::logger::get_logger()->trace("renderer_playable_ds.start(0x%x): already started", (void*)this);
		return;
	}
	m_is_showing = true;
	if (!m_dest) {
		lib::logger::get_logger()->trace("renderer_playable_ds.start: no destination surface, skipping media item");
		m_context->stopped(m_cookie, 0);
		return;
	}
	m_dest->show(this);
	if (m_src) {
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
	} else {
		lib::logger::get_logger()->trace("renderer_playable_ds.start: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
	}
}


#if 0
void
renderer_playable_ds::readdone()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	if (m_dest)
		m_dest->need_redraw();
	m_context->stopped(m_cookie, 0);
}
#endif

void
renderer_playable_ds::seek(double t)
{
	lib::logger::get_logger()->trace("renderer_playable_ds: seek(%f) not implemented", t);
}

void
renderer_playable_ds::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.stop(0x%x)", (void *)this);
	if (!m_is_showing) {
		lib::logger::get_logger()->trace("renderer_playable_ds.stop(0x%x): not started", (void*)this);
	} else {
		if (m_dest)
			m_dest->renderer_done(this);
	}
	m_is_showing = false;
	if (m_src)
		m_src->stop();
	m_src = NULL;
}

#if 0
void
renderer_playable_ds::wantclicks(bool want)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds(0x%x)::wantclicks(%d)", (void*)this, want);
	if (m_dest)
		m_dest->need_events(want);
}
#endif

renderer_playable_dsall::~renderer_playable_dsall()
{
	if (m_data) free(m_data);
}

void
renderer_playable_dsall::readdone()
{
	unsigned cur_size = m_src->size();
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x, size=%d)", (void *)this, cur_size);
	
	if (!m_data)
		m_data = malloc(cur_size);
	else
		m_data = realloc(m_data, m_data_size + cur_size);
	
	if (m_data == NULL) {
		lib::logger::get_logger()->fatal("renderer_playable_dsall.readdone: cannot allocate %d bytes", cur_size);
		// What else can we do...
		m_context->stopped(m_cookie, 0);
	}
	
	char *cur_data = m_src->get_read_ptr();
	memcpy((char *)m_data + m_data_size, cur_data, cur_size);
	m_data_size += cur_size;
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->readdone(%d)", (void *)this,m_data_size);
	m_src->readdone(cur_size);
	
	if (m_src->end_of_file()) {
		// All done
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  all done, calling need_redraw() and stopped_callback", (void *)this);
		if (m_dest)
			m_dest->need_redraw();
		m_src->stop();
		m_context->stopped(m_cookie, 0);
	} else {
		// Continue reading
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  more to come, calling m_src->start()", (void *)this);
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
	}
	
}

global_playable_factory::global_playable_factory()
:   m_default_factory(new gui::none::none_playable_factory())
{
}

global_playable_factory::~global_playable_factory()
{
    // XXXX Should I delete the factories in m_factories? I think
    // so, but I'm not sure...
    delete m_default_factory;
}
    
void
global_playable_factory::add_factory(playable_factory *rf)
{
    m_factories.push_back(rf);
}
    
playable *
global_playable_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_playable(context, cookie, node, evp);
        if (rv) return rv;
    }
    return m_default_factory->new_playable(context, cookie, node, evp);
}

playable *
global_playable_factory::new_aux_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::audio_datasource *src)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_aux_audio_playable(context, cookie, node, evp, src);
        if (rv) return rv;
    }
	
    return m_default_factory->new_playable(context, cookie, node, evp);
}



void 
active_video_renderer::redraw(const lib::screen_rect<int> &dirty, common::gui_window *window)
{
	AM_DBG lib::logger::get_logger ()->debug("active_video_renderer::redraw (this = 0x%x)", (void *) this);
}

active_video_renderer::active_video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node * node,
	lib::event_processor * evp,
	common::factories *factory)
:	renderer_playable (context, cookie, node, evp),
	m_src(NULL),
	m_audio_ds(NULL),
	m_audio_renderer(NULL),
	m_is_playing(false),
	m_is_paused(false)
	
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger ()->debug("active_video_renderer::active_video_renderer() (this = 0x%x): Constructor ", (void *) this);
	// XXXX FIXME : The path to the jpg's is fixed !!!!!
	net::url url = node->get_url("src");
	m_src = factory->df->new_video_datasource(url);
	if (m_src == NULL) {
		lib::logger::get_logger ()->warn("active_video_renderer::active_video_renderer(): Cannot open video");
	}

	AM_DBG lib::logger::get_logger ()->debug("active_video_renderer::active_video_renderer() leaving Constructor !(m_src = 0x%x)", (void *) m_src);
	m_lock.leave();
}

void
active_video_renderer::stop()
{ 
	m_lock.enter();
	m_is_playing = false;
	if (m_dest)
	  m_dest->renderer_done(this);
	m_dest = NULL;
	if (m_audio_renderer) 
		m_audio_renderer->stop();
	m_audio_renderer = NULL;
	m_lock.leave();
}


void
active_video_renderer::start (double where = 1)
{
	m_lock.enter();
	int w;

	if (m_audio_renderer) 
		m_audio_renderer->start(where);
		
	m_is_playing = true;
	m_epoch = m_event_processor->get_timer()->elapsed();
	w = (int) round (where);
	lib::event * e = new dataavail_callback (this, &active_video_renderer::data_avail);
	AM_DBG lib::logger::get_logger ()->debug ("active_video_renderer::start(%d) (this = 0x%x) ", w, (void *) this);
	if (!m_src) {
		lib::logger::get_logger()->trace("active_video_renderer.start: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
		m_lock.leave();
		return;
	}
	if (m_dest) {
		m_dest->show(this);
	} else {
		AM_DBG lib::logger::get_logger ()->debug ("active_video_renderer::start(%d) (this = 0x%x) m_dest == NULL", w, (void *) this);
	}
	m_src->start_frame (m_event_processor, e, w);
	AM_DBG lib::logger::get_logger ()->debug ("active_video_renderer::start(%d) (this = 0x%x) m_src(0x%x)->start called", w, (void *) this, (void*) m_src);
	m_lock.leave();
}

void
active_video_renderer::seek(double t)
{
	lib::logger::get_logger()->trace("active_video_renderer: seek(%f) not implemented", t);
}



// now() returns the time in seconds !
double
active_video_renderer::now() 
{
	// private method - no locking
	double rv;
	if (m_is_paused)
		rv = (double)(m_paused_epoch - m_epoch) / 1000;
	else
		rv = ((double)m_event_processor->get_timer()->elapsed() - m_epoch)/1000;
	return rv;
}

void
active_video_renderer::pause()
{
	m_lock.enter();
	if (!m_is_paused) {
		if (m_audio_renderer) 
			m_audio_renderer->pause();
		m_is_paused = true;
		m_paused_epoch = m_event_processor->get_timer()->elapsed();
	}
	m_lock.leave();
}

void
active_video_renderer::resume()
{
	m_lock.enter();
	if (m_is_paused) {
		if (m_audio_renderer) 
			m_audio_renderer->resume();
		m_is_paused = false;
		unsigned long int pause_length = m_event_processor->get_timer()->elapsed() - m_paused_epoch;
		m_epoch += pause_length;
	}
	m_lock.leave();
}



void
active_video_renderer::data_avail()
{
	m_lock.enter();
	double ts;
	char *buf = NULL;
	int size;
	unsigned long int event_time;
	bool displayed;
	
	if (!m_src) {
		lib::logger::get_logger()->trace("active_video_renderer.data_avail: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
		m_lock.leave();
		return;
	}
	
	AM_DBG lib::logger::get_logger()->debug("active_video_renderer::data_avail(this = 0x%x):", (void *) this);
	m_size.w = m_src->width();
	m_size.h = m_src->height();
	AM_DBG lib::logger::get_logger()->debug("active_video_renderer::data_avail: size=(%d, %d)", m_size.w, m_size.h);
	buf = m_src->get_frame(&ts, &size);
	displayed = false;
	AM_DBG lib::logger::get_logger()->debug("active_video_renderer::data_avail(buf = 0x%x) (ts=%f, now=%f):", (void *) buf,ts, now());	
	if (m_is_playing && buf) {
		if (ts <= now()) {
			AM_DBG lib::logger::get_logger()->debug("**** (this = 0x%x) Calling show_frame() timestamp : %f, now = %f (located at 0x%x) ", (void *) this, ts, now(), (void *) buf);
			show_frame(buf, size);
			m_dest->need_redraw();
			displayed = true;
			m_src->frame_done(ts, true);
			if (!m_src->end_of_file()) {
				lib::event * e = new dataavail_callback (this, &active_video_renderer::data_avail);
				m_src->start_frame (m_event_processor, e, ts);
			}
		} else {
			lib::event * e = new dataavail_callback (this, &active_video_renderer::data_avail);
			event_time = (unsigned long int) round( 1 + ts*1000 - now()*1000); 
			m_event_processor->add_event(e, event_time);
		}
	} else {
		if (m_is_playing && !m_src->end_of_file()) {
			lib::logger::get_logger()->debug("active_video_renderer::data_avial: No more data, but not end of file!");
		}
		AM_DBG lib::logger::get_logger ()->debug("active_video_renderer::data_avail(this = 0x%x): end_of_file ", (void *) this);
		m_is_playing = false;
		m_lock.leave();
		m_context->stopped(m_cookie, 0);
		return;
	}
	m_lock.leave();
}
