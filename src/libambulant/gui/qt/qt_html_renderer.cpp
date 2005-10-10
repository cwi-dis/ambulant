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

// Unique key used to access our renderer_private data
static common::renderer_private_id my_renderer_id = (common::renderer_private_id)"qt_html_renderer";

class gui::qt::browser_container : public ref_counted_obj {
    KHTMLPart *m_browser;
    int m_generation;
    
  public:
    browser_container(KHTMLPart *br)
    :   m_browser(br),
        m_generation(0) {}
  
    ~browser_container() {
        m_browser->hide();
        // XXX delete m_browsser
    }
    KHTMLPart *show() {
        m_generation++;
        return m_browser;
    }
	void hide_generation(int gen) {
		if (m_generation == gen) {
			m_browser->hide();
			m_generation++;
			AM_DBG lib::logger::get_logger()->debug("browser_container: %d: hiding HTML view", gen);
		} else {
			AM_DBG lib::logger::get_logger()->debug("browser_container: %d: not hiding HTML view", gen);
		}
	}
	void hide(event_processor *evp) {
		typedef lib::scalar_arg_callback_event<browser_container, int> hide_cb;
		hide_cb *cb = new hide_cb(this, &browser_container::hide_generation, m_generation);
		evp->add_event(cb, 1, lib::event_processor::med);
	}
    
};

qt_html_renderer::qt_html_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory)
:   renderer_playable(context, cookie, node, evp),
    m_html_browser(NULL) 
{
    
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer(0x%x)",this);
}

void 
gui::qt::qt_html_renderer::start(double t) {
	m_lock.enter();
 	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::start(0x%x)", this);

	assert(!m_html_browser);
	m_html_browser = dynamic_cast<browser_container*>(m_dest->get_renderer_private_data(my_renderer_id));
	if (m_html_browser == NULL) {
		lib::rect rc = m_dest->get_rect();
		const lib::point p = m_dest->get_global_topleft();
		rc.translate(p);

		assert(m_dest);
		common::gui_window* window = m_dest->get_gui_window();
		assert(window);
		ambulant_qt_window* aqw = (ambulant_qt_window*)window;
		qt_ambulant_widget* qaw = aqw->get_ambulant_widget();
		assert(qaw);

		KHTMLPart *br = new KHTMLPart((QWidget*)qaw);
		// XXX new html_browser(rc.left(), rc.top(), rc.width(), rc.height());
		assert(br);
		KHTMLView *vw = br->view();
		assert(vw);
		vw->move(rc.left(), rc.top());
		vw->resize(rc.width(), rc.height());
		m_html_browser = new browser_container(br);
		m_dest->set_renderer_private_data(my_renderer_id, static_cast<common::renderer_private_data*>(m_html_browser));
	}
	assert(m_html_browser);
	KHTMLPart *browser = m_html_browser->show();
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::start(0x%x) html_widget=0x%x",this,browser);

	net::url url = m_node->get_url("src");
	browser->openURL(url.get_url().c_str());

	browser->show();

	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	m_lock.leave();
}

qt_html_renderer::~qt_html_renderer() {
}

void
gui::qt::qt_html_renderer::stop() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::stop(0x%x)", this);
	m_dest->renderer_done(this);
	m_activated = false;
	if (m_html_browser)
	   m_html_browser->hide(m_event_processor);
	m_lock.leave();
}
#endif/*WITH_QT_HTML_WIDGET*/
