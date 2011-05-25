/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifndef QT_FACTORY_IMPL_H
#define QT_FACTORY_IMPL_H


#include "qt_includes.h"
#include "qt_fill.h"
#include "ambulant/gui/qt/qt_factory.h"

namespace ambulant {

namespace gui {

namespace qt {

#ifdef WITH_DUMPPIXMAP
/// Debug function that dumps a pixmap to a file. An incrementing
/// count is appended to the filenname, and an extension added.
void dumpPixmap(QPixmap* qpm, std::string filename);
#endif // WITH_DUMPPIXMAP

class qt_ambulant_widget;

/// Qt implementation of window_factory

class qt_window_factory : public common::window_factory {
  public:
	qt_window_factory( QWidget* parent_widget, int top_offset, gui_player *gpl);

	common::gui_window* new_window(const std::string &name, lib::size bounds, common::gui_events *region);
	common::bgrenderer *new_background_renderer(const common::region_info *src);

  private:
	QWidget* m_parent_widget;
	int m_top_offset;
	gui_player* m_gui_player;
};

/// Qt implementation of playable_factory

class qt_renderer_factory : public common::playable_factory {
  public:
	qt_renderer_factory(common::factories *factory);

	bool supports(common::renderer_select *rs) {
		return true;
	}
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
  protected:
	common::factories *m_factory;
};

/// Qt implementation of another playable_factory that handles video.

class qt_video_factory : public common::playable_factory {
  public:
	qt_video_factory(common::factories *factory)
	:   m_factory(factory)
	{}
	~qt_video_factory();

	bool supports(common::renderer_select *rs);

	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
  private:
	common::factories *m_factory;
};

/// ambulant_qt_window is the Qt implementation of gui_window, it is the
/// class that corresponds to a SMIL topLayout element.

class ambulant_qt_window : public common::gui_window {
  public:
	ambulant_qt_window(const std::string &name, lib::rect* bounds, common::gui_events *region);
	~ambulant_qt_window();

	// gui_window API:

	void need_redraw(const lib::rect &r);
	void redraw_now();
	void need_events(bool want);

	// gui_events API:

	void redraw(const lib::rect &r);
	bool user_event(const lib::point &where, int what=0);

	// semi-private helpers:

	/// Set the corresponding widget.
	void set_ambulant_widget(qt_ambulant_widget* qaw);

	/// Get the Qt widget corresponding to this ambulant window.
	qt_ambulant_widget* get_ambulant_widget();

	// XXX These need to be documented...
	QPixmap* get_ambulant_pixmap();
	QPixmap* new_ambulant_surface();
	QPixmap* get_ambulant_surface();
	QPixmap* get_ambulant_oldpixmap();
	QPixmap* get_pixmap_from_screen(const lib::rect &r);
	void reset_ambulant_surface(void);
	void set_ambulant_surface(QPixmap* surf);
	void delete_ambulant_surface();

	void startScreenTransition();
	void endScreenTransition();
	void screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now);

	void _screenTransitionPreRedraw();
	void _screenTransitionPostRedraw(const lib::rect &r);

  private:
	void clear();

	qt_ambulant_widget* m_ambulant_widget;

	QPixmap* m_pixmap;
	QPixmap* m_oldpixmap;
	QPixmap* m_surface;

	int m_fullscreen_count;
	QPixmap* m_fullscreen_prev_pixmap;
	QPixmap* m_fullscreen_old_pixmap;
	smil2::transition_engine* m_fullscreen_engine;
	lib::transition_info::time_type m_fullscreen_now;

  public:
	QPixmap* m_tmppixmap;
};

/// qt_ambulant_widget is the Qt-counterpart of ambulant_qt_window: it is the
/// QWidget that corresponds to an Ambulant topLayout window.

class qt_ambulant_widget : public QWidget {
  public:
	qt_ambulant_widget(const std::string &name, lib::rect* bounds, QWidget* parent_widget);
	~qt_ambulant_widget();

	/// Helper: set our counterpart gui_window.
	void set_qt_window( ambulant_qt_window* aqw);

	/// Helper: get our counterpart gui_window.
	ambulant_qt_window* qt_window();

	/// Helper: set our top-level gui_player.
	void set_gui_player(gui_player* gpl);

	/// Helper: get our top-level gui_player.
	gui_player* get_gui_player();

	// QWidget API:
	void paintEvent(QPaintEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);

  private:
	ambulant_qt_window* m_qt_window;
	gui_player* m_gui_player;
};

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // QT_FACTORY_IMPL_H
