
/* 
 * @$Id$ 
 */

#include "ambulant/gui/qt/qt_gui.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"

namespace ambulant {
  
using namespace lib;
  
namespace gui {
namespace qt_renderer {

 qt_active_text_renderer::~qt_active_text_renderer() {
    if (m_text_storage != NULL) {
      free(m_text_storage);
      m_text_storage =  NULL;
    }
  }
  void
  qt_active_text_renderer::redraw(const screen_rect<int> &r,
				  passive_window* w,
				  const point& p)
  {
    m_lock.enter();
    logger::get_logger()->trace
      ("qt_active_text_renderer.redraw(0x%x)"
       ",\nltrb=(%d,%d,%d,%d) m_data = %s"
       ", p=(%d,%d)",
       (void *)this, r.left(), r.top(), r.right(), r.bottom(),
       m_data == NULL ? "(null)": (const char*) m_data,
       p.x, p.y);
    if (m_data && !m_text_storage) {
      m_text_storage = (char*) malloc(m_data_size+1);
      strncpy(m_text_storage, (const char*) m_data, m_data_size);
      m_text_storage[m_data_size] = '\0';
    }
    if (m_text_storage) {
      int L = r.left()+p.x, T = r.top()+p.y,
	W = r.width(), H = r.height();
      qt_passive_window* qpw = (qt_passive_window*) w;
      QPainter paint;
      paint.begin(qpw->view()->workspace());
      paint.setPen(Qt::blue);
      paint.eraseRect(L,T,W,H);
      paint.drawText(L,T,W,H, Qt::AlignAuto, m_text_storage);
      paint.flush();
      paint.end();
    }
    m_lock.leave();
  }
  
} // namespace qt_renderer

} // namespace gui

} //namespace ambulant
