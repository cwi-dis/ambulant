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

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/document.h"

#include "ambulant/common/smil_player.h"
#include "ambulant/common/mms_player.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

using namespace ambulant;

gui::dx::dx_mms_player_impl::dx_mms_player_impl(const std::string& url, VCF f) 
:	m_url(url),
	m_create_viewport_fn(f),
	m_viewport(0),
	m_wf(0), 
	m_rf(0),
	m_mms_player(0), 
	m_logger(lib::logger::get_logger()) {
		
}

gui::dx::dx_mms_player_impl::~dx_mms_player_impl() {
	// verify:
	delete m_mms_player;
	delete m_rf;
	delete m_wf;
	delete m_viewport;
}

gui::dx::viewport* gui::dx::dx_mms_player_impl::create_viewport(int w, int h) {
	if(m_create_viewport_fn)
		return (*m_create_viewport_fn)(w, h);
	if(!m_viewport)
		m_viewport = new viewport(w, h, 0);
	return m_viewport;
}

bool gui::dx::dx_mms_player_impl::is_done() const { 
	return m_mms_player == NULL || m_mms_player->is_done();
}

bool gui::dx::dx_mms_player_impl::start() {
	m_logger->trace("Attempting to play: %s", m_url.c_str());

	lib::document *doc = lib::document::create_from_file(m_url);
	if(!doc) {
		m_logger->error("Failed to parse document %s", m_url.c_str());
		return false;
	}
	
	// Create GUI window_factory and renderer_factory
	m_wf = new gui::dx::dx_window_factory(this);
	m_rf = new gui::dx::dx_renderer_factory(this);
	
	m_mms_player = new lib::mms_player(doc, m_wf, m_rf);
	m_mms_player->start();

	m_logger->trace("Started playing");
	return  true;
}

void gui::dx::dx_mms_player_impl::stop() {
	m_logger->trace("Attempting to stop: %s", m_url.c_str());
	if(m_mms_player) m_mms_player->stop();
	if(m_viewport) m_viewport->redraw();
}

void gui::dx::dx_mms_player_impl::pause() {
	m_logger->trace("Attempting to pause: %s", m_url.c_str());
	if(m_mms_player)
		m_mms_player->pause();
}

void gui::dx::dx_mms_player_impl::resume() {
	m_logger->trace("Attempting to resume: %s", m_url.c_str());
	if(m_mms_player)
		m_mms_player->resume();
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url) {
	//return new dx_mms_player_impl(url, 0);
	return new dx_smil_player_impl(url, 0);
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url, VCF f) {
	//return new dx_mms_player_impl(url, f);
	return new dx_smil_player_impl(url, f);
}

////////////////////////////////////////

//////////////////////////////////////////////
//
// EXPERIMENTAL TEST IMPLEMENTATION
// 
//////////////////////////////////////////////

void show(const char *format, ...) {
	va_list	args;
	va_start(args, format);
	char buf[2048] = "";
	vsprintf(buf, format, args);
	va_end(args);
	MessageBox(NULL, buf, "dx_smil_player", MB_OK);
}

gui::dx::dx_smil_player_impl::dx_smil_player_impl(const std::string& url, VCF f) 
:	m_url(url),
	m_create_viewport_fn(f),
	m_viewport(0),
	m_wf(0), 
	m_rf(0),
	m_smil_player(0),
	m_logger(lib::logger::get_logger()) {
}

gui::dx::dx_smil_player_impl::~dx_smil_player_impl() {
	if(m_smil_player) stop();
	delete m_smil_player;
	delete m_rf;
	delete m_wf;
    delete m_viewport;
}

gui::dx::viewport* gui::dx::dx_smil_player_impl::create_viewport(int w, int h) {
	m_logger->trace("dx_smil_player_impl::create_viewport(%d, %d)", w, h);
	if(m_create_viewport_fn)
		return (*m_create_viewport_fn)(w, h);
	if(!m_viewport)
		m_viewport = new viewport(w, h, 0);
	return m_viewport;
}

bool gui::dx::dx_smil_player_impl::is_done() const { 
	if(!m_smil_player) return true;
	return m_smil_player->is_done();
}

bool gui::dx::dx_smil_player_impl::start() {
	m_logger->trace("Attempting to play: %s", m_url.c_str());
	
	lib::document *doc = lib::document::create_from_file(m_url);
	if(!doc) {
		show("Failed to parse document %s", m_url.c_str());
		return false;
	}
	
	// Create GUI window_factory and renderer_factory
	m_wf = new gui::dx::dx_window_factory(this);
	m_rf = new gui::dx::dx_renderer_factory(this);
	m_smil_player = new lib::smil_player(doc, m_wf, m_rf);	
	m_smil_player->start();
	m_logger->trace("Started playing");
	return  true;
}

void gui::dx::dx_smil_player_impl::stop() {
	m_logger->trace("Attempting to stop: %s", m_url.c_str());
	if(m_smil_player) {
		m_smil_player->pause();
		delete m_smil_player;
		m_smil_player = 0;
	}
	//
}

void gui::dx::dx_smil_player_impl::pause() {
	m_logger->trace("Attempting to pause: %s", m_url.c_str());
	if(m_smil_player) m_smil_player->pause();
}

void gui::dx::dx_smil_player_impl::resume() {
	m_logger->trace("Attempting to resume: %s", m_url.c_str());
	if(m_smil_player) m_smil_player->resume();
}

void gui::dx::dx_smil_player_impl::on_click(int x, int y) {
	POINT pt = {0, 0}; // margins
	if(m_smil_player)
		m_smil_player->on_click(x-pt.x, y-pt.y);
}
	
void gui::dx::dx_smil_player_impl::on_char(int ch) {
	if(m_smil_player)
		m_smil_player->on_char(ch);
}
