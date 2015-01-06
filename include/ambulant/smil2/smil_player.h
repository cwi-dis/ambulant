/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#ifndef AMBULANT_SMIL2_SMIL_PLAYER_H
#define AMBULANT_SMIL2_SMIL_PLAYER_H

#include "ambulant/lib/timer.h"
#include "ambulant/lib/timer_sync.h"
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

class smil_player :
	public common::player,
	public time_node_context,
	public common::playable_notification,
	virtual public lib::ref_counted_obj
{
  public:
	typedef time_traits::value_type time_value_type;

	smil_player(lib::document *doc, common::factories *factory, common::embedder *sys = 0);
	void initialize();
	void terminate();
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
    bool uses_external_sync() const {
#ifdef WITH_EXTERNAL_SYNC
        return m_timer_sync && m_timer_sync->uses_external_sync();
#else
        return false;
#endif
    }

	common::play_state get_state() const {return m_state;}

	void before_mousemove(int cursorid);
	int after_mousemove();
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
	virtual void on_state_change(const char *ref);
	common::state_component *get_state_engine() { return m_state_engine;}
	virtual void on_focus_advance();
	virtual void on_focus_activate();

	///////////////////
	// playable_notification interface

	virtual void started(int n, double t);
	virtual void stopped(int n, double t);
	virtual void clicked(int n, double t);
	virtual void pointed(int n, double t);
	virtual void transitioned(int n, double t);
	virtual void marker_seen(cookie_type n, const char *name, double t);
    virtual void clicked_external(lib::node *n, lib::timer::time_type t);
// Defined below
//	virtual void playable_stalled(const playable *p, const char *reason);
//	virtual void playable_unstalled(const playable *p);
//	virtual void playable_started(const playable *p, const lib::node *n, const char *comment);
//	virtual void playable_resource(const playable *p, const char *resource, long amount);

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
    lib::document* get_document() { return m_doc; }
	void show_link(const lib::node *n, const net::url& href,
		src_playstate srcstate, dst_playstate dststate, const char *target);
	lib::event_processor* get_evp() { return m_event_processor;}
	virtual time_value_type elapsed() const { return m_timer->elapsed();}
	virtual void schedule_event(lib::event *ev, lib::timer::time_type t, lib::event_priority ep);
	virtual void cancel_event(lib::event *ev, lib::event_priority ep = ep_low)
		{ m_event_processor->cancel_event(ev, ep);}
	virtual void cancel_all_events() { m_event_processor->cancel_all_events();}
	virtual bool wait_for_eom() const { return m_wait_for_eom_flag;}
	virtual void set_wait_for_eom(bool b) { m_wait_for_eom_flag = b;}

    // Focus feedback stuff
	void set_focus_feedback(common::focus_feedback *h) { m_focus_handler = h; }
	void node_focussed(const lib::node *n) { if (m_focus_handler) m_focus_handler->node_focussed(n); }
	// Feedback stuff
	void set_feedback(common::player_feedback *h) { m_feedback_handler = h; }
    common::player_feedback *get_feedback() { return m_feedback_handler; }
	void document_loaded(lib::document *doc) { if (m_feedback_handler) m_feedback_handler->document_loaded(doc); }
	void document_started() { if (m_feedback_handler) m_feedback_handler->document_started(); }
	void document_stopped() { if (m_feedback_handler) m_feedback_handler->document_stopped(); }
	void node_started(const lib::node *n) { if (m_feedback_handler) m_feedback_handler->node_started(n); }
	void node_filled(const lib::node *n) { if (m_feedback_handler) m_feedback_handler->node_filled(n); }
	void node_stopped(const lib::node *n) { if (m_feedback_handler) m_feedback_handler->node_stopped(n); }
	void playable_started(const playable *p, const lib::node *n, const char *comment) {
		if (m_feedback_handler) m_feedback_handler->playable_started(p, n, comment);
	}
	void playable_stalled(const playable *p, const char *reason) { if (m_feedback_handler) m_feedback_handler->playable_stalled(p, reason); }
	void playable_unstalled(const playable *p) { if (m_feedback_handler) m_feedback_handler->playable_unstalled(p); }
	void playable_cached(const playable *p) { if (m_feedback_handler) m_feedback_handler->playable_cached(p); }
	void playable_deleted(const playable *p) { if (m_feedback_handler) m_feedback_handler->playable_deleted(p); }
	void playable_resource(const playable *p, const char *resource, long amount) {
		if (m_feedback_handler) m_feedback_handler->playable_resource(p, resource, amount);
	}
	
	virtual bool goto_node(const lib::node *n);

	bool highlight(const lib::node *n, bool on=true);

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
	void create_state_engine();
	common::playable* _new_playable(const lib::node *n);
	void _destroy_playable(common::playable *r, const lib::node *n);
	void destroy_playable_in_cache(std::pair<const lib::node*, common::playable*> victim);
    void empty_playable_cache();
	common::playable* _get_playable(const lib::node *n) {
		std::map<const lib::node*, common::playable *>::iterator it =
			m_playables.find(n);
		return (it != m_playables.end())?(*it).second:0;
	}
	// timegraph sampling
	void update();
	void _update();
	void _resume();

	common::state_component *m_state_engine;
	lib::document *m_doc;
	common::factories *m_factory;
	//common::window_factory *m_wf;
	//common::playable_factory *m_pf;
	common::embedder *m_system;
	common::focus_feedback *m_focus_handler;
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
	bool m_wait_for_eom_flag;
	std::map<const lib::node*, common::playable *> m_playables;
	std::map<const std::string, common::playable *> m_cached_playables;

	critical_section m_playables_cs;
	std::map<const node*, double> m_playables_dur;
	lib::logger *m_logger;
	std::map<int, int> m_accesskey_map;
	const lib::node *m_focus;
	std::set<int> *m_focussed_nodes;
	std::set<int> *m_new_focussed_nodes;
    lib::timer_sync *m_timer_sync;
	lib::critical_section m_lock;
	
	// Calling time_node members must be done while locking
	// the scheduler to avoid race conditions.
	// Therefore callbacks are needed to avoid deadlock. */
	typedef std::pair<time_node*, q_smil_time> async_arg;
	typedef lib::scalar_arg_callback<smil_player, async_arg> async_cb;
	typedef std::pair<time_node*, std::pair<q_smil_time, int> > async_int_arg;
	typedef lib::scalar_arg_callback<smil_player, async_int_arg> async_int_cb;
	typedef std::pair<time_node*, std::pair<q_smil_time, std::string> > async_string_arg;
	typedef lib::scalar_arg_callback<smil_player, async_string_arg> async_string_cb;
	void clicked_async(async_arg);
	void mouse_outofbounds_async(async_arg aa);
	void focus_outofbounds_async(async_arg aa);
	void mouse_inbounds_async(async_arg aa);
	void focus_inbounds_async(async_arg aa);
	void after_mousemove_async(async_arg);
	void started_async(async_arg);
	void stopped_async(async_arg);
	void transitioned_async(async_arg);
	void marker_seen_async(async_string_arg);
	void on_char_async(async_int_arg);
	void on_state_change_async(async_string_arg);
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_SMIL_PLAYER_H
