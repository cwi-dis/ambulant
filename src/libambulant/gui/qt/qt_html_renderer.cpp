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

#define AM_DBG
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
  :	qt_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory)
  ,	m_url(NULL) 
  ,	m_browser(NULL) 
{
	m_url = new net::url(node->get_url("src"));
    
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer(0x%x) m_url=%s",this,m_url->get_url().c_str());
}

qt_html_renderer::~qt_html_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~qt_html_renderer(0x%x)", this);
	if (m_url) {
		delete m_url;
		m_url = NULL;
	}
	if (m_browser) {
//		m_browser->closeURL();
		delete m_browser;
		m_browser = NULL;
	}
	m_lock.leave();
}


void
qt_html_renderer::redraw_body(const lib::screen_rect<int> &r,
				     common::gui_window* w) {
	m_lock.enter();
	const lib::point p = m_dest->get_global_topleft();
	AM_DBG lib::logger::get_logger()->debug(
		"qt_html_renderer.redraw(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_url = %s, p=(%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom(),
		(const char*) m_url->get_url().c_str(),
		p.x, p.y);
	if (m_browser == NULL) {
		ambulant_qt_window* aqw = (ambulant_qt_window*)w;
		qt_ambulant_widget* qaw = aqw-> get_ambulant_widget();
		m_browser = new KHTMLPart((QWidget*)qaw);
		m_browser->openURL(m_url->get_url().c_str());
		m_browser->view()->resize(r.width(),r.height());
		m_browser->show();
	}
	m_lock.leave();
}

#endif/*WITH_QT_HTML_WIDGET*/
