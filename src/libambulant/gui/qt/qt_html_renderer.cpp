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
#ifdef	WITH_QT_HTML_WIDGET

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_html_renderer.h"
#include "ambulant/smil2/params.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

qt_html_renderer::qt_html_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory)
  :	renderer_playable(context, cookie, node, evp)
  ,	m_html_browser(NULL) 
{
    
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer(0x%x)",this);
}

void 
gui::qt::qt_html_renderer::start(double t) {
 	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::start(0x%x)", this);
}

qt_html_renderer::~qt_html_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~qt_html_renderer(0x%x)", this);
	if (m_html_browser) {
// the following hide() causes flicker, is needed to clear at the end
		m_html_browser->hide();
//	m_html_widget is not deleted, but remembered in the surface_impl
//		delete m_html_browser;
		m_html_browser = NULL;
	}
	m_lock.leave();
}

void
gui::qt::qt_html_renderer::set_surface(common::surface *dest) {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::set_surface(0x%x) dest=0x%x", this, dest);

	m_dest = dest;
	
	lib::rect rc = dest->get_rect();
	const lib::point p = dest->get_global_topleft();
	m_url = m_node->get_url("src");

// following test works on WIN32, fails on linux when url contains #tag
//	if(!lib::memfile::exists(m_url)) {
//		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
//			m_url.get_url().c_str());
//		m_lock.leave();
//		return;
//	}
	if(m_activated) {
		// repeat
		m_dest->need_redraw();
		m_lock.leave();
		return;	
	}
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	// Currently already done by show()
	// m_dest->need_redraw();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
gui::qt::qt_html_renderer::stop() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::stop(0x%x)", this);
// the following hide() causes flicker
//	if (m_html_browser)
//		m_html_browser->hide();
	m_dest->renderer_done(this);
	m_activated = false;
	m_lock.leave();
}

void
gui::qt::qt_html_renderer::user_event(const lib::point& pt, int what) {
	m_lock.enter();
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	m_lock.leave();
}

void
gui::qt::qt_html_renderer::redraw(const lib::rect& r, common::gui_window *window) {
	m_lock.enter();
	const lib::point p = m_dest->get_global_topleft();
	AM_DBG lib::logger::get_logger()->debug(
		"qt_html_renderer.redraw(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_url = %s, p=(%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom(),
		(const char*) m_url.get_url().c_str(),
		p.x, p.y);
	// Get the top-level surface
	ambulant_qt_window* aqw = (ambulant_qt_window*)window;
	qt_ambulant_widget* qaw = aqw->get_ambulant_widget();

	if(!qaw) return;
	
	if ( ! m_html_browser) {
		//XXXX for some reason the pointer to the browser is stored in the parent of the current surface node
		common::surface_impl* parent = ((common::surface_impl*)m_dest)->get_parent();
		// Parent can be NULL, when playing on the default region
		if (parent == NULL) parent = (common::surface_impl*)m_dest;
		m_html_browser = (KHTMLPart*) parent->get_renderer_data(parent);
		if (m_html_browser == NULL) {
			m_html_browser = new KHTMLPart((QWidget*)qaw);
			parent->set_renderer_data(parent, m_html_browser);
		}
	}
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::redraw_body(0x%x) html_browser=0x%x",this,m_html_browser);
	assert(m_html_browser != NULL);
	m_html_browser->openURL(m_url.get_url().c_str());
	m_html_browser->view()->resize(r.width(),r.height());

	m_html_browser->show();
	m_lock.leave();
}
#endif/*WITH_QT_HTML_WIDGET*/
