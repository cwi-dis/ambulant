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

#include "ambulant/gui/dg/dg_audio.h"
#include "ambulant/gui/dg/dg_audio_player.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/memfile.h"

#include "ambulant/common/region.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::dg_audio_renderer::dg_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::gui_window *window,
	lib::event_processor* worker)
:   common::renderer_playable(context, cookie, node, evp), 
	m_player(0), 
	m_update_event(0), 
	m_worker(worker) {
	
	AM_DBG lib::logger::get_logger()->trace("dg_audio_renderer(0x%x)", this);
	net::url url = m_node->get_url("src");
	if(url.is_local_file() && lib::win32::file_exists(url.get_file()))
		m_player = new gui::dg::audio_player(url.get_file());
	else {
		lib::logger::get_logger()->error("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
}

gui::dg::dg_audio_renderer::~dg_audio_renderer() {
	AM_DBG lib::logger::get_logger()->trace("~dg_audio_renderer()");
	if(m_player) {
		m_player->stop();
		delete m_player;	
	}
}

void gui::dg::dg_audio_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->trace("dg_audio_renderer::start(0x%x)", this);
	
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
		// And the user
		lib::logger::get_logger()->show("Unsupported audio format");
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
		
	// Start the underlying player
	m_player->start(t);
		
	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);
		
	// Schedule a self-update
	schedule_update();
}

std::pair<bool, double> gui::dg::dg_audio_renderer::get_dur() {
	if(m_player) return m_player->get_dur();
	return std::pair<bool, double>(false, 0.0);
}

void gui::dg::dg_audio_renderer::stop() {
	AM_DBG lib::logger::get_logger()->trace("dg_audio_renderer.stop(0x%x)", this);
	if(!m_player) return;
	m_cs.enter();
	audio_player *p = m_player;
	m_player = 0;
	m_update_event = 0;
	m_activated = false;
	m_cs.leave();
	p->stop();
	delete p;
	
}

void gui::dg::dg_audio_renderer::pause() {
	AM_DBG lib::logger::get_logger()->trace("dg_audio_renderer.pause(0x%x)", this);
	if(m_player) m_player->pause();
}

void gui::dg::dg_audio_renderer::resume() {
	lib::logger::get_logger()->trace("dg_audio_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
}

void gui::dg::dg_audio_renderer::redraw(const lib::screen_rect<int> &dirty, common::gui_window *window) {
	// we don't have any bits to blit for audio
}

void gui::dg::dg_audio_renderer::update_callback() {
	m_cs.enter();
	if(!m_update_event || !m_player) {
		m_cs.leave();
		return;
	}
	if(m_player->is_playing()) {
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
	m_cs.leave();
}

void gui::dg::dg_audio_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dg_audio_renderer>(this, 
		&dg_audio_renderer::update_callback);
	m_worker->add_event(m_update_event, 100);
}
