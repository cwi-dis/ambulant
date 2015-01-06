// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gstreamer/gstreamer_audio_renderer.h"
#include "ambulant/gui/gstreamer/gstreamer_player.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
using namespace gui::gstreamer;


// ***************** gstreamer_audio_renderer **************************

typedef lib::no_arg_callback<gui::gstreamer::gstreamer_audio_renderer> readdone_callback;

gstreamer_audio_renderer::gstreamer_audio_renderer
(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	common::renderer_playable(context, cookie, node, evp, factory, mdp),
	m_player(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
{
	init_player(node);
}

gstreamer_audio_renderer::gstreamer_audio_renderer
(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories* factory,
	net::audio_datasource *ds)
:	common::renderer_playable(context, cookie, node, evp, factory, NULL),
	m_player(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
{
	init_player(node);
}

gstreamer_audio_renderer::~gstreamer_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::~gstreamer_audio_renderer(0x%x) m_url=%s",  this, m_url.get_url().c_str());
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
	if (m_player) delete m_player;
	m_player = NULL;
	m_lock.leave();
}

void
gstreamer_audio_renderer::init_player(const lib::node *node)
{
	m_lock.enter();
	assert (node);
	m_url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::init_player(0x%x) url=%s",  this, m_url.get_url().c_str());
	_init_clip_begin_end();
	m_player = new gstreamer_player(m_url.get_url().c_str(), this);
	m_lock.leave();
}

void
gstreamer_audio_renderer::set_intransition(const lib::transition_info* info)
{
	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gstreamer_audio_renderer::start_outtransition(const lib::transition_info* info)
{
	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}

bool
gstreamer_audio_renderer::is_supported(const lib::node *node)
{
	if ( ! node)
		return false;
	std::string mimetype(node->get_url("src").guesstype());

	if (mimetype == "audio/mpeg" || mimetype == "audio/wav")
		return true;
	return false;
}

bool
gstreamer_audio_renderer::stop()
{
	m_lock.enter();
	bool rv = _stop();
	m_lock.leave();
	return rv;
}

void
gstreamer_audio_renderer::stopped()
{
	m_lock.enter();
	_stopped();
	m_lock.leave();
}

void
gstreamer_audio_renderer::pause(common::pause_display d)
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.pause(0x%x)", (void *)this);
	m_lock.enter();
	_pause();
	m_lock.leave();
}

void
gstreamer_audio_renderer::resume()
{
	m_lock.enter();
	_resume();
	m_lock.leave();
}

void
gstreamer_audio_renderer::start(double where)
{
	m_lock.enter();
	_start(where);
	m_lock.leave();
}

void
gstreamer_audio_renderer::seek(double where)
{
	m_lock.enter();
	_seek(where);
	m_lock.leave();
}

common::duration
gstreamer_audio_renderer::get_dur()
{
	common::duration rv(false, 0.0);
	double dur = 0.0;

	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.get_dur(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player) {
		dur = m_player->get_dur();
	}
	m_lock.leave();

	if (dur != 0.0) {
		double microsec = 1e6;
		double clip_begin = m_clip_begin / microsec;
		double clip_end   = m_clip_end / microsec;

		if (clip_end > 0 && dur > clip_end)
			dur = clip_end;
		if (clip_begin > 0)
			dur -= clip_begin;
		lib::logger::get_logger()->trace("gstreamer_audio_renderer: get_dur() clip_begin=%f clip_end=%f dur=%f", clip_begin, clip_end, dur);
		rv = common::duration(true, dur);
	}
	return rv;
}

// private functions -- to be called under semaphore protection

void
gstreamer_audio_renderer::_start(double where)
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.start(0x%x): url=%s, where=%f", (void *)this, m_url.get_url().c_str(),where);
	_pause();
	_seek(where);
	_resume(); // turn on playing
	m_context->started(m_cookie, 0);
}

bool
gstreamer_audio_renderer::_stop()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::stop(0x%x)",(void*)this);
	if (m_player) {
		m_player->stop_player();
		m_player = NULL;
	}
	_stopped();
	return false;
}

void
gstreamer_audio_renderer::_stopped()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::stopedp(0x%x)",(void*)this);
	if (m_is_playing) {
		// inform scheduler
		m_context->stopped(m_cookie, 0);
		m_is_playing = false;
	}
}

void
gstreamer_audio_renderer::_pause()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.pause(0x%x)", (void *)this);
	if (m_player)
		m_player->pause();
	m_is_paused = true;
}

void
gstreamer_audio_renderer::_resume()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.resume(0x%x)", (void *)this);
	if (m_player)
		m_player->play();
	m_is_playing = true;
	m_is_paused = false;
}

void
gstreamer_audio_renderer::_seek(double where)
{
	double microsec = 1e6;
	AM_DBG  lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek(%f) NOT YET IMPLEMENETD", where);
	where += (m_clip_begin / microsec);
	if (m_player)
		m_player->seek(where);
}

