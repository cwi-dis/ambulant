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

#ifndef AMBULANT_GUI_QT_SMILTEXT_H
#define AMBULANT_GUI_QT_SMILTEXT_H

#include <string>
#include "ambulant/config/config.h"
#include "ambulant/smil2/smiltext.h"
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_transition.h"

namespace ambulant {

namespace gui {

namespace qt {

class smiltext_renderer;

class qt_smiltext_renderer :
	public qt_renderer<renderer_playable>,
	public smil2::smiltext_notification,
	public smil2::smiltext_layout_provider
{
  public:
	qt_smiltext_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	~qt_smiltext_renderer();
	void start(double t);
	bool stop();
	void seek(double t) {}
	// Callback from the engine
	void smiltext_changed();
	void marker_seen(const char *name);
	void redraw_body(const lib::rect &dirty, common::gui_window *window);
	// smiltext_layout_provider called from smiltext_layout_engine
	smil2::smiltext_metrics get_smiltext_metrics(const smil2::smiltext_run& str);
	void render_smiltext(const smil2::smiltext_run& str, const lib::rect& r);
	void smiltext_stopped();
	const lib::rect& get_rect();

  private:
	// functions required by inheritance
	void smiltext_changed(bool);
	// internal helper functions
	void _qt_smiltext_set_font(const smil2::smiltext_run& run);
	// instance variables
	net::datasource_factory *m_df;
	smil2::smiltext_layout_engine m_layout_engine;
//XX bool m_render_offscreen; // True if m_params does not allows rendering in-place
	critical_section m_lock;
	// Qt related variables
	ambulant_qt_window* m_window;
	lib::rect m_rect;
	QFont  m_font;
	bool   m_blending;
	QColor m_qt_transparent;
	QColor m_qt_alternative;
	double m_bgopacity;
};

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_QT_SMILTEXT_H
