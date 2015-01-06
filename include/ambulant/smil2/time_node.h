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

#ifndef AMBULANT_SMIL2_TIME_NODE_H
#define AMBULANT_SMIL2_TIME_NODE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/node_navigator.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/sync_rule.h"
#include "ambulant/smil2/time_attrs.h"
#include "ambulant/smil2/time_state.h"
#include "ambulant/smil2/time_nctx.h"

#include <cassert>
#include <utility>
#include <list>
#include <map>

namespace ambulant {

namespace smil2 {

// fwd declaration.
class time_calc;

// Represents a node in the timing model.

class time_node : public schedulable {
  public:
	typedef time_node_context context_type;
	typedef node_navigator<time_node> nnhelper;
	typedef node_navigator<const time_node> const_nnhelper;

	time_node(context_type *ctx, const node *n, time_container_type type = tc_none, bool discrete = false);

	virtual ~time_node();

	// TimeElement DOM interface
	// Currently support only startElement for the root.
	virtual void start();
	virtual void stop();
	virtual void pause();
	virtual void resume();
	virtual void reset();

	// driver interface
	virtual void exec(qtime_type timestamp);
	void get_pending_events(std::map<time_type, std::list<time_node*> >& events);

	// Sets the timer for this node
	// This node becomes the owner of the timer e.g. should delete it on exit
	// The timegraph builder has already established the clocks network
	virtual void set_timer(lib::timer_control *tmr) { m_timer = tmr;}
	virtual lib::timer_control *get_timer() { return m_timer;}

	// Timegarph configuration
	void set_want_activate_event(bool want) { m_want_activate_events = want;}
	void set_want_inbounds_event(bool want) { m_want_inbounds_events = want;}
	void set_want_outofbounds_event(bool want) { m_want_outofbounds_events = want;}
	void set_want_focusin_event(bool want) { m_want_focusin_events = want;}
	void set_want_focusout_event(bool want) { m_want_focusout_events = want;}
	bool wants_activate_event() const { return m_want_activate_events;}
	bool wants_inbounds_event() const { return m_want_inbounds_events;}
	bool wants_outofbounds_event() const { return m_want_outofbounds_events;}
	bool wants_focusin_event() const { return m_want_focusin_events;}
	bool wants_focusout_event() const { return m_want_focusout_events;}
	void want_accesskey(bool want) { m_want_accesskey = want;}
	bool want_accesskey() const { return m_want_accesskey;}

	// Functions that may be overriden by subclasses
	virtual time_node *append_child(time_node *child) {return nnhelper::append_child(this, child);}
	virtual void get_children(std::list<time_node*>& l) { nnhelper::get_children(this, l);}
	virtual void get_children(std::list<const time_node*>& l) const { const_nnhelper::get_children(this, l);}
	virtual time_type get_implicit_dur();

	// End sync functions
	virtual bool end_sync_cond_applicable() const { return false;}
	virtual bool end_sync_cond() const { return true;}

	// Begin and end conditions evaluator
	virtual bool begin_cond(qtime_type timestamp);
	virtual bool end_cond(qtime_type timestamp);

	// Feedback
	void node_started();

	// Forced transitions
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void reset_children(qtime_type timestamp, time_node *oproot);
	virtual void startup_children(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void kill_children(qtime_type timestamp, time_node *oproot);
	virtual void kill_blockers(qtime_type timestamp, time_node *oproot);

	// Sync update
	virtual void sync_update(qtime_type timestamp);

	// Begin of media update
	void on_bom(qtime_type timestamp);

	// End of media update
	void on_eom(qtime_type timestamp);

	// Return true if the node is waiting for on on_eom callback.
	bool want_on_eom();

	// Pause of media update
	void on_pom(qtime_type timestamp);

	// Resume of media update
	void on_rom(qtime_type timestamp);

	// End of simple duration update. Returns true if anything happened (repeat).
	bool on_eosd(qtime_type timestamp);

	// End of (visual) transition
	void on_transitioned(qtime_type timestamp);

	// Raising events
	virtual void raise_begin_event(qtime_type timestamp);
	virtual void raise_repeat_event(qtime_type timestamp);
	virtual void raise_end_event(qtime_type timestamp, time_node *oproot);
	virtual void raise_activate_event(qtime_type timestamp);
	virtual void raise_inbounds_event(qtime_type timestamp);
	virtual void raise_outofbounds_event(qtime_type timestamp);
	virtual void raise_focusin_event(qtime_type timestamp);
	virtual void raise_focusout_event(qtime_type timestamp);
	virtual void raise_accesskey(std::pair<qtime_type, int> accesskey);
	virtual void raise_marker_event(std::pair<qtime_type, std::string> args);
	virtual void raise_state_change(std::pair<qtime_type, std::string> statearg);
	virtual void raise_update_event(qtime_type timestamp);

	// Interval manipulators
	void set_interval(qtime_type timestamp, const interval_type& i);
	void cancel_interval(qtime_type timestamp);
	void update_interval(qtime_type timestamp, const interval_type& i);
	void update_interval_end(qtime_type timestamp, time_type newend);
	void played_interval(qtime_type timestamp);
	void clear_history() { m_history.clear(); }
	bool can_set_interval(qtime_type timestamp, const interval_type& i);

	// excl
	void defer_interval(qtime_type timestamp);

	// Node activities
	void activate(qtime_type timestamp);
	void repeat(qtime_type timestamp);
	void remove(qtime_type timestamp);
	void fill(qtime_type timestamp);
	void pause(qtime_type timestamp, pause_display d);
	void resume(qtime_type timestamp);
	void check_repeat(qtime_type timestamp);

	// Anchors and areas
	void follow_link(qtime_type timestamp);

	// Playable commands
	common::playable *create_playable();
	void start_playable(time_type offset = 0);
	void seek_playable(time_type offset);
	void pause_playable(common::pause_display d = common::display_show);
	void resume_playable();
	void repeat_playable();
	void stop_playable();
	time_type get_playable_dur();
	void prepare_playables();

	// Animations are special internal playables
	void start_animation(time_type offset);
	void stop_animation();
	// State commands (setvalue, send) are special internal playables
	void start_statecommand(time_type offset);
	// Prefetch is special internal playables
	void start_prefetch(time_type offset = 0);

	// Std xml tree navigation interface
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

	void get_path(std::list<time_node*>& path) { nnhelper::get_path(this, path);}
	void get_path(std::list<const time_node*>& path) { const_nnhelper::get_path(this, path);}

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
	time_type get_rad() const { return m_rad;}
	time_type get_pad() const { return m_pad;}
	value_type get_simple_time() const;
	time_type get_interval_end() const;
	value_type get_time() const;

	// Time type queries and classification
	time_container_type get_type() const { return m_type;}
	const char* get_type_as_str() const { return time_container_type_as_str(m_type);}
	bool is_time_container() const { return m_type != tc_none;}
	bool is_seq() const { return m_type == tc_seq;}
	bool is_par() const { return m_type == tc_par;}
	bool is_excl() const { return m_type == tc_excl;}
#if 0
	// Handling discrete media different than continuous media
	// is a bad idea from a SMIL point of view (the tag name
	// really is documentary only), and practically it
	// caused problems with AmisAmbulant, which wants to
	// pause after putting up a text node to allow the screen
	// reader to do its thing.
	// Code temporarily re-enabled because of bug #1553249.
	// And now disabled again, hoping the new seek code will also fix this
	bool is_discrete() const { return m_discrete;}
#else
	bool is_discrete() const { return false;}
#endif
	bool is_root() const { return !up();}
	bool is_cmedia() const {return !is_time_container() && !is_discrete();}
	bool is_area() const { return m_attrs.get_tag() == "area";}
	bool is_a() const { return m_attrs.get_tag() == "a";}
	bool is_link() const { return is_area() || is_a();}
	bool is_animation() const;
	bool is_statecommand() const;
	bool is_prefetch() const;
	bool is_playable() const;

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

	// Sets the transOut rule for this node.
	void set_transout_rule(sync_rule *sr);

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
	void on_add_instance(qtime_type timestamp, sync_event ev, time_type instance, std::string data, time_node *filter=0);

	// Returns the lifetime state handler object of this time node
	time_state* get_state() { return m_state;}
	const time_state* get_state() const { return m_state;}

	// Retuns the context of this time node
	context_type* get_context() {return m_context;}
	void set_context(context_type *ctx) { m_context = ctx;}

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

	// For excl children, mainly: return true if the node is in fill mode
	bool is_filled() const { return !m_active && m_needs_remove; }

	// Returns the current interval associated with this (maybe invalid)
	const interval_type& get_current_interval() const {
		return m_interval;
	}
	// Returns the first interval associated with this (maybe invalid)
	const interval_type& get_first_interval(bool asdoc = false) const;

	// Returns the last interval associated with this (maybe invalid)
	const interval_type& get_last_interval() const;

	// Returns true when this has played any interval
	bool played() const { return !m_history.empty();}

	// Returns the last calc dur
	// Used to avoid recursion when a child makes calcs
	// that depend on parent simple dur
	// but parent's simple dur depends on the child
	time_type get_last_dur() const { return m_last_cdur;}

	// Returns the priority class of this node
	// Applicable for excl children.
	int priority() const { return m_priority;}
	void set_priority(int prio) { m_priority = prio;}

	// Excl set/get flags
	bool paused() const { return m_paused;}
	void set_paused(bool b) { m_paused = b;}
	bool deferred() const { return m_deferred;}
	void set_deferred(bool b) { m_deferred = b;}

	// fast forward mode
	void set_ffwd_mode(bool b);

	// Hyperjump-specific handling
	void set_hyperjump_mode(bool b) { m_in_hyperjump_path = b; }

	// synchronise playable clock to time_node
	void sync_playable_clock();

	////////////////////////
	// Time calculations

	// Calculates the simple duration of this node
	time_type calc_dur();

	// Calculate interval
	interval_type calc_first_interval();
	interval_type calc_next_interval(interval_type prev = interval_type::unresolved);

	// Re-calculate current interval end
	time_type calc_current_interval_end();

	std::string to_string() const;

	std::string get_sig() const;
	// Verifier
	static int get_node_counter() {return node_counter;}

	friend class time_state;

	// public S/O transitions
	void set_deferred_interval(qtime_type timestamp);
	void set_begin_event_inst(time_type inst) {m_begin_event_inst = inst;}

	// Debug method
	bool has_debug(const char *attrvalue=NULL) const { return m_node->has_debug(attrvalue); }
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
	lib::timer_control *m_timer;

	// The lifetime state of this node.
	// Summarizes the state variables below.
	// For each state the analytic state variables below
	// take particular values.
	time_state	*m_state;

	// Smil timing calculator
	time_calc *m_time_calc;

	// The current interval associated with this time node.
	// When this has not a current interval this is set to unresolved
	interval_type m_interval;

	// Past intervals
	// Some intervals may have not "played" but they did
	// affected the state of the model by propagating
	// time change notifications.
	// Canceled intervals do not contribute.
	std::list<interval_type> m_history;
	std::list<interval_type> m_doc_history;

	// Flag set when this is active
	// e.g during the current interval
	bool m_active;

	// Flag set when this node has finished an interval,
	// has called start against its peer playable but not stop yet.
	// e.g there maybe display effects that should be removed
	// == this has to call stop() against its peer playable.
	bool m_needs_remove;

	// Accumulated repeat duration
	// Incremented after the completion of a simple dur
	// Last begin or repeat instance as measured by the AD timer of this node.
	time_type m_rad;

	// Number of completed repeat counts
	// e.g. the current zero-based repeat index
	long m_precounter;

	// End Of Media (EOM) flag
	bool m_eom_flag;

	// The priority of this node
	// Applicable for excl children.
	int m_priority;

	// Paused flag
	bool m_paused;

	// Accumulated pause duration
	time_type m_pad;

	// Register for storing pause time
	time_type m_paused_sync_time;

	// Defered flag
	bool m_deferred;

	// Fast forward mode flag
	bool m_ffwd_mode;

	// Flag that signals this node is part of the path to the node we're hyperjumping to.
	bool m_in_hyperjump_path;

	// Sync update event
	std::pair<bool, qtime_type> m_update_event;

	// Sync rules
	typedef std::list<sync_rule*> rule_list;

	// The begin sync rules of this node.
	rule_list m_begin_list;

	// The end sync rules of this node.
	rule_list m_end_list;

	// The transOut sync rule of this node.
	sync_rule *m_transout_sr;

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

	// when set the associated renderer should notify for DOM events
	bool m_want_activate_events;
	bool m_want_focusin_events;
	bool m_want_focusout_events;
	bool m_want_inbounds_events;
	bool m_want_outofbounds_events;

	// when set the associated UI should notify for accesskey events
	bool m_want_accesskey;

	// Cashed implicit duration of a continous media node (audio, video).
	// It is set to m_mediadur when an EOM event is raised by the playing media node.
	time_type m_impldur;

	// Last calc_dur() result
	time_type m_last_cdur;

	// Provide access to the states
	friend class reset_state;
	friend class proactive_state;
	friend class active_state;
	friend class postactive_state;
	friend class dead_state;

	// logger
	lib::logger *m_logger;

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

	// Verify that renderers produce started() and stopped() callbacks in the right order.
	bool m_saw_on_bom, m_saw_on_eom;
};

class time_container : public time_node {
  public:
	time_container(context_type *ctx, const lib::node *n, time_container_type type)
	:	time_node(ctx, n, type) {}

	~time_container() {}

	virtual time_type get_implicit_dur();
	virtual bool end_sync_cond_applicable() const;
	virtual bool end_sync_cond() const;
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
	virtual time_type get_implicit_dur();
	virtual bool end_sync_cond() const;
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
	time_node* get_filled_child();
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

template <class T>
T qualify(time_node *n) {
	// Bad, but we can't assume RTTI available
	// return dynamic_cast<T>(n);

	return static_cast<T>(n);
}

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_NODE_H
