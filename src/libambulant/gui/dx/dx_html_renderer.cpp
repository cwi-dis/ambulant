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

#ifdef	WITH_HTML_WIDGET

#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/html_bridge.h"
#include "ambulant/gui/dx/dx_html_renderer.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/common/region.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

// #define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

extern const char dx_html_playable_tag[] = "text";
extern const char dx_html_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererIE");
extern const char dx_html_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererHtml");

common::playable_factory *
gui::dx::create_dx_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererIE"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererHtml"), true);
	return new common::single_playable_factory<
		gui::dx::dx_html_renderer,
		dx_html_playable_tag,
		dx_html_playable_renderer_uri2,
		dx_html_playable_renderer_uri2,
		dx_html_playable_renderer_uri3 >(factory, mdp);
}

// Unique key used to access our renderer_private data
static common::renderer_private_id my_renderer_id = (common::renderer_private_id)"dx_html_renderer";

class gui::dx::browser_container : public lib::ref_counted_obj {
	html_browser *m_browser;
	int m_generation;
  public:
	browser_container(html_browser *br)
		:	m_browser(br),
			m_generation(0) {}

	~browser_container() {
		/*AM*DBG*/ lib::logger::get_logger()->debug("~browser_container(m_browser=0x%x)", m_browser);
		m_browser->hide();
		// XXX Cannot delete?
	}
	html_browser *show() {
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
	void hide(lib::event_processor *evp) {
#if 0
		// XXXJack: bug workaround for hang in AmisAmbulant, see AmisHtmlView::html_browser_imp::hide.
		m_browser->hide();
#else
		typedef lib::scalar_arg_callback_event<browser_container, int> hide_cb;
		hide_cb *cb = new hide_cb(this, &browser_container::hide_generation, m_generation);
		evp->add_event(cb, 1, lib::ep_med);
#endif
	}
};

gui::dx::dx_html_renderer::dx_html_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, factory, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_html_browser(NULL),
	m_df(factory->get_datasource_factory())
{
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer(0x%x)", this);
}

gui::dx::dx_html_renderer::~dx_html_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_html_renderer(0x%x)", this);
}

void
gui::dx::dx_html_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::start(0x%x)", this);

//	assert(!m_html_browser);
	if (m_html_browser) {
		lib::logger::get_logger()->trace("dx_html_renderer: programmer error: attempt to start() again.\n");
		m_context->stopped(m_cookie);
		return;
	}
	m_html_browser = dynamic_cast<browser_container*>(m_dest->get_renderer_private_data(my_renderer_id));
	if (m_html_browser == NULL) {
//		dx_window *dxwindow = static_cast<dx_window*>(m_window);
//		viewport *v = dxwindow->get_viewport();
		lib::rect rc = m_dest->get_rect();
		const lib::point p = m_dest->get_global_topleft();
		rc.translate(p);
		html_browser_factory *brf = m_dxplayer->get_html_browser_factory();
		html_browser *br = brf->new_html_browser(rc.left(), rc.top(), rc.width(), rc.height());
		if (!br) {
			lib::logger::get_logger()->error("html_renderer: HtmlWidget not configured in main program");
			m_context->stopped(m_cookie);
			return;
		}
		m_html_browser = new browser_container(br);
		m_dest->set_renderer_private_data(my_renderer_id, static_cast<common::renderer_private_data*>(m_html_browser));
	}
	assert(m_html_browser);
	html_browser *it = m_html_browser->show();
	assert(it);
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::start(0x%x) html_widget=0x%x", this, it);

	net::url url = m_node->get_url("src");
	it->goto_url(url.get_url(), m_df);

	it->show();

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
	m_context->started(m_cookie);
	if (!it->uses_screen_reader()) m_context->stopped(m_cookie); // XXXJack
}

common::duration
gui::dx::dx_html_renderer::get_dur()
{
	if (m_html_browser) {
		html_browser *it = m_html_browser->show();
		if (it && it->uses_screen_reader())
			return common::duration(false, 0);
	}
	return common::duration(true, 0);
}


bool
gui::dx::dx_html_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::stop(0x%x)", this);
	// m_html_browser->hide();
	assert(m_html_browser);

	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	if (m_html_browser)
		m_html_browser->hide(m_event_processor);
	m_context->stopped(m_cookie);
	return true;
}

bool
gui::dx::dx_html_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void
gui::dx::dx_html_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::redraw");
}

#endif // WITH_HTML_WIDGET
