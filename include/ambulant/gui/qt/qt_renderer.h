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
 * $Id$
 */

/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */

#ifndef AMBULANT_GUI_QT_QT_RENDERER_H
#define AMBULANT_GUI_QT_QT_RENDERER_H

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"

#include "qt_fill.h"
#include "qt_includes.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace qt {

class qt_ambulant_widget;

class ambulant_qt_window : public common::abstract_window {
  public:
	ambulant_qt_window(const std::string &name,
			   lib::screen_rect<int>* bounds,
			   common::abstract_rendering_source *region)
:	common::abstract_window(region) ,
	m_ambulant_widget(NULL) {
		AM_DBG lib::logger::get_logger()->trace(
			"ambulant_qt_window::ambulant_qt_window(0x%x)",
			(void *)this);
		}
		void need_redraw(const lib::screen_rect<int> &r);
		void mouse_region_changed();
		void redraw(const lib::screen_rect<int> &r);
		void user_event(const lib::point &where);
		void set_ambulant_widget(qt_ambulant_widget* qaw) {
			AM_DBG lib::logger::get_logger()->trace(
				"ambulant_qt_window:"
				":set_ambulant_widget(0x%x)",
				(void *)qaw);
			m_ambulant_widget = qaw;
		}
		qt_ambulant_widget* ambulant_widget() {
			AM_DBG lib::logger::get_logger()->trace(
				"ambulant_qt_window:"
				" ambulant_widget(0x%x)",
				(void *)m_ambulant_widget);
			return m_ambulant_widget;
		}
  private:
		qt_ambulant_widget* m_ambulant_widget;
};  // class ambulant_qt_window

class qt_ambulant_widget : public QWidget {
  public:
	qt_ambulant_widget(const std::string &name,
			   lib::screen_rect<int>* bounds,
			   QWidget* parent_widget)
:	QWidget(parent_widget,"qt_ambulant_widget",0),
	m_qt_window(NULL) {
		AM_DBG lib::logger::get_logger()->trace(
			"qt_ambulant_widget::qt_ambulant_widget"
			"(0x%x-0x%x(%d,%d,%d,%d))",
			(void *)this,
			(void*)  parent_widget,
			bounds->left(),
			bounds->top(),
			bounds->right(),
			bounds->bottom());
		setGeometry(bounds->left(),
			    bounds->top(),
			    bounds->right(),
			    bounds->bottom());
		}
	void paintEvent(QPaintEvent* e) {
		AM_DBG lib::logger::get_logger()->trace(
			"qt_ambulant_widget::paintEvent"
			"(0x%x) e=0x%x)",
			(void*) this, (void*) e);
		QRect qr = e->rect();
		lib::screen_rect<int> r =  lib::screen_rect<int>(
			lib::point(qr.left(),qr.top()),
			lib::point(qr.right(),qr.bottom()));
		if (m_qt_window == NULL) {
			lib::logger::get_logger()->trace(
			"qt_ambulant_widget::paintEvent"
			"(0x%x) e=0x%x m_qt_window==NULL",
			(void*) this, (void*) e);
		return;
		}
		m_qt_window->redraw(r);
	}
	void set_qt_window( ambulant_qt_window* aqw) {
		m_qt_window = aqw;
		AM_DBG lib::logger::get_logger()->trace(
		"qt_ambulant_widget::set_qt_window"
		"((0x%x) m_qt_window==0x%x)",
		(void*) this, (void*) m_qt_window);
	}
	ambulant_qt_window* qt_window() {
		return m_qt_window;
	}
  private:
	ambulant_qt_window* m_qt_window;

};  // class qt_ambulant_widget

class qt_window_factory : public common::window_factory {
  public:
	qt_window_factory( QWidget* parent_widget, int x, int y)
:	m_parent_widget(parent_widget), m_p(lib::point(x,y)) {
		AM_DBG lib::logger::get_logger()->trace(
			"qt_window_factory (0x%x)", (void*) this);
		}
		common::abstract_window* new_window(
			const std::string &name,
			lib::size bounds,
			common::abstract_rendering_source *region);
		common::abstract_mouse_region *new_mouse_region();
		common::abstract_bg_rendering_source *new_background_renderer();
  private:
	QWidget* m_parent_widget;
	lib::point m_p;
};  // class qt_window_factory

class qt_renderer_factory : public common::renderer_factory {
  public:
	qt_renderer_factory() {
		AM_DBG lib::logger::get_logger()->trace(
			"qt_renderer factory (0x%x)", (void*) this);
		}
	common::active_renderer *new_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		net::passive_datasource *src,
		common::abstract_rendering_surface *const dest);

};  // class qt_renderer_factory

} // namespace qt

} // namespace gui

} // namespace ambulant
#endif  /*AMBULANT_GUI_QT_QT_RENDERER_H*/
