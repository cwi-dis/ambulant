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

 
#include "ambulant/gui/dx/dx_player_impl.h"

#include "ambulant/common/player.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

using namespace ambulant;

gui::dx::dx_player_impl::dx_player_impl(const std::string& url, VCF f) 
:	m_url(url),
	m_create_viewport_fn(f),
	m_viewport(0),
	m_wf(0), 
	m_rf(0),
	m_pplayer(0), 
	m_aplayer(0),
	m_logger(lib::logger::get_logger()) {
		
}

gui::dx::dx_player_impl::~dx_player_impl() {
	m_aplayer = lib::release(m_aplayer);
	// verify:
	if(m_aplayer != 0)
		m_logger->warn("active_player ref_count: %ld" + m_aplayer->get_ref_count());
	delete m_pplayer;
	delete m_rf;
	delete m_wf;
	delete m_viewport;
}

gui::dx::viewport* gui::dx::dx_player_impl::create_viewport(int w, int h) {
	if(m_create_viewport_fn)
		return (*m_create_viewport_fn)(w, h);
	if(!m_viewport)
		m_viewport = new viewport(w, h, 0);
	return m_viewport;
}

bool gui::dx::dx_player_impl::is_done() const { 
	return m_aplayer && m_aplayer->is_done();
}

bool gui::dx::dx_player_impl::start() {
	m_logger->trace("Attempting to play: %s", m_url.c_str());

	// Create passive_player from filename
	m_pplayer = new lib::passive_player(m_url.c_str());
	if (!m_pplayer) {
		m_logger->error("Failed to construct passive_player from file %s", m_url.c_str());
		return false;
	}
	
	// Create GUI window_factory and renderer_factory
	m_wf = new gui::dx::dx_window_factory(this);
	m_rf = new gui::dx::dx_renderer_factory(this);
	
	// Request an active_player for the provided factories 
	m_aplayer = m_pplayer->activate(m_wf, m_rf);
	if (!m_aplayer) {
		m_logger->error("passive_player::activate() failed to create active_player");
		return false;
	}
	
	// Pass a flag_event to be set when done.
	m_aplayer->start();
	m_logger->trace("Started playing");
	return  true;
}

void gui::dx::dx_player_impl::stop() {
	m_logger->trace("Attempting to stop: %s", m_url.c_str());
	if(m_aplayer) m_aplayer->stop();
	if(m_viewport) m_viewport->redraw();
}

void gui::dx::dx_player_impl::pause() {
	m_logger->trace("Attempting to pause: %s", m_url.c_str());
	if(m_aplayer)
		m_aplayer->pause();
}

void gui::dx::dx_player_impl::resume() {
	m_logger->trace("Attempting to resume: %s", m_url.c_str());
	if(m_aplayer)
		m_aplayer->resume();
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url) {
	return new dx_player_impl(url, 0);
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url, VCF f) {
	return new dx_player_impl(url, f);
}

 

