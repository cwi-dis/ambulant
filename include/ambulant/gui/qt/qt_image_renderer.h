/* 
 * $Id$
 */

/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */
 
#ifndef AMBULANT__QT_IMAGE_RENDERER_H
#define AMBULANT__QT_IMAGE_RENDERER_H

#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/none/none_gui.h"

class qt_gui;

using namespace std;

namespace ambulant {

using namespace lib;

namespace gui {

namespace qt_renderer {
  class qt_active_image_renderer : active_final_renderer {

  public:
    qt_active_image_renderer(event_processor *const evp,
			     net::passive_datasource *src,
			     passive_region *const dest,
			     const node *node)
      : active_final_renderer(evp, src, dest, node),
          m_image(NULL),
          m_image_loaded(false) {};
   ~qt_active_image_renderer();

    void redraw(const screen_rect<int> &r,
		passive_window* w,
		const point &p);
  private:
    QImage m_image;
    bool m_image_loaded;
    critical_section m_lock;
};

} // namespace qt_renderer

} // namespace gui
 
} // namespace ambulant

#endif/*AMBULANT__QT_IMAGE_RENDERER_H*/
