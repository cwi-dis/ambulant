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

#ifndef AMBULANT_SMIL2_SYNC_RULE_H
#define AMBULANT_SMIL2_SYNC_RULE_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include <set>
#include <cassert>

// used by sync_list
#include <list>

namespace ambulant {

namespace smil2 {

// Time node and global "happenings" that may be used for synchronization.
enum sync_event {
	// Model abstract events raised by a time node.
	// A model abstract event is raised by a time node
	// when the associated time value becomes known or changes.
	// The timestamp associated with a model abstract event
	// is the associated time value.
	tn_begin, tn_repeat, tn_end,

	// Model real events raised by a time node.
	tn_begin_event, tn_repeat_event, tn_end_event,

	// DOM events raised by a time node.
	tn_activate_event,
	tn_focusin_event,
	tn_focusout_event,
	tn_inbounds_event,
	tn_outofbounds_event,

	// DOM events raised globally and may affect a time node.
	accesskey_event,

	// Media marker event
	tn_marker_event,

	// State change event
	state_change_event,
	// tev in smilText seen
	tev_event,

	// DOM calls
	tn_dom_call,

	// Placeholder for unknown events
	tn_unknown_event
};

sync_event sync_event_from_str(const std::string& s);

const char* sync_event_str(sync_event ev);


// A sync rule is a condition associated with a timing node
// that specifies directly or indirectly within the context
// of the model, when the node should begin or end.
//
// A sync rule depends either on the occurrence of a global
// event or on something "happening" to a timing node.
// Node happenings includes both real and abstract model events.
//
// When the sync rule depends on something happening
// to a timing node, this node is called the syncbase
// of the rule. The syncbase node keeps a pointer to the rule,
// called syncarc, and when the associated real or abstract
// model event "occurs", the syncbase node updates the rule.
//
// The syncbase update notification includes a timestamp
// having a value as dictated by the model.
// For real events the timestamp is the time
// of the occurrence of the event.
// For model events the timestamp is a calculated
// time instance which can be in the past or in the future.

// A sync_rule, as designed below, is a stand alone object
// at the same level as a time node is.
// Though it belongs to a time node (the target) this is
// not visible within the sync rule implementation.
// It has all the available info to do whatever calculations
// it needs that otherwise would have to delegate to its owner.

class time_node;

// Rule types
enum rule_type { rt_begin, rt_end, rt_transout};

const char* rule_type_str(rule_type rt);

class sync_rule : public time_traits {
  public:
	// The dstr should be virtual since concrete rules will
	// be deleted through sync_rule base pointers.
	virtual ~sync_rule() {}

	// Appends to the time set arg any resolved instance times.
	// The times appended are relative to target's implicit sync node.
	// The target node is the owner of this rule
	// and also the caller of this function.
	virtual void get_instance_times(time_mset& s) const {}

	// Notification when a target's ancestor begins or repeats
	// Clears instance times as dictated by the model.
	// Syncbased and media-marker instance times are cleared
	// when the src argument node is a common ascendant
	// of both the syncbase and the target.
	// Instance times due to events, e.g. due to any of
	// tn_begin_event, tn_repeat_event, tn_end_event,
	// tn_activate_event, accesskey_event, and dom calls
	// are always cleared by this function
	virtual void reset(time_node *src) {}

	// Update notifications coming from the syncbase node when
	// a new interval is created or when an interval is updated or canceled.
	// e.g. when a tn_begin or a tn_end abstract model event
	// is raised by the syncbase node.
	// The times appended are relative to syncbase's implicit sync node.
	virtual void new_instance(qtime_type timestamp, time_type instance) {}
	virtual void cancel_instance(qtime_type timestamp, time_type instance) {}
	virtual void update_instance(qtime_type timestamp, time_type instance, time_type old_instance) {}

	// Notifications when a global or a syncbased event is raised.
	// e.g. any event except a tn_begin or a tn_end
	// The instances are relative to syncbase's implicit
	// sync node clock or relative to the document's clock for global events.
	virtual void add_instance(qtime_type timestamp, time_type instance, int data = 0) {}
	virtual void add_instance(qtime_type timestamp, time_type instance, const std::string& data) {}

	// Internal function that sets the target of this rule.
	// The target node is the owner of this rule
	// and also the caller of get_instance_times().
	virtual void set_target(time_node *tn, rule_type rt) = 0;

	// Internal function that sets the syncnbase and syncevent of this rule.
	// For rules that depend on global events such as
	// an access key or dom calls, the syncbase is assumed
	// to be the time root.
	virtual void set_syncbase(time_node *tn, sync_event se) = 0;

	virtual time_node* get_target() = 0;
	virtual time_node* get_syncbase() = 0;
	virtual rule_type get_target_attr() = 0;
	virtual sync_event get_syncbase_event() = 0;
	virtual std::string to_string() = 0;
	virtual void set_trace_mode(bool b) = 0;

};

// A base class providing an implementation for the internal functions.
class sync_rule_impl : public sync_rule {
  public:
	sync_rule_impl(time_node *sb, sync_event se);
	void set_target(time_node *tn, rule_type rt);
	void set_syncbase(time_node *tn, sync_event se);
	time_node* get_target() { return m_target;}
	time_node* get_syncbase() { return m_syncbase;}
	rule_type get_target_attr() { return m_target_attr;}
	sync_event get_syncbase_event() { return m_syncbase_event;}
	std::string to_string();
	void set_trace_mode(bool b) {m_trace = b;}
  protected:
	void eval_refnode();
	time_type to_ref(time_type instance) const;
	time_type from_ref(time_type instance) const;
	void lock() { m_locked = true;}
	void unlock() { m_locked = false;}
	bool locked() const { return m_locked;}

	time_node *m_target;
	rule_type m_target_attr;
	time_node *m_syncbase;
	sync_event m_syncbase_event;
	time_node *m_refnode;
	bool m_locked;
	bool m_trace;
};

// A basic model rule
class model_rule : public sync_rule_impl {
  public:
	model_rule(time_node *sb, sync_event se, time_type offset)
	:	sync_rule_impl(sb, se),
		m_offset(offset)  {}
	virtual void get_instance_times(time_mset& s) const;
	virtual void reset(time_node *src);
	virtual void new_instance(qtime_type timestamp, time_type instance);
	virtual void cancel_instance(qtime_type timestamp, time_type instance);
	virtual void update_instance(qtime_type timestamp, time_type instance, time_type old_instance);
  protected:
	time_type m_offset;
	time_list m_instances;
};

// A basic event rule
class event_rule : public sync_rule_impl {
  public:
	event_rule(time_node *sb, sync_event se, value_type offset = 0, int selector = 0)
	:	sync_rule_impl(sb, se),
		m_offset(offset),
		m_selector(selector)  {}
	event_rule(time_node *sb, sync_event se, value_type offset, const std::string& selector)
	:	sync_rule_impl(sb, se),
		m_offset(offset),
		m_selector(0),
		m_str_selector(selector)  {}
	virtual void get_instance_times(time_mset& s) const;
	virtual void reset(time_node *src);
	virtual void add_instance(qtime_type timestamp, time_type instance, int data = 0);
	virtual void add_instance(qtime_type timestamp, time_type instance, const std::string& data);
protected:
	time_type m_offset;
	int m_selector;
	std::string m_str_selector;
	time_list m_instances;
};

// A simple offset rule.
// An offset_rule is special case of a model_rule.
// The implementation avoid inheritance from a model_rule for optinization reasons
class offset_rule : public sync_rule_impl {
  public:
	offset_rule(time_node *sb, sync_event se, time_type offset)
	:	sync_rule_impl(sb, se),
		m_offset(offset)  {}
	virtual void get_instance_times(time_mset& s) const { s.insert(m_offset);}
  protected:
	time_type m_offset;
};

// A special model rule for transOut
class transout_rule : public sync_rule_impl {
  public:
	transout_rule(time_node *sb, sync_event se, time_type offset)
	:	sync_rule_impl(sb, se),
		m_offset(offset)  {}
	virtual void get_instance_times(time_mset& s) const;
	virtual void reset(time_node *src);
	virtual void new_instance(qtime_type timestamp, time_type instance);
	virtual void cancel_instance(qtime_type timestamp, time_type instance);
	virtual void update_instance(qtime_type timestamp, time_type instance, time_type old_instance);
  protected:
	time_type m_offset;
	time_list m_instances;
};

// A trigger rule is a special model rule that differs
// from the rest, in that it does not contribute to the
// model through its instance times.
// A trigger rule may link dependent nodes when the dependency
// cannot be expressed through instance times.
// The alt mechanism implemented by a trigger rule
// is knowledge transfer between nodes.
// The syncbase node uses this link to wake up
// the target when the target may take benefit
// of the newly acquired info at the syncbase.
class trigger_rule : public sync_rule_impl {
  public:
	trigger_rule(time_node *sb, sync_event se, time_type offset)
	:	sync_rule_impl(sb, se), m_offset(offset) {}
	virtual void new_instance(qtime_type timestamp, time_type instance);
	virtual void cancel_instance(qtime_type timestamp, time_type instance);
	virtual void update_instance(qtime_type timestamp, time_type instance, time_type old_instance);
  protected:
	time_type m_offset;
};

// Sync rules context interface.
// Used by a sync rule to notify its context
// that something has changed.
// The context of a sync_rule is a time node.
// Therefore time nodes should implement this interface.
class sync_rule_context : public time_traits {
  public:
	virtual ~sync_rule_context() {}

	// Called by a sync rule when its state has changed.
	// The context time node should re-evaluate its state.
	// If the evaluation results to a change
	// then the time node should update any dependents.
	virtual void sync_update(qtime_type timestamp) = 0;
};


} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_SYNC_RULE_H
