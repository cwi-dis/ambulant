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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {
  
using namespace lib;
  
namespace gui {
namespace qt_renderer {
  
  void
  ambulant_qt_window::need_redraw(const screen_rect<int> &r)
  {
    AM_DBG logger::get_logger()->trace
      ("ambulant_qt_window::need_redraw(0x%x), "
       "ltrb=(%d,%d,%d,%d)",
       (void *)this, r.left(), r.top(), r.right(), r.bottom());
    if (ambulant_widget() == NULL) {
      logger::get_logger()->error
	("ambulant_qt_window::need_redraw(0x%x):ambulant_widget() == NULL !!!",
	 (void*) this);
      return;
    }
    ambulant_widget()->repaint(r.left(), r.top(), 
			       r.width(), r.height(),
			       true);
    ambulant_widget()->update();
  }
  
  void
  ambulant_qt_window::mouse_region_changed()
  {
  	logger::get_logger()->error
	  ("ambulant_qt_window::mouse_region_changed needs to be implemented");
  }
  void
  ambulant_qt_window::redraw(const screen_rect<int> &r)
  {
    AM_DBG logger::get_logger()->trace
      ("ambulant_qt_window::redraw(0x%x), ltrb=(%d,%d,%d,%d)",
       (void *)this, r.left(), r.top(), r.right(), r.bottom());
    m_region->redraw(r, this);
  }
  void
  ambulant_qt_window::user_event(const point &where)
  {
    m_region->user_event(where);
  }
  active_renderer *
  qt_renderer_factory::new_renderer
  (
   lib::active_playable_events *context,
   lib::active_playable_events::cookie_type cookie,
   const lib::node *node,
   event_processor *const evp,
   net::passive_datasource *src,
   abstract_rendering_surface *const dest
   )
  {
    xml_string tag = node->get_qname().second;
    active_renderer* rv;
    if (tag == "img") {
      rv = (active_renderer*) 
	new qt_active_image_renderer(context, cookie,
				     node, evp, src, dest);
      AM_DBG logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_image_renderer 0x%x", 
	 (void*) node, (void*) rv);
    } else if ( tag == "text") {
      rv = (active_renderer*)
	new qt_active_text_renderer(context, cookie,
				    node, evp, src, dest);
      AM_DBG logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_text_renderer 0x%x",
	 (void*) node, (void*) rv);
    } else {
      return NULL;
    }
    return rv;
  }
  
  abstract_window *
  qt_window_factory::new_window (const std::string &name,
				 size bounds,
				 abstract_rendering_source *region)
  {
    screen_rect<int> * r = new screen_rect<int>(m_p, bounds);
    AM_DBG logger::get_logger()->trace
      ("qt_window_factory::new_window (0x%x) name=%s %d,%d,%d,%d", 
       (void*) this, name.c_str(),r->left(),r->top(),r->right(),r->bottom());
      ambulant_qt_window * aqw
	= new ambulant_qt_window(name, r, region);
      qt_ambulant_widget * qaw
	= new qt_ambulant_widget(name, r, m_parent_widget);
      aqw->set_ambulant_widget(qaw);
      qaw->set_qt_window(aqw);
      AM_DBG logger::get_logger()->trace
	("qt_window_factory::new_window(0x%x) ambulant_widget=0x%x qt_window=0x%x",
	 (void*) this, (void*) qaw, (void*) aqw);
      return aqw;
  }
  abstract_mouse_region *
  qt_window_factory::new_mouse_region()
  {
    logger::get_logger()->error
      ("qt_window_factory::new_mouse_region needs to be implemented");
  }
  abstract_bg_rendering_source *
  qt_window_factory::new_background_renderer()
  {
    logger::get_logger()->trace
	    ("qt_window_factory::new_background_renderer(0x%x): TBD",
	     (void*) this);
       return new qt_background_renderer();
//JNK	   return new none::none_background_renderer();
  }

} // namespace qt_renderer

} // namespace gui

} //namespace ambulant
