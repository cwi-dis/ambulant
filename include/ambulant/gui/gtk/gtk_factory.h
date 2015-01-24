/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#ifndef GTK_FACTORY_H
#define GTK_FACTORY_H

#include "ambulant/common/playable.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/common/gui_player.h"

#include "gtk_fill.h"

#include <gtk/gtk.h>

namespace ambulant {

namespace gui {

namespace gtk {
#if GTK_MAJOR_VERSION >= 3
	void cairo_surface_bitblt(cairo_surface_t* dst, int dst_x, int dst_y, cairo_surface_t* src, int src_x, int src_y, int width, int height);
#else //  GTK_MAJOR_VERSION < 3
	void gdk_pixmap_bitblt(GdkPixmap* dst, int dst_x, int dst_y, GdkPixmap* src, int src_x, int src_y, int width, int height);
#endif //  GTK_MAJOR_VERSION < 3

class gtk_ambulant_widget;

/// GTK implementation of window_factory
class gtk_window_factory : public common::window_factory {
  public:
	gtk_window_factory(gtk_ambulant_widget* gtk_widget, common::gui_player* gpl);
	~gtk_window_factory();
	common::gui_window* new_window(const std::string &name, lib::size bounds, common::gui_events *region);
	common::bgrenderer *new_background_renderer(const common::region_info *src);


  private:
//	GtkWidget* m_parent_widget;
	gtk_ambulant_widget* m_parent_widget;
	lib::point m_p;
	gui_player* m_gui_player;
	GdkCursor* m_arrow_cursor;
	GdkCursor* m_hand1_cursor;
	GdkCursor* m_hand2_cursor;
};  // class gtk_window_factory


/// ambulant_gtk_window is the GTK implementation of gui_window, it is the
/// class that corresponds to a SMIL topLayout element.

class ambulant_gtk_window : public common::gui_window {
  public:
	ambulant_gtk_window(const std::string &name, lib::rect* bounds, common::gui_events *region);
	~ambulant_gtk_window();

	// gui_window API:
	void need_redraw(const lib::rect &r);
	void redraw(const lib::rect &r);
	void redraw_now();

	// gui_events API:
	bool user_event(const lib::point &where, int what=0);
	void need_events(bool want);

	// semi-private helpers:

	/// Set the corresponding widget.
	void set_ambulant_widget(gtk_ambulant_widget* gtkaw);

	/// Get the GTK widget corresponding to this ambulant window.
	gtk_ambulant_widget* get_ambulant_widget();

	/// Set our top-level gui_player.
	void set_gui_player(gui_player* gpl);

	/// Get our top-level gui_player.
	gui_player* get_gui_player();

	/// Initialize a GDK cached cursortype
	void set_gdk_cursor(GdkCursorType, GdkCursor*);
	/// Return any of GDK cached cursortypes
	GdkCursor* get_gdk_cursor(GdkCursorType);

	void delete_ambulant_surface();

	// Screen transitions
	void startScreenTransition();
	void endScreenTransition();
	void screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now);
	void _screenTransitionPreRedraw();
	void _screenTransitionPostRedraw(const lib::rect &r);
	// XXX These need to be documented...

#if GTK_MAJOR_VERSION >= 3
	cairo_surface_t* new_ambulant_surface();
	cairo_surface_t* get_ambulant_surface();
	cairo_surface_t* get_old_target_surface();
	/// return copy of (recctangle 'rp' in ) surface 'srf'
	cairo_surface_t* copy_surface(cairo_surface_t* srf, rect* rp = NULL);
	cairo_surface_t* get_surface_from_screen(const lib::rect &r);
	void set_target_surface(cairo_surface_t* surf);
	void set_drawing_surface(cairo_surface_t* surf) { m_target_surface = surf; }
	cairo_surface_t* get_target_surface() { return m_target_surface; }
	lib::rect get_bounds() { return m_bounds; }
	cairo_surface_t* create_similar_surface (cairo_surface_t* surface);
	void reset_target_surface(void);

	cairo_surface_t* m_tmp_surface; //X
	cairo_surface_t* m_transition_surface;//TBD
	cairo_surface_t* get_transition_surface();//TBD
	lib::rect m_target_bounds;
	void set_target_bounds (lib::rect target_bounds) { m_target_bounds = target_bounds; }
	lib::rect get_target_bounds() { return m_target_bounds; }
#else //  GTK_MAJOR_VERSION < 3
	GdkPixmap* get_ambulant_pixmap();
	GdkPixmap* new_ambulant_surface();

	GdkPixmap* get_ambulant_surface();
	GdkPixmap* get_ambulant_oldpixmap();
	GdkPixmap* get_pixmap_from_screen(const lib::rect &r);
	void set_ambulant_surface(GdkPixmap* surf);
	void reset_ambulant_surface(void);
	GdkPixmap* m_tmppixmap;
#endif //  GTK_MAJOR_VERSION < 3
	guint signal_redraw_id;

  private:
	void clear();
	lib::rect  m_bounds;
	gtk_ambulant_widget* m_ambulant_widget;
	gui_player* m_gui_player;
	GdkCursor* m_arrow_cursor;
	GdkCursor* m_hand1_cursor;
	GdkCursor* m_hand2_cursor;
	int m_fullscreen_count;
	smil2::transition_engine* m_fullscreen_engine;
	lib::transition_info::time_type m_fullscreen_now;

#if GTK_MAJOR_VERSION >= 3
	cairo_surface_t* m_target_surface; // surface for final bitblt

	cairo_surface_t* m_old_target_surface;
	cairo_surface_t* m_surface;
	cairo_surface_t* m_fullscreen_prev_surface;
	cairo_surface_t* m_fullscreen_old_surface;
#else //  GTK_MAJOR_VERSION < 3
	GdkPixmap* m_pixmap;
	GdkPixmap* m_oldpixmap;
	GdkPixmap* m_surface;
	GdkPixmap* m_fullscreen_prev_pixmap;
	GdkPixmap* m_fullscreen_old_pixmap;
#endif //  GTK_MAJOR_VERSION < 3
};  // class ambulant_gtk_window

/// gtk_ambulant_widget is the GTK-counterpart of ambulant_gtk_window: it is the
/// GTKWidget that corresponds to an Ambulant topLayout window.
class gtk_ambulant_widget : public GtkWidget, public ambulant::common::gui_screen
{
  public:
	gtk_ambulant_widget(GtkWidget* widget);
	~gtk_ambulant_widget();

	/// Helper: set our counterpart gui_window.
	void set_gtk_window( ambulant_gtk_window* agtkw);

	/// Helper: get our counterpart gui_window.
	ambulant_gtk_window* gtk_window();

	/// Helper: get the actual GTK Widget
	GtkWidget* get_gtk_widget();

	// GTKWidget API:
#if GTK_MAJOR_VERSION >= 3
	void do_draw_event (GtkWidget *widget, cairo_t *cr);
#else //  GTK_MAJOR_VERSION < 3
	void do_paint_event (GdkEventExpose * event);
#endif //  GTK_MAJOR_VERSION < 3
	void do_motion_notify_event(GdkEventMotion *event);
	void do_button_release_event(GdkEventButton *event);
	void do_key_release_event(GdkEventKey *event);
//	void mouseReleaseEvent(QMouseEvent* e);

	// gui_screen implementation
	void get_size(int *width, int *height);
	bool get_screenshot(const char *type, char **out_data, size_t *out_size);
	bool set_overlay(const char *type, const char *data, size_t size);
	bool clear_overlay();
	bool set_screenshot(char **screenshot_data, size_t *screenshot_size);

  public:
	// For the gui_screen implementation
	gchar * m_screenshot_data;
	gsize m_screenshot_size;

	// widget counter (with s_lock protection) is used to assuere that the GtkWidget
	// in drawing callback functions are still valid pointers at the time the callback
	// is executed by the main thread */
	static lib::critical_section s_lock;
	static int s_widgets;

	// gtk_ambulant_widget::m_draw_area_tags contains the set of tags returned by
	// g_idle_queue_add() that are not yet processed. This set is maintained because
	// in the npambulant plugin, when the plugin is unloaded all unprocessed queue entries
	// must be removed from the main event loop, otherwise the callback will be done on
	// removed code and the browser may crash.
	std::set<guint> m_draw_area_tags;
  private:
	ambulant_gtk_window* m_gtk_window;
	GtkWidget *m_widget;
	gulong m_expose_event_handler_id;
	gulong m_motion_notify_handler_id;
	gulong m_button_release_handler_id;
	gulong m_key_release_handler_id;
	lib::critical_section m_lock;

};  // class gtk_ambulant_widget

AMBULANTAPI common::window_factory *create_gtk_window_factory(gtk_ambulant_widget* gtk_widget, common::gui_player* gpl);
// XXXX Needs to be implemented:
// Create gtk_ambulant_widget inside gtk_parent_widget, call create_gtk_window_factory.
AMBULANTAPI common::window_factory *create_gtk_window_factory_unsafe(void* gtk_parent_widget, common::gui_player* gpl);

// Playable factories
AMBULANTAPI common::playable_factory *create_gtk_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_gtk_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_gtk_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_gtk_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_gtk_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);



} // namespace gtk

} // namespace gui

} // namespace ambulant

#endif // GTK_FACTORY_H
