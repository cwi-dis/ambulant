
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_DX_TEXT_H
#define AMBULANT_GUI_DX_TEXT_H

#include <string>
#include "ambulant/lib/renderer.h"

namespace ambulant {

namespace gui {

namespace dx {

class viewport;
class region;

class dx_text_renderer : public lib::active_renderer {
  public:
	dx_text_renderer(lib::event_processor *evp, net::passive_datasource *src, 
		lib::passive_region *dest, const lib::node *node);
	~dx_text_renderer();
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

#endif // AMBULANT_GUI_DX_TEXT_H
