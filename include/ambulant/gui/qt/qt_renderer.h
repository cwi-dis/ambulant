/* 
 * $Id$
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */
 
#ifndef AMBULANT__QT_RENDERER_H
#define AMBULANT__QT_RENDERER_H

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"

class qt_gui;

using namespace std;

namespace ambulant {

using namespace lib;

namespace gui {

namespace qt_renderer {

  class qt_passive_window : public passive_window {
    
  public:
    qt_passive_window::qt_passive_window(const std::string &name, 
					 size bounds, qt_gui* view) 
      : passive_window(name, bounds),
      m_view(view) {
      logger::get_logger()->trace
	("qt_passive_window::qt_passive_window(0x%x)", (void *)this);
    }
    void need_redraw(const screen_rect<int> &r);
    qt_gui* view() { return m_view; }
    
  private:
    qt_gui* m_view;
  };

  class qt_window_factory : window_factory {
  
  public:
    qt_window_factory(qt_gui* view)
      : m_view(view) {
      logger::get_logger()->trace
	("qt_window_factory (0x%x)", (void*) this);
    }
    passive_window* new_window(const std::string &name, size bounds);
  private:
    qt_gui* m_view;
  };
  
  class qt_renderer_factory : renderer_factory {

  public:
    qt_renderer_factory() {
      logger::get_logger()->trace
	("qt_renderer factory (0x%x)", (void*) this);
    }
    active_renderer *new_renderer( event_processor *const evp,
				  net::passive_datasource *src,
				  passive_region *const dest,
				  const node *node);
  };

} // namespace qt_renderer

} // namespace gui
 
} // namespace ambulant

#endif/*AMBULANT__QT_RENDERER_H*/
