/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
#include "qt_gui.h"
#include "qt_image_renderer.h"

namespace ambulant {
  
using namespace lib;
  
namespace gui {
namespace qt_renderer {

  qt_active_image_renderer::~qt_active_image_renderer() {
    m_lock.enter();
    logger::get_logger()->trace
      ("qt_active_image_renderer::~qt_active_image_renderer()");
    m_lock.leave();
  }
  void
  qt_active_image_renderer::redraw(const screen_rect<int> &r,
				   passive_window* w,
				   const point& p)
  {
    m_lock.enter();
    logger::get_logger()->trace
      ("qt_active_image_renderer.redraw(0x%x)"
       ", ltrb=(%d,%d,%d,%d)"
       ", p=(%d,%d)", 
       (void *)this, r.left(), r.top(), r.right(), r.bottom()
       ,p.x,p.y);
    if (m_data && !m_image_loaded) {
      m_image_loaded
	= m_image.loadFromData((const uchar*)m_data, m_data_size);
    }
    if (m_image_loaded) {
      int L = r.left()+p.x, T = r.top()+p.y,
	W = r.width(), H = r.height();
      logger::get_logger()->trace
	("qt_active_image_renderer.redraw(0x%x), "
	 "drawImage at (L=%d,T=%d,W=%d,H=%d)",
	 (void *)this,L,T,W,H);
      qt_passive_window* qpw = (qt_passive_window*) w;
      QPainter paint;
      paint.begin(qpw->view()->workspace());
      paint.eraseRect(L,T,W,H);
      paint.drawImage(L,T,m_image,0,0,W,H);
      paint.flush();
      paint.end();
     } else {
      logger::get_logger()->error
	("qt_active_image_renderer.redraw(0x%x), "
	 "no m_image",
	 (void *)this
	 );
    }
    m_lock.leave();
  }
  
} // namespace qt_renderer

} // namespace gui

} //namespace ambulant
