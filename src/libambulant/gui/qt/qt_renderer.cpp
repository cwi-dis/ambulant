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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_fill.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/gui/none/none_mouse.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant; 
using namespace gui::qt;
  
void
ambulant_qt_window::need_redraw(const lib::screen_rect<int> &r) {
	AM_DBG lib::logger::get_logger()->trace(
		"ambulant_qt_window::need_redraw(0x%x), "
		"ltrb=(%d,%d,%d,%d)",
	       (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (ambulant_widget() == NULL) {
		lib::logger::get_logger()->error(
			"ambulant_qt_window::need_redraw(0x%x):"
			"ambulant_widget() == NULL !!!",
		 	(void*) this);
	      return;
	}
	ambulant_widget()->repaint(r.left(), r.top(), 
				   r.width(), r.height(),
				   true);
	ambulant_widget()->update();
}
  
void
ambulant_qt_window::mouse_region_changed() {
	lib::logger::get_logger()->error(
	"ambulant_qt_window::mouse_region_changed"
	" needs to be implemented");
}

void
ambulant_qt_window::redraw(const lib::screen_rect<int> &r) {
	AM_DBG lib::logger::get_logger()->trace(
		"ambulant_qt_window::redraw(0x%x), ltrb=(%d,%d,%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_region->redraw(r, this);
}

void
ambulant_qt_window::user_event(const lib::point &where) {
	m_region->user_event(where);
}

common::playable *
qt_renderer_factory::new_playable (
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {

	lib::xml_string tag = node->get_qname().second;
	common::playable* rv;
	if (tag == "img") {
 		rv = new qt_active_image_renderer(context, cookie, node, evp, 
 			m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace(
			"qt_renderer_factory: node 0x%x: "
			"returning qt_active_image_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if ( tag == "text") {
		rv = new qt_active_text_renderer(context, cookie, node, evp,
			m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace(
			"qt_renderer_factory: node 0x%x: "
	 		"returning qt_active_text_renderer 0x%x",
			(void*) node, (void*) rv);
	} else {
		return NULL;
	}
    return rv;
  }
  
common::abstract_window *
qt_window_factory::new_window (const std::string &name,
			       lib::size bounds,
			       common::renderer *region)
{
	lib::screen_rect<int> * r = new lib::screen_rect<int>(m_p, bounds);
	AM_DBG lib::logger::get_logger()->trace(
		"qt_window_factory::new_window (0x%x)"
		" name=%s %d,%d,%d,%d",
		(void*) this, name.c_str(),
		r->left(),r->top(),r->right(),r->bottom());
 	ambulant_qt_window * aqw = new ambulant_qt_window(
		name, r, region);
      	qt_ambulant_widget * qaw = new qt_ambulant_widget(
		name, r, m_parent_widget);
#ifndef	QT_NO_FILEDIALOG     /* Assume plain Qt */
	if (qApp == NULL || qApp->mainWidget() == NULL) {
		lib::logger::get_logger()->error(
			"qt_window_factory::new_window (0x%x) %s",
			(void*) this,
	   		"qApp == NULL || qApp->mainWidget() == NULL");
	}
	qApp->mainWidget()->resize(bounds.w + m_p.x, bounds.h + m_p.y);
#else	/*QT_NO_FILEDIALOG*/  /* Assume embedded Qt */
	/* No resize implemented for embedded Qt */
#endif	/*QT_NO_FILEDIALOG*/
	aqw->set_ambulant_widget(qaw);
	qaw->set_qt_window(aqw);
 	AM_DBG lib::logger::get_logger()->trace(
		"qt_window_factory::new_window(0x%x)"
		"ambulant_widget=0x%x qt_window=0x%x",
		(void*) this, (void*) qaw, (void*) aqw);
	qaw->show();
	return aqw;
}

common::gui_region *
qt_window_factory::new_mouse_region() {
	lib::logger::get_logger()->error(
		"qt_window_factory::new_mouse_region"
		" needs to be implemented");
	return new gui::none::none_mouse_region();
}

common::renderer *
qt_window_factory::new_background_renderer(const common::region_info *src) {
	lib::logger::get_logger()->trace(
		"qt_window_factory::new_background_renderer(0x%x): TBD",
		(void*) this);
	return new qt_background_renderer(src);
}
