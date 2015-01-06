// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/logger.h"
#include "ambulant/smil2/sync_rule.h"
#include "ambulant/smil2/time_node.h"
#include <list>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include <sstream>

using namespace ambulant;
using namespace smil2;


//////////////////////////////////
// sync_rule_impl implementation

sync_rule_impl::sync_rule_impl(time_node *syncbase, sync_event se)
:	m_target(0),
	m_target_attr(rt_begin),
	m_syncbase(syncbase),
	m_syncbase_event(se),
	m_refnode(0),
	m_locked(false),
#ifdef AM_DBG
	m_trace(true) {}
#else
	m_trace(false) {}
#endif

void sync_rule_impl::set_target(time_node *tn, rule_type rt) {
	m_target = tn;
	m_target_attr = rt;
	eval_refnode();
}

void sync_rule_impl::set_syncbase(time_node *tn, sync_event se) {
	m_syncbase = tn;
	m_syncbase_event = se;
	eval_refnode();
}

void sync_rule_impl::eval_refnode() {
	if(!m_refnode && m_target && m_syncbase) {
		typedef lib::node_navigator<time_node> nnhelper;
		m_refnode = nnhelper::get_common_ancestor(m_target->sync_node(),
			m_syncbase->sync_node());
	}
}

sync_rule::time_type
sync_rule_impl::to_ref(time_type instance) const {
	if(!instance.is_definite()) return instance;
	assert(m_syncbase && m_refnode);
	qtime_type tc(m_syncbase->sync_node(), instance);
	return tc.to_ancestor(m_refnode);
}

sync_rule::time_type
sync_rule_impl::from_ref(time_type instance) const {
	assert(m_target && m_refnode);
	if(!instance.is_definite()) return instance;
	qtime_type tc(m_refnode, instance);
	return tc.to_descendent(m_target->sync_node());
}

std::string sync_rule_impl::to_string() {
	std::ostringstream os;
	os << m_target->to_string() << "."  << rule_type_str(m_target_attr);
	os << "=";
	if(m_syncbase == m_target->up())
		os << "parent." << sync_event_str(m_syncbase_event) << " + offset";
	else if(m_syncbase == m_target)
		os << "self." << sync_event_str(m_syncbase_event) << " + offset";
	else if(m_syncbase == m_target->previous())
		os << "previous." << sync_event_str(m_syncbase_event) << " + offset";
	else
		os << m_syncbase->to_string() << "." << sync_event_str(m_syncbase_event) << "+offset";
	return os.str();
}

//////////////////////////////////
// offset_rule implementation is inline


//////////////////////////////////
// model_rule implementation

void model_rule::get_instance_times(time_mset& s) const {
	for(time_list::const_iterator it = m_instances.begin();it!=m_instances.end();it++)
		s.insert(from_ref(*it) + m_offset);
}

void model_rule::reset(time_node *src) {
	// !src means a document reset.
	if(!src || m_refnode->is_descendent_of(src))
		m_instances.clear();
}

void model_rule::new_instance(qtime_type timestamp, time_type instance) {
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->debug("model_rule::new_instance(%ld) (%s)", instance(), to_string().c_str());
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

void model_rule::cancel_instance(qtime_type timestamp, time_type instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->debug("Internal error: model_rule::cancel_instance(%ld) failed. List is empty (%s)",
			instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
		unlock();
		return;
	}
	time_type ref_instance = to_ref(instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), ref_instance);
	if(it != m_instances.end())
		m_instances.erase(it);
	else {
		logger::get_logger()->error("Internal error: model_rule::cancel_instance(%ld) failed (%s)",
			instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
	}
	m_target->sync_update(timestamp);
	unlock();
}

void model_rule::update_instance(qtime_type timestamp, time_type instance, time_type old_instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->error("Internal error: model_rule::update_instance(%ld, %ld) failed. List is empty (%s)",
			instance(), old_instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
		unlock();
		return;
	}
	time_type old_ref_instance = to_ref(old_instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), old_ref_instance);
	if(it != m_instances.end())
		(*it) = to_ref(instance);
	else {
		logger::get_logger()->error("Internal error: model_rule::update_instance(%ld, %ld) failed (%s)",
			instance(), old_instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
	}
	m_target->sync_update(timestamp);
	unlock();
}

//////////////////////////////////
// event_rule implementation

void event_rule::get_instance_times(time_mset& s) const {
	for(time_list::const_iterator it = m_instances.begin();it!=m_instances.end();it++)
		s.insert(from_ref(*it) + m_offset);
}

void event_rule::reset(time_node *src) {
	m_instances.clear();
}

void event_rule::add_instance(qtime_type timestamp, time_type instance, int data) {
	if(m_selector != data) return;
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->debug("event_rule::add_instance(%ld, %d) (%s)[%d]", instance(), data, to_string().c_str(), m_selector);
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

void event_rule::add_instance(qtime_type timestamp, time_type instance, const std::string& data) {
	if(m_str_selector != data) return;
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->debug("event_rule::add_instance(%ld, %s) (%s)[%s]", instance(), data.c_str(), to_string().c_str(), m_str_selector.c_str());
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

//////////////////////////////////
// transout_rule implementation

void transout_rule::get_instance_times(time_mset& s) const {
	for(time_list::const_iterator it = m_instances.begin();it!=m_instances.end();it++)
		s.insert(from_ref(*it) + m_offset);
}

void transout_rule::reset(time_node *src) {
	// !src means a document reset.
	if(!src || m_refnode->is_descendent_of(src))
		m_instances.clear();
}

void transout_rule::new_instance(qtime_type timestamp, time_type instance) {
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->debug("transout_rule::new_instance(%ld) (%s)", instance(), to_string().c_str());
	m_instances.push_back(to_ref(instance));
	unlock();
}

void transout_rule::cancel_instance(qtime_type timestamp, time_type instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->error("Internal error: model_rule::cancel_instance(%ld) failed. List is empty (%s)",
			instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
		unlock();
		return;
	}
	time_type ref_instance = to_ref(instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), ref_instance);
	if(it != m_instances.end())
		m_instances.erase(it);
	else {
		logger::get_logger()->error("Internal error: model_rule::cancel_instance(%ld) failed (%s)",
			instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
	}
	unlock();
}

void transout_rule::update_instance(qtime_type timestamp, time_type instance, time_type old_instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->error("Internal error: model_rule::update_instance(%ld, %ld) failed. List is empty (%s)",
			instance(), old_instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
		unlock();
		return;
	}
	time_type old_ref_instance = to_ref(old_instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), old_ref_instance);
	if(it != m_instances.end())
		(*it) = to_ref(instance);
	else {
		logger::get_logger()->error("Internal error: model_rule::update_instance(%ld, %ld) failed (%s)",
			instance(), old_instance(), to_string().c_str());
		logger::get_logger()->error(gettext("Programmer error, attempting to continue"));
	}
	unlock();
}

//////////////////////////////////
// trigger_rule implementation

void trigger_rule::new_instance(qtime_type timestamp, time_type instance) {
	//m_target->sync_update(timestamp);
	logger::get_logger()->debug("trigger_rule::new_instance");
}

void trigger_rule::cancel_instance(qtime_type timestamp, time_type instance) {
	m_target->sync_update(timestamp);
}

void trigger_rule::update_instance(qtime_type timestamp, time_type instance, time_type old_instance) {
	m_target->sync_update(timestamp);
}
////////////////////////////

sync_event
ambulant::smil2::sync_event_from_str(const std::string& s) {
	static std::map<std::string, sync_event> events;
	typedef std::string st;
	if(events.empty()) {
		events[st("begin")] = tn_begin;
		events[st("end")] = tn_end;
		events[st("beginEvent")] = tn_begin_event;
		events[st("endEvent")] = tn_end_event;
		events[st("repeat")] = tn_repeat_event;
		events[st("activateEvent")] = tn_activate_event;
		events[st("focusInEvent")] = tn_focusin_event;
		events[st("focusOutEvent")] = tn_focusout_event;
		events[st("inBoundsEvent")] = tn_inbounds_event;
		events[st("outOfBoundsEvent")] = tn_outofbounds_event;
		events[st("click")] = tn_activate_event;
		events[st("marker")] = tn_marker_event;
		events[st("stateChange")] = state_change_event;
		events[st("tevEvent")] = tev_event;
		events[st("accesskey")] = accesskey_event;
	}
	std::map<std::string, sync_event>::iterator it = events.find(s);
	return (it != events.end())?(*it).second:tn_unknown_event;
}

////////////////////////////
// tracing helpers

const char*
ambulant::smil2::sync_event_str(sync_event ev) {
	switch(ev) {
		case tn_begin: return "begin";
		case tn_repeat: return "repeat";
		case tn_end: return "end";
		case tn_begin_event: return "beginEvent";
		case tn_repeat_event: return "repeat(.)";
		case tn_end_event: return "endEvent";
		case tn_activate_event: return "activateEvent";
		case tn_focusin_event: return "focusInEvent";
		case tn_focusout_event: return "focusOutEvent";
		case tn_inbounds_event: return "inBoundsEvent";
		case tn_outofbounds_event: return "outOfBoundsEvent";
		case tn_marker_event: return "marker";
		case state_change_event: return "stateChange";
		case tev_event: return "tevEvent";
		case accesskey_event: return "accesskey";
		case tn_dom_call: return "beginElement()";
		case tn_unknown_event: break;
	}
	return "unknownEvent";
}

const char*
ambulant::smil2::rule_type_str(rule_type rt) {
	switch(rt) {
		case rt_begin: return "begin";
		case rt_end: return "end";
		case rt_transout: return "transOut";
	}
	return "unknown";
}
