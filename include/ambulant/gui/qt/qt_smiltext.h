/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_QT_SMILTEXT_H
#define AMBULANT_GUI_QT_SMILTEXT_H

#ifdef  WITH_SMIL30

#include "ambulant/config/config.h"
#include <string>

#include "ambulant/smil2/smiltext.h"
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_transition.h"

namespace ambulant {

namespace gui {

namespace qt {

class text_metrics {
  public:
	text_metrics(unsigned int ascent, unsigned int descent, unsigned int height, unsigned int width, unsigned int line_spacing)
	  :	m_ascent(ascent), 
		m_descent(descent), 
	  	m_height(height),
		m_width(width),
		m_line_spacing(line_spacing) {}

	~text_metrics() {}

	unsigned int get_ascent()	{ return m_ascent; }
	unsigned int get_descent()	{ return m_descent; };
	unsigned int get_height()	{ return m_height; };
	unsigned int get_width()	{ return m_width; };
	unsigned int get_line_spacing() { return m_line_spacing; };

  private:
	unsigned int m_ascent;
	unsigned int m_descent;
	unsigned int m_height;	
	unsigned int m_width;	
	unsigned int m_line_spacing;	
};

class smiltext_renderer;

class qt_smiltext_renderer : 
		public qt_renderer<renderer_playable>,
		public smil2::smiltext_notification
 {
  public:
	qt_smiltext_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp);
	~qt_smiltext_renderer();
	void start(double t);
	void stop();
	void seek(double t) {}
	// Callback from the engine
	void smiltext_changed();
	void user_event(const lib::point& pt, int what);
	void redraw_body(const lib::rect &dirty, common::gui_window *window);
  private:
	// functions required by inheritance
	void smiltext_changed(bool);
	// internal helper functions
	void _qt_smiltext_changed(const lib::rect r);
	bool _qt_smiltext_fits(const smil2::smiltext_run run, const lib::rect r);
	text_metrics _qt_smiltext_get_text_metrics(const smil2::smiltext_run run);
	lib::rect _qt_smiltext_compute(const smil2::smiltext_run run, const lib::rect r);
	void _qt_smiltext_render(const smil2::smiltext_run run, const lib::rect r, const lib::point p);
	void _qt_smiltext_set_font(const smil2::smiltext_run run);
	void _qt_smiltext_shift(const lib::rect r, const lib::point p);
	// instance variables
	net::datasource_factory *m_df;
	smil2::smiltext_engine m_engine;
	const smil2::smiltext_params& m_params;
//XX bool m_render_offscreen; // True if m_params does not allows rendering in-place
	lib::timer::time_type m_epoch;
	critical_section m_lock;
	int m_x; // (L,T) of current word in <smiltext/> during computations
	int m_y;
	unsigned int m_max_ascent;
	unsigned int m_max_descent;
	// Qt related variables
	ambulant_qt_window* m_window;
	QFont m_font;
 };

} // namespace qt

} // namespace gui
 
} // namespace ambulant
#endif //WITH_SMIL30

#endif // AMBULANT_GUI_QT_SMILTEXT_H
