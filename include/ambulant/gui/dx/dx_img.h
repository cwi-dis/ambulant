
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DX_IMG_H
#define AMBULANT_GUI_DX_IMG_H

#include <string>
#include "ambulant/common/renderer.h"
#include "ambulant/gui/dx/jpg_decoder.h"

namespace ambulant {

namespace gui {

namespace dx {

class viewport;
class region;

class dx_img_renderer : public lib::active_renderer {
  public:
	dx_img_renderer(lib::event_processor *evp, net::passive_datasource *src, 
		lib::passive_region *dest, const lib::node *node);
	~dx_img_renderer();
	void start(lib::event *playdone);
	void readdone();
	void stop();
	void redraw(const lib::screen_rect<int> &dirty, lib::passive_window *window, 
		const lib::point &window_topleft); 
  private:
	viewport* get_viewport(lib::passive_window *window);
	viewport* get_viewport();
	region* m_region;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_IMG_H
