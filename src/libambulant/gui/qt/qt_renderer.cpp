
/* 
 * @$Id$ 
 */

#include <qt_gui.h>
#include <qt_renderer.h>
#include <qt_image_renderer.h>
#include <qt_text_renderer.h>

namespace ambulant {
  
using namespace lib;
  
namespace gui {
namespace qt_renderer {
  
  void
  qt_passive_window::need_redraw(const screen_rect<int> &r)
  {
    logger::get_logger()->trace
      ("qt_passive_window::need_redraw(0x%x), "
       "ltrb=(%d,%d,%d,%d)",
       (void *)this, r.left(), r.top(), r.right(), r.bottom());
    view()->repaint(r.left(), r.top(), 
		    r.width(), r.height(),
		    true);
  }
    active_renderer *
  qt_renderer_factory::new_renderer(event_processor *const evp,
				    net::passive_datasource *src,
				    passive_region *const dest,
				    const node *node)
  {
    xml_string tag = node->get_qname().second;
    active_renderer* rv;
    if (tag == "img") {
      rv = (active_renderer*) 
	new qt_active_image_renderer(evp, src, dest, node);
      logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_image_renderer 0x%x", 
	 (void*) node, (void*) rv);
    } else if ( tag == "text") {
      rv = (active_renderer*)
	new qt_active_text_renderer(evp, src, dest, node);
      logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_text_renderer 0x%x",
	 (void*) node, (void*) rv);
    } else {
      logger::get_logger()->error("qt_renderer_factory: "
				  "no Qt renderer for tag \"%s\"",
				  tag.c_str());
      rv = new gui::none::none_active_renderer(evp, src, dest, node);
      logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning none_active_renderer 0x%x",
	 (void*) node, (void*) rv);
    }
    return rv;
  }    
  passive_window *
  qt_window_factory::new_window(const std::string &name, size bounds)
  {
    logger::get_logger()->trace
      ("qt_window_factory::new_window (0x%x) name=%s", 
       (void*) this, name.c_str());
      qt_passive_window * qpw = new qt_passive_window(name, bounds, m_view);
      m_view->set_ambulant_window((void*)qpw);
      return qpw;
  }
  
  
} // namespace qt_renderer

} // namespace gui

} //namespace ambulant
