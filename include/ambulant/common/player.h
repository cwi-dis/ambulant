
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_PLAYER_H
#define AMBULANT_LIB_PLAYER_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/timelines.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/region.h"
#include "ambulant/lib/renderer.h"

namespace ambulant {

namespace lib {

// Forward
class active_player;

namespace detail {

} // namespace detail

// I'm not clear on the split between active and passive player yet. The
// passive_player could do loading of the document and parsing it, and
// possibly even generating the timelines.
// Splitting the player in active/passive may be a case of over-generality,
// in which case we get rid of the passive_player later.
class passive_player {
  public:
	friend class active_player;
	
	passive_player() 
	:	m_url("") {}
	passive_player(const char *url)
	:	m_url(url) {}
	~passive_player() {}
	
	active_player *activate(window_factory *wf, renderer_factory *rf);
  private:
  	const char *m_url;
};

class active_player : public ref_counted_obj {
  public:
	active_player(passive_player *const source, node *tree, window_factory *wf, renderer_factory *rf);
	~active_player();
	
	void start(event_processor *evp, event *playdone);
	void stop();
	void set_speed(double speed);
	
	inline void timeline_done_callback() {
		m_done = true;
	}
	
  private:
  	passive_timeline *build_timeline();
	
	node *m_tree;
	timer *m_timer;
  	event_processor *const m_event_processor;
	passive_player *const m_source;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;
	window_factory *m_window_factory;
	renderer_factory *m_renderer_factory;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PLAYER_H
