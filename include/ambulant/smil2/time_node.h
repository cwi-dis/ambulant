/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_SMIL2_TIME_NODE_H
#define AMBULANT_SMIL2_TIME_NODE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/node_navigator.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/sync_rule.h"
#include "ambulant/smil2/time_attrs.h"
#include "ambulant/smil2/time_state.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/timer.h"
#include "ambulant/common/playable.h"

#include "ambulant/lib/logger.h"

#ifndef AMBULANT_NO_IOSTREAMS

#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

#endif //AMBULANT_NO_IOSTREAMS

#include <cassert>
#include <utility>
#include <list>

namespace ambulant {

namespace smil2 {

class animation_engine;

// Time nodes context requirements
class time_node_context : public lib::event_scheduler<time_traits::value_type> {
  public:
	// Services
	virtual time_traits::value_type elapsed() const = 0;
	virtual timer* get_timer() = 0;
	virtual void show_link(const lib::node *n, const std::string& href) = 0;
	virtual animation_engine* get_animation_engine() = 0;
	
	// Playable commands
	virtual common::playable *create_playable(const lib::node *n) = 0;
	virtual void start_playable(const lib::node *n, double t) = 0;
	virtual void stop_playable(const lib::node *n) = 0;
	virtual void pause_playable(const lib::node *n, pause_display d = display_show) = 0;
	virtual void resume_playable(const lib::node *n) = 0;
	virtual void wantclicks_playable(const lib::node *n, bool want) = 0;
	
	// Playable queries
	virtual std::pair<bool, double> get_dur(const lib::node *n) = 0;
		
	// Notifications
	virtual void started_playback() = 0;
	virtual void done_playback() = 0;
};

// fwd declaration.
class transition_event;
class repeat_event;
class timer_event;

// Represents a node in the timing model.

class time_node : public time_traits {
  public:
	typedef time_node_context context_type;
	typedef node_navigator<time_node> nnhelper;
	typedef node_navigator<const time_node> const_nnhelper;
	
	time_node(context_type *ctx, const node *n, time_container_type type = tc_none, bool discrete = false); 
	
	virtual ~time_node();
  	
	// TimeElement interface
	// Currently support only startElement for the root.
	virtual void start();
	virtual void stop();
	virtual void pause();
	
	// Sets the timer for this node
	// This node becomes the owner of the timer e.g. should delete it on exit
	// The timegraph builder has already established the clocks network
	virtual void set_timer(lib::timer *tmr) { m_timer = tmr;}
	virtual lib::timer *get_timer() { return m_timer;}
	
	void set_want_activate_event(bool want) { m_want_activate_events = want;}
	bool wants_activate_event() const { return m_want_activate_events;}
	void want_accesskey(bool want) { m_want_accesskey = want;}
	bool want_accesskey() const { return m_want_accesskey;}

	// Functions that may be overriden by subclasses
	virtual time_node *append_child(time_node *child) {return nnhelper::append_child(this, child);}
	virtual void get_children(std::list<time_node*>& l) { nnhelper::get_children(this, l);}
	virtual void get_children(std::list<const time_node*>& l) const { const_nnhelper::get_children(this, l);}
	virtual time_type calc_implicit_dur();
	virtual bool needs_end_sync_update(const time_node *c, qtime_type timestamp) const { return false;}
	
	// Forced transitions
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void reset_children(qtime_type timestamp, time_node *oproot);
	virtual void startup_children(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);	
	virtual void kill_children(qtime_type timestamp, time_node *oproot);	
	
	// Sync update 
	virtual void sync_update(qtime_type timestamp);
	
	// Begin of media update
	void on_bom(qtime_type timestamp);
	
	// End of media update
	void on_eom(qtime_type timestamp);
	
	// Pause of media update
	void on_pom(qtime_type timestamp);
	
	// Resume of media update
	void on_rom(qtime_type timestamp);
	
	// End of simple duration update
	void on_eosd(qtime_type timestamp);
	
	virtual void raise_begin_event(qtime_type timestamp);
	virtual void raise_repeat_event(qtime_type timestamp);
	virtual void raise_end_event(qtime_type timestamp, time_node *oproot);
	virtual void raise_activate_event(qtime_type timestamp);
	virtual void raise_accesskey(std::pair<qtime_type, int> accesskey);
	void raise_begin_event_async(qtime_type timestamp);
	void raise_repeat_event_async(qtime_type timestamp);
	void raise_end_event_async(qtime_type timestamp, time_node *oproot);
	void activate_async(qtime_type timestamp);
	
	void schedule_interval(qtime_type timestamp, const interval_type& i);
	void cancel_interval(qtime_type timestamp);
	void update_interval_end(qtime_type timestamp, time_type newend);
	void update_interval(qtime_type timestamp, const interval_type& i);
	void defer_interval(qtime_type timestamp);
	
	void activate(qtime_type timestamp);
	void repeat(qtime_type timestamp);
	void remove(qtime_type timestamp);
	void fill(qtime_type timestamp);
	void pause(qtime_type timestamp, pause_display d);
	void resume(qtime_type timestamp);
	void check_repeat(qtime_type timestamp);
	
	// Std xml tree interface
	const time_node *down() const { return m_child;}
	const time_node *up() const { return m_parent;}
	const time_node *next() const { return m_next;}

	time_node *down()  { return m_child;}
	time_node *up()  { return m_parent;}
	time_node *next()  { return m_next;}

	void down(time_node *n)  { m_child = n;}
	void up(time_node *n)  { m_parent = n;}
	void next(time_node *n)  { m_next = n;}
	
	const time_node* previous() const {return const_nnhelper::previous(this);}
	time_node* previous() { return nnhelper::previous(this);}
	
	time_node* last_child() { return nnhelper::last_child(this);}
	const time_node* last_child() const { return const_nnhelper::last_child(this);}
	
	time_node* get_root() {return nnhelper::get_root(this);}
	const time_node* get_root() const {return const_nnhelper::get_root(this);}
	
	bool is_descendent_of(time_node *tn) const {return const_nnhelper::is_descendent(this, tn);}
	
	// tree iterators
	typedef tree_iterator<time_node> iterator;
	typedef const_tree_iterator<time_node> const_iterator;
	
    iterator begin() { return iterator(this);}
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}
	
	// Returns the underlying DOM node associated with this
	const node* dom_node() const { return m_node;}
	
	// Timing reference node for this.
	// This node uses the clock of the sync node for time notifications.
	// Here it is defined to be the parent unless it is the time root.
	// Could be the time root always (document time).
	const time_node* sync_node() const { return up()?up():this;}
	time_node* sync_node() { return up()?up():this;}
	value_type get_sync_simple_time() const;
	value_type get_simple_time() const;
	value_type get_rad() const { return m_rad;}
	
	// Time type queries
	time_container_type get_type() const { return m_type;}
	const char* get_type_as_str() const { return time_container_type_as_str(m_type);}
	bool is_time_container() const { return m_type != tc_none;}
	bool is_seq() const { return m_type == tc_seq;}
	bool is_par() const { return m_type == tc_par;}
	bool is_excl() const { return m_type == tc_excl;}
	bool is_discrete() const { return m_discrete;}
	bool is_root() const { return !up();}
	bool is_cmedia() const {return !is_time_container() && !is_discrete();}
	bool is_area() const {return m_attrs.get_tag() == "area";}
	bool is_animation() const;
	const time_attrs* get_time_attrs() const { return &m_attrs;}
	bool needs_implicit_dur() const;
	
	// Time graph building functions
	
	// Adds a begin rule to this node.
	// The rule maybe implicit dictated by the model 
	// or due to an explicit begin spec in the DOM.
	// For each rule, the timegraph builder will call 
	// this function in pair with add_dependent() against
	// the node that this rule depends on.
	void add_begin_rule(sync_rule *sr);
	
	// Adds an end rule to this node.
	// The rule maybe implicit dictated by the model 
	// or due to an explicit end spec in the DOM.
	// For each rule, the timegraph builder will call 
	// this function in pair with add_dependent() against
	// the node that this rule depends on.
	void add_end_rule(sync_rule *sr);
	
	// Adds a rule that dependents on this time node.
	// The rule will get a timestamped notification from this
	// node when the real or model event specified by the ev
	// argument occurs for this node.
	void add_dependent(sync_rule *sr, sync_event ev); 
			
	// Update dependents
	void on_new_instance(qtime_type timestamp, sync_event ev, time_type instance, time_node *filter=0);
	void on_cancel_instance(qtime_type timestamp, sync_event ev, time_type instance, time_node *filter=0);
	void on_update_instance(qtime_type timestamp, sync_event ev, time_type instance, time_type old_instance, time_node *filter=0);
	void on_add_instance(qtime_type timestamp, sync_event ev, time_type instance, int data = 0, time_node *filter=0);
		
	// Returns the lifetime state handler object of this time node
	time_state* get_state() { return m_state;}
	const time_state* get_state() const { return m_state;}
	
	// Retuns the context of this time node
	context_type* get_context() {return m_context;}
	
	// Sets the state of this node
	void set_state(time_state_type state, qtime_type timestamp, time_node *oproot);
	// Calls set_state() after checking for excl
	void set_state_ex(time_state_type tst, qtime_type timestamp);

	bool is_alive() const {
		time_state_type tst = m_state->ident(); 
		return tst == ts_proactive || tst == ts_active || tst == ts_postactive;
	}
	
	bool is_activateable() const {
		time_state_type tst = m_state->ident(); 
		return tst == ts_proactive || tst == ts_postactive;
	}
	
	// this may be called by a second thread
	bool is_active() const { return m_active;}

	bool has_started() const {
		time_state_type tst = m_state->ident(); 
		return tst == ts_active || tst == ts_postactive;
	}
				
	// Returns the current interval associated with this
	const interval_type& get_interval() const {
		return m_interval;
	}
	
	// Returns the last calc dur 
	// Used to avoid recursion when a child makes calcs 
	// that depend on parent simple dur
	// but parent's simple dur depends on the child
	time_type get_last_cdur() const { return m_last_cdur;}

	// Returns the priority class of this node
	// Applicable for excl children.
	int priority() const { return m_priority;}
	void set_priority(int prio) { m_priority = prio;}
	
	// Excl set/get flags
	bool paused() { return m_paused;}
	void set_paused(bool b) { m_paused = b;}
	bool deferred() { return m_deferred;}
	void set_deferred(bool b) { m_deferred = b;}
	
	////////////////////////
	// Time calculations
	
	// Calculates the simple duration of this node
	time_type calc_dur();
	
	// Calculates the active duration (AD) of this node
	// Uses simple duration calculation: calc_dur()
	time_type calc_ad(time_type b, time_type e);
	time_type calc_ad(time_type b);
	
	// AD helpers
	time_type calc_preliminary_ad(time_type b, time_type e);
	time_type calc_intermediate_ad();
	time_type calc_active_rad();
	time_type calc_preliminary_ad(time_type b);
	
	time_type calc_end(time_type b, time_type e);
	time_type calc_end(time_type b);

	// Calculate interval
	interval_type calc_first_interval();
	interval_type calc_next_interval();
	
	// Re-calculate current interval end
	time_type calc_current_interval_end();
	
	// Returns the current interval as clipped 
	// by its parent simple duration.
	interval_type calc_clipped_interval();
	
	// Dump this branch to the provided std::ostream.
#ifndef AMBULANT_NO_IOSTREAMS
	void dump(std::ostream& os);
	void dump_dependents(std::ostream& os, sync_event se);
#endif

	std::string to_string() const;
		
	// Verifier
 	static int get_node_counter() {return node_counter;}
 	
 	friend class time_state;
 	
 	// public S/O transitions
 	void cancel_schedule();
	void schedule_deferred_interval(qtime_type timestamp);
	void set_begin_event_inst(time_type inst) {m_begin_event_inst = inst;}
	
 protected:
	context_type *m_context;
	
	// The underlying DOM node
	// Mimimize or eliminate usage after timegraph construction 
	const node *m_node;
	
	
	// Attributes parser
	time_attrs m_attrs;
	
	// The time type of this node
	time_container_type m_type;
	
	// The intrinsic time type of this node 
	// Always false for time containers
	bool m_discrete;
			
	// The timer assigned to this node by the timegraph builder
	lib::timer *m_timer;
	
	// The lifetime state of this node.
	// Summarizes the state variables below.
	// For each state the analytic state variables below 
	// take particular values.
	time_state* m_state;
			
	// The current interval associated with this time node.
	// When this has not a current interval this is set to unresolved
	interval_type m_interval;
	
	// The number of valid past intervals this node has created 
	// till now during its lifetime (after exiting ts_new).
	// Any current interval will increment
	// this counter when it ends.
	// Some intervals may have not "played" but they did
	// affected the state of the model by propagating 
	// time change notifications.
	// Canceled intervals do not contribute to the counter.
	long m_picounter;
	
	// Flag set when this is active 
	// e.g during the current interval
	bool m_active;
	
	// Flag set when this node has finished an interval,
	// has called start against its peer playable but not stop yet.
	// e.g there maybe display effects that should be removed
	// == this has to call stop() against its peer playable.
	bool m_needs_remove;
	
	// The following 3 state variables are incemented when the node is active  
	// and it repeats: 
	// m_rad += m_last_cdur; m_rad_offset calc, m_precounter++;
	
	// Accumulated repeat duration
	// Incremented after the completion of a simple dur
	value_type m_rad;
	
	// Last begin or repeat instance in parent simple time.
	// e.g. the accumulated repeat duration (rad) as a parent simple time instance
	// Therefore: simple_dur_offset = parent_simple_time - m_rad_offset;
	value_type m_rad_offset;
	
	// Number of completed repeat counts
	// e.g. the current zero-based repeat index
	long m_precounter;
	
	// Registers for storing activation params
	// Required by the set-of-implicit-dur-on-eom mechanism
	time_type m_activation_time;
	time_type m_media_offset;
	
	// The priority of this node
	// Applicable for excl children.
	int m_priority;
	
	// Paused flag
	bool m_paused;
	time_type m_pause_time;
	
	// Defered flag
	bool m_deferred;
		
	// Sync rules
	typedef std::list<sync_rule*> rule_list;
	
	// The begin sync rules of this node.
	rule_list m_begin_list;
	
	// The end sync rules of this node.
	rule_list m_end_list;
	
	// On reset all event instances are cleared. 
	// Keep a register for holding one such instance
	// The one that will start the current interval
	time_type m_begin_event_inst;
	
	// Special DOM calls sync rule of this node.
	sync_rule *m_domcall_rule;
	
	// The dependents of this node.
	// Use pointers to minimize the overhead when there are no dependents
	typedef std::map<sync_event, rule_list* > dependency_map;
	dependency_map m_dependents;
	
	// rule_list manipulation helpers
	void get_instance_times(const rule_list& rules, time_mset& set) const;
	void reset(rule_list& rules, time_node *src);
	
	// preventing cycles flag
	bool m_locked;
	void lock() { m_locked = true;}
	void unlock() { m_locked = false;}
	bool locked() const { return m_locked;}
	
	// when set the associated renderer should notify for activate events
	bool m_want_activate_events;
	
	// when set the associated UI should notify for accesskey events
	bool m_want_accesskey;
	
	// Cashed continous media node duration (audio, video).
	// A value != time_type::unresolved comes from the playable
	// It is set by querying the playable.  
	time_type m_mediadur;
	
	// Cashed implicit duration of a continous media node (audio, video).
	// It is set to m_mediadur when an EOM event is raised by the playing media node.
	time_type m_impldur;
	
	// Last calc_dur() result
	time_type m_last_cdur;
	
	// S-transitions scheduling
	void schedule_state_transition(time_state_type state, qtime_type timestamp, time_type dt);
	void state_transition_callback(const transition_event *e);
	void schedule_next_timer_event(qtime_type timestamp);
	void timer_event_callback(const timer_event *e);	
	void schedule_sync_update(qtime_type timestamp, time_type dt);
	event *m_pending_event;
	friend class transition_event;
	friend class timer_event;
	friend class active_state;
	enum {sampling_delta = 100};
	
  private:
	// Time states
	time_state* m_time_states[ts_dead+1];
	void create_time_states();
    
	// timing-tree bonds
	time_node *m_parent;
	time_node *m_next;
	time_node *m_child;	
	
	// verifier
	static int node_counter;	
};

class time_container : public time_node {
  public:
	time_container(context_type *ctx, const lib::node *n, time_container_type type) 
	:	time_node(ctx, n, type) {}
	
	~time_container() {}
	
	virtual time_type calc_implicit_dur();
	virtual bool needs_end_sync_update(const time_node *c, qtime_type timestamp) const;
  private:
	time_type calc_implicit_dur_for_esr_first(std::list<const time_node*>& cl);
	time_type calc_implicit_dur_for_esr_last(std::list<const time_node*>& cl);
	time_type calc_implicit_dur_for_esr_all(std::list<const time_node*>& cl);
	time_type calc_implicit_dur_for_esr_id(std::list<const time_node*>& cl);
};

class par : public time_container {
  public:
	par(context_type *ctx, const lib::node *n) 
	:	time_container(ctx, n, tc_par) {}
	~par() {}
};

class seq : public time_container {
  public:
	seq(context_type *ctx, const lib::node *n) 
	:	time_container(ctx, n, tc_seq) {}
	~seq() {}
	virtual time_type calc_implicit_dur();
	virtual bool needs_end_sync_update(const time_node *c, qtime_type timestamp) const;
};

class excl_queue;

class excl : public time_container {
  public:
	excl(context_type *ctx, const lib::node *n); 
	~excl();
	void interrupt(time_node *c, qtime_type timestamp);
	void on_child_normal_end(time_node *c, qtime_type timestamp);
	void built_priorities();
	void remove(time_node *c);
  private:
	time_node* get_active_child();
	excl_queue *m_queue;
	int m_num_classes;
	typedef priority_attrs* priority_attrs_ptr;
	priority_attrs_ptr *m_priority_attrs;

};

class excl_queue {
  public:
	void push_pause(time_node *tn);
	void push_defer(time_node *tn);
	void remove(time_node *tn) { m_cont.remove(tn);}
	bool empty() const { return m_cont.empty();}
	time_node *pop() { 
		if(m_cont.empty()) return 0;
		time_node *tn = m_cont.front(); 
		m_cont.pop_front();
		return tn;
	}
 	void assert_invariants() const;
 private:
	typedef std::list<time_node*> cont;
	cont m_cont;
};

class transition_event : public event_callback<time_node, const transition_event> {
  public:
	typedef event_callback<time_node, const transition_event> evcb;
	transition_event(time_node* tn, time_state_type state, time_traits::qtime_type timestamp) 
	:	evcb(tn, &time_node::state_transition_callback),
		m_state(state), 
		m_timestamp(timestamp) {}
	time_state_type m_state;
	time_traits::qtime_type m_timestamp;
};

class timer_event : public event_callback<time_node, const timer_event> {
  public:
	typedef event_callback<time_node, const timer_event> evcb;
	timer_event(time_node* tn, time_traits::qtime_type timestamp) 
	:	evcb(tn, &time_node::timer_event_callback),
		m_timestamp(timestamp) {}
	time_traits::qtime_type m_timestamp;
};

template <class T> 
T qualify(time_node *n) {
	// Bad, but we can't assume RTTI available
	// return dynamic_cast<T>(n);
	
	return static_cast<T>(n);
}

} // namespace smil2
 
} // namespace ambulant


#ifndef AMBULANT_NO_IOSTREAMS
inline std::ostream& operator<<(std::ostream& os, const ambulant::smil2::time_node& tn) {
	if(tn.get_type() == ambulant::smil2::tc_none)
		os << (tn.dom_node())->get_local_name();
	else 
		os << tn.get_type_as_str();
	return os;
}
#endif

#endif // AMBULANT_SMIL2_TIME_NODE_H
