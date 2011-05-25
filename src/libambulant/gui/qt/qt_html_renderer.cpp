// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

/*
 * @$Id$
 */
#ifdef	WITH_QT_HTML_WIDGET

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_factory_impl.h"
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
	:	m_browser(br),
		m_generation(0)
	{}

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
		evp->add_event(cb, 1, lib::ep_med);
	}

};

extern const char qt_html_playable_tag[] = "text";
extern const char qt_html_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererQt");
extern const char qt_html_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererHtml");

common::playable_factory *
gui::qt::create_qt_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererQt"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererHtml"), true);
	return new common::single_playable_factory<
		qt_html_renderer,
		qt_html_playable_tag,
		qt_html_playable_renderer_uri,
		qt_html_playable_renderer_uri2,
		qt_html_playable_renderer_uri2>(factory, mdp);
}

qt_html_renderer::qt_html_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	renderer_playable(context, cookie, node, evp, factory, mdp),
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

bool
gui::qt::qt_html_renderer::stop() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_html_renderer::stop(0x%x)", this);
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	if (m_html_browser)
		m_html_browser->hide(m_event_processor);
	m_lock.leave();
	return true;
}
#endif/*WITH_QT_HTML_WIDGET*/
