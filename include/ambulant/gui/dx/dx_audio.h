
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DX_AUDIO_H
#define AMBULANT_GUI_DX_AUDIO_H

#include "ambulant/common/renderer.h"
#include "ambulant/gui/dx/dx_audio_player.h"

namespace ambulant {

namespace gui {

namespace dx {

class dx_audio_renderer : public lib::active_renderer {
  public:
	dx_audio_renderer(lib::event_processor *evp, net::passive_datasource *src, 
		lib::passive_region *dest, const lib::node *node);
	~dx_audio_renderer();
	void start(lib::event *playdone);
	void stop();
	void redraw(const lib::screen_rect<int> &dirty, lib::passive_window *window, 
		const lib::point &window_topleft); 
  private:
	audio_player<net::active_datasource> *m_player;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_AUDIO_H
