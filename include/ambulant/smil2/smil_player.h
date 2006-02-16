/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_SMIL2_SMIL_PLAYER_H
#define AMBULANT_SMIL2_SMIL_PLAYER_H

#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/node.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/smil2/time_nctx.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/embedder.h"

#include <map>

namespace ambulant {
namespace lib {
class document;
class timer;
} // namespace lib

namespace common {
class player;
class window_factory;
class playable_factory;
} // namespace common

namespace smil2 {

class smil_layout_manager;
class animation_engine;
class scheduler;

class smil_player : public common::player, /* public common::player_feedback,*/ public time_node_context, public common::playable_notification {
  public:
	typedef time_traits::value_type time_value_type;
	
	smil_player(lib::document *doc, common::factories *factory, common::embedder *sys = 0);
#ifdef USE_SMIL21
	void initialize();
#endif
	~smil_player();
		
	///////////////////
	// UI commands
	
	void start();
	void stop();
	void pause();
	void resume();
	
	bool is_playing() const { return m_state == common::ps_playing;}
	bool is_pausing() const { return m_state == common::ps_pausing;}
	bool is_done() const { return m_state == common::ps_done;}
	
	common::play_state get_state() const {return m_state;}
	
	void set_cursor(int cursorid) { m_cursorid = cursorid;}
	int get_cursor() const { return m_cursorid;}
	std::string get_pointed_node_str() const;
		
	//////////////////////
	// Time node context: Playable commands
	
	virtual common::playable *create_playable(const lib::node *n);
	virtual void start_playable(const lib::node *n, double t, const lib::transition_info *trans = 0);
	virtual void stop_playable(const lib::node *n);
	virtual void pause_playable(const lib::node *n, pause_display d = display_show);
	virtual void resume_playable(const lib::node *n);
	virtual void seek_playable(const lib::node *n, double t);
	virtual void wantclicks_playable(const lib::node *n, bool want);
	virtual void start_transition(const lib::node *n, const lib::transition_info *trans, bool in);
	//////////////////////
	// raw notifications from the UI
	
	virtual void on_char(int ch);
	
	///////////////////
	// playable_notification interface
	
	virtual void started(int n, double t);
	virtual void stopped(int n, double t);
	virtual void clicked(int n, double t);	
	virtual void pointed(int n, double t);	
	virtual void stalled(int n, double t);
	virtual void unstalled(int n, double t);
	virtual void transitioned(int n, double t);
	
	//////////////////////
	// Time node context: Playable queries
	
	virtual common::duration get_dur(const lib::node *n);
	
	//////////////////
	// Time node context: Notifications
	
	virtual void started_playback();
	virtual void done_playback();
	
	//////////////////////
	// Time node context: Services
	
	lib::timer* get_timer() { return m_timer;}
	void show_link(const lib::node *n, const net::url& href, 
		src_playstate srcstate=src_replace, dst_playstate dststate=dst_play);
	lib::event_processor* get_evp() { return m_event_processor;}	
	virtual time_value_type elapsed() const { return m_timer->elapsed();}
	virtual void schedule_event(lib::event *ev, lib::timer::time_type t, lib::event_priority ep);
	virtual void cancel_event(lib::event *ev, lib::event_priority ep = ep_low) 
		{ m_event_processor->cancel_event(ev, ep);}
	virtual void cancel_all_events() { m_event_processor->cancel_all_events();}
	virtual bool wait_for_eom() const { return m_eom_flag;}
	virtual void set_wait_for_eom(bool b) { m_eom_flag = b;}
	
	// Feedback stuff
	void set_feedback(common::player_feedback *h) { m_feedback_handler = h; }
	void document_started() { if (m_feedback_handler) m_feedback_handler->document_started(); }
	void document_stopped() { if (m_feedback_handler) m_feedback_handler->document_stopped(); }
	void node_started(const lib::node *n) { if (m_feedback_handler) m_feedback_handler->node_started(n); }
	void node_stopped(const lib::node *n) { if (m_feedback_handler) m_feedback_handler->node_stopped(n); }
	
	virtual bool goto_node(const lib::node *n);

	// Export the layout functionality for those who need it
	virtual smil_layout_manager *get_layout() { return m_layout_manager;}
	
 	// Builds or re-builds the layout
	// The layout may need to be rebuild when the
	// user changes custom test preferences. 
	void build_layout();
	
	// Builds or re-builds the timegraph
	// The timegraph may need to be rebuild when the
	// user changes custom test preferences. 
	void build_timegraph();
	
	animation_engine* get_animation_engine() { return m_animation_engine;}
	
 private:
	common::playable *new_playable(const lib::node *n); 
	void destroy_playable(common::playable *r, const lib::node *n); 
	common::playable* get_playable(const lib::node *n) {
		std::map<const lib::node*, common::playable *>::iterator it = 
			m_playables.find(n);
		return (it != m_playables.end())?(*it).second:0;
	}
	// timegraph sampling
	void update();
	
	lib::document *m_doc;
	common::factories *m_factory;
	//common::window_factory *m_wf;
	//common::playable_factory *m_pf;
	common::embedder *m_system;
	common::player_feedback *m_feedback_handler;
	animation_engine *m_animation_engine;
	time_node* m_root;
	std::map<int, time_node*> *m_dom2tn;
	smil_layout_manager *m_layout_manager;
	lib::timer_control *m_timer;
	lib::event_processor *m_event_processor;	
	scheduler *m_scheduler;
	common::play_state m_state;
	int m_cursorid;
	const time_node *m_pointed_node;
	bool m_eom_flag;
	std::map<const lib::node*, common::playable *> m_playables;
	critical_section m_playables_cs;
	std::map<const node*, double> m_playables_dur;
	lib::logger *m_logger;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_SMIL_PLAYER_H
