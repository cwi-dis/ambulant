
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_COCOA_COCOA_AUDIO_H
#define AMBULANT_GUI_COCOA_COCOA_AUDIO_H

#include "ambulant/lib/renderer.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {
using namespace lib;

namespace gui {

namespace cocoa {

class cocoa_active_audio_renderer : public active_basic_renderer, public abstract_timer_client {
  public:
	cocoa_active_audio_renderer(event_processor *const evp,
		net::passive_datasource *src,
		const node *node);
	~cocoa_active_audio_renderer();

	void start(event *playdone);
//	void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft);
	void stop();
	void speed_changed();
  private:
	std::string m_url;
  	NSSound *m_sound;
	abstract_timer_client::client_index m_timer_index;
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_AUDIO_H
