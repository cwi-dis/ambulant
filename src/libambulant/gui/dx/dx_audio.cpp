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

#include "ambulant/common/region.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

gui::dx::dx_audio_renderer::dx_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::abstract_window *window)
:   common::active_renderer(context, cookie, node, evp), 
	m_window(window),
	m_player(0) {
	
	// create player so that get_dur() succeeds
	if(m_src->exists())
		m_player = new gui::dx::audio_player(m_src->get_url());
	else {
		lib::logger::get_logger()->error("The location specified for the data source does not exist. [%s]",
			m_src->get_url().c_str());
	}
}

gui::dx::dx_audio_renderer::~dx_audio_renderer() {
	lib::logger::get_logger()->trace("~dx_audio_renderer()");
	if(m_player) stop();
}

void gui::dx::dx_audio_renderer::start(double t) {
	if(!m_src->exists()) {
		stopped_callback();
	} else if(m_player) {
		//lib::show_message("Starting audio %s at %f", m_src->get_url().c_str(), t);
		m_player->start(t);
	}
}

std::pair<bool, double> gui::dx::dx_audio_renderer::get_dur() {
	if(m_player) return m_player->get_dur();
	return std::pair<bool, double>(false, 0.0);
}

void gui::dx::dx_audio_renderer::stop() {
	lib::logger::get_logger()->trace("dx_audio_renderer.stop(0x%x)", this);
	if(m_player) {
		m_player->stop();
		delete m_player;
		m_player = 0;
	}
}
void gui::dx::dx_audio_renderer::pause() {
	lib::logger::get_logger()->trace("dx_audio_renderer.pause(0x%x)", this);
	if(m_player) m_player->pause();
}
void gui::dx::dx_audio_renderer::resume() {
	lib::logger::get_logger()->trace("dx_audio_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
}
void gui::dx::dx_audio_renderer::redraw(const lib::screen_rect<int> &dirty, common::abstract_window *window) {
	// we don't need to do anything for audio
}

