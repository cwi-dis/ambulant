/* 
 * $Id$
 */
/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */
 
#ifndef AMBULANT__QT_TEXT_RENDERER_H
#define AMBULANT__QT_TEXT_RENDERER_H

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
  class qt_active_text_renderer : active_final_renderer {

  public:
    qt_active_text_renderer(event_processor *const evp,
			    net::passive_datasource *src,
			    passive_region *const dest,
			    const node *node)
      : active_final_renderer(evp, src, dest, node),
      m_text_storage(NULL){}
    ~qt_active_text_renderer();

    void redraw(const screen_rect<int> &r,
		passive_window* w, 
		const point &p);

  private:
    char* m_text_storage;
    critical_section m_lock;
};

} // namespace qt_renderer

} // namespace gui
 
} // namespace ambulant

#endif/*AMBULANT__QT_TEXT_RENDERER_H*/
