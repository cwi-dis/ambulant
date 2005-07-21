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

#include "ambulant/gui/dx/dx_audio.h"
#include "ambulant/gui/dx/dx_audio_player.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"

#include "ambulant/common/region.h"


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_audio_renderer::dx_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::gui_window *window,
	lib::event_processor* worker)
:   common::renderer_playable(context, cookie, node, evp), 
	m_player(0), 
	m_update_event(0), 
	m_worker(worker),
	m_level(1.0)
#ifdef USE_SMIL21
	, m_balance(0),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif
{
	
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer(0x%x)", this);
	net::url url = m_node->get_url("src");
	if(url.is_local_file() && lib::win32::file_exists(url.get_file()))
		m_player = new gui::dx::audio_player(url.get_file());
	else if(url.is_absolute())
		m_player = new gui::dx::audio_player(url);
	else {
		lib::logger::get_logger()->error("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
}

gui::dx::dx_audio_renderer::~dx_audio_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_audio_renderer()");
	if(m_player) stop();
#ifdef USE_SMIL21
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
#endif                                   
}

void gui::dx::dx_audio_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer::start(0x%x)", this);
	
	if(!m_player) {
		// Not created or stopped (gone)
		
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
	
	// Does it have all the resources to play?
	if(!m_player->can_play()) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
	
	// Already activated
	if(m_activated) {
		// repeat
		m_player->start(t);
		schedule_update();
		return;	
	}
	
	// Activate this renderer.
	m_activated = true;
		
#ifdef USE_SMIL21
	if (m_intransition && !m_transition_engine) {
		m_transition_engine = new smil2::audio_transition_engine();
		m_transition_engine->init(m_event_processor, false, m_intransition);
	}
#endif                                   
	// And set volume(s)
	update_levels();

	// Start the underlying player
	m_player->start(t);
		
	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);
		
	// Schedule a self-update
	schedule_update();
}

void gui::dx::dx_audio_renderer::update_levels() {
	if (!m_dest) return;
	const common::region_info *info = m_dest->get_info();
	double level = info ? info->get_soundlevel() : 1;
#ifdef USE_SMIL21
	if (m_intransition || m_outtransition) {
		level = m_transition_engine->get_volume(level);
	}
#endif                                   
	if (level != m_level)
		m_player->set_volume((long)(level*100));
	m_level = level;
#ifdef USE_SMIL21
	common::sound_alignment align = info ? info->get_soundalign() : common::sa_default;
	int balance = 0;
	
	if (align == common::sa_left) {
		balance = -100;
	} else if (align == common::sa_right) {
		balance = 100;
	}
	if (balance != m_balance)
		m_player->set_balance(balance);
	m_balance = balance;
#endif
}

void
gui::dx::dx_audio_renderer::set_intransition(const lib::transition_info* info) {
#ifdef USE_SMIL21
 	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
#endif // USE_SMIL21
}

void
gui::dx::dx_audio_renderer::start_outtransition(const lib::transition_info* info) {
#ifdef USE_SMIL21
 	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
#endif // USE_SMIL21
}

void gui::dx::dx_audio_renderer::seek(double t) {
	if (m_player) m_player->seek(t);
}
std::pair<bool, double> gui::dx::dx_audio_renderer::get_dur() {
	if(m_player) return m_player->get_dur();
	return std::pair<bool, double>(false, 0.0);
}

void gui::dx::dx_audio_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.stop(0x%x)", this);
	if(!m_player) return;
	audio_player *p = m_player;
	m_player = 0;
	m_update_event = 0;
	p->stop();
	delete p;
	m_activated = false;
}

void gui::dx::dx_audio_renderer::pause() {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.pause(0x%x)", this);
	if(m_player) m_player->pause();
}

void gui::dx::dx_audio_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
}

void gui::dx::dx_audio_renderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	// we don't have any bits to blit for audio
}

void gui::dx::dx_audio_renderer::update_callback() {
	if(!m_update_event || !m_player) {
		return;
	}
	if(m_player->is_playing()) {
		update_levels();
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::dx::dx_audio_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dx_audio_renderer>(this, 
		&dx_audio_renderer::update_callback);
	m_event_processor->add_event(m_update_event, 100, lib::event_processor::high);
}
