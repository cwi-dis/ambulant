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
#ifndef QT_FACTORY_H
#define QT_FACTORY_H

#include "ambulant/common/playable.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"

#include "qt_includes.h"
#include "qt_fill.h"

namespace ambulant {

namespace gui {

namespace qt {

class qt_ambulant_widget;

class ambulant_qt_window : public common::gui_window {
  public:
	ambulant_qt_window(const std::string &name,
			   lib::screen_rect<int>* bounds,
			   common::gui_events *region);
	~ambulant_qt_window();
			   
	void set_ambulant_widget(qt_ambulant_widget* qaw);
	QPixmap* ambulant_pixmap();
	qt_ambulant_widget* get_ambulant_widget();

	void need_redraw(const lib::screen_rect<int> &r);
	void redraw(const lib::screen_rect<int> &r);
	void mouse_region_changed();
	void user_event(const lib::point &where, int what=0);
	void need_events(bool want);
	QPixmap* new_ambulant_surface();
	QPixmap* get_ambulant_surface();
	QPixmap* get_ambulant_oldpixmap();
	void reset_ambulant_surface(void);
	void set_ambulant_surface(QPixmap* surf);
	void delete_ambulant_surface();

  private:
	qt_ambulant_widget* m_ambulant_widget;
	QPixmap* m_pixmap;
	QPixmap* m_oldpixmap;
	QPixmap* m_surface;
};  // class ambulant_qt_window

class qt_ambulant_widget : public QWidget {
  public:
	qt_ambulant_widget(const std::string &name,
			   lib::screen_rect<int>* bounds,
			   QWidget* parent_widget);
	~qt_ambulant_widget();
	
	void set_qt_window( ambulant_qt_window* aqw);
	ambulant_qt_window* qt_window();
	
	void paintEvent(QPaintEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

  private:
	ambulant_qt_window* m_qt_window;

  protected:
	void mouseMoveEvent(QMouseEvent* e);

};  // class qt_ambulant_widget

class qt_window_factory : public common::window_factory {
  public:
	qt_window_factory( QWidget* parent_widget, int x, int y);
		
		common::gui_window* new_window(
			const std::string &name,
			lib::size bounds,
			common::gui_events *region);
		common::bgrenderer *new_background_renderer(
			const common::region_info *src);
  private:
	QWidget* m_parent_widget;
	lib::point m_p;
};  // class qt_window_factory

class qt_renderer_factory : public common::playable_factory {
  public:
	qt_renderer_factory(net::datasource_factory *df);
	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp);
  protected:
  	net::datasource_factory *m_datasource_factory;

};  // class qt_renderer_factory

//class qt_video_factory : qt_renderer_factory {
class qt_video_factory : public common::playable_factory {
  public:
  
	qt_video_factory(net::datasource_factory *df)
	:   m_datasource_factory(df) {}
	~qt_video_factory();

	common::playable *
	  qt_video_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
 private:
        net::datasource_factory *m_datasource_factory;
			
}; // class qt_video_factory 

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // qt_FACTORY_H
