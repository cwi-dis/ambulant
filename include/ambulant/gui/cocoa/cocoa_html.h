/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_GUI_COCOA_COCOA_HTML_H
#define AMBULANT_GUI_COCOA_COCOA_HTML_H

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/renderer_impl.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

// Opaque class used to store the WebKit view:
class wvc_container;

class cocoa_html_renderer : public common::renderer_playable {
  public:
	cocoa_html_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
	:	common::renderer_playable(context, cookie, node, evp, fp, mdp),
		m_html_view(NULL) {};
	~cocoa_html_renderer() {};

	void start(double where);
//	void stop();
	bool stop();

	void seek(double t) {}
	void redraw(const lib::rect &dirty, common::gui_window *window) {}
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	wvc_container *m_html_view; // We don't want to include WebKit.h here...
	lib::critical_section m_lock;
};

} // namespace cocoa

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_HTML_H
