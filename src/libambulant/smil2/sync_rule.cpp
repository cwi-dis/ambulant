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

#include "ambulant/lib/logger.h"
#include "ambulant/smil2/sync_rule.h"
#include "ambulant/smil2/time_node.h"
#include <list>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <strstream>
#endif

using namespace ambulant;
using namespace smil2;


//////////////////////////////////
// abstract_sync_rule implementation

abstract_sync_rule::abstract_sync_rule(time_node *syncbase, sync_event se) 
:	m_target(0),
	m_target_attr(rt_begin),
	m_syncbase(syncbase),
	m_syncbase_event(se),
	m_refnode(0),
	m_locked(false),
	m_trace(false) {}

void abstract_sync_rule::set_target(time_node *tn, rule_type rt) { 
	m_target = tn; 
	m_target_attr = rt; 
	eval_refnode();
}

void abstract_sync_rule::set_syncbase(time_node *tn, sync_event se) { 
	m_syncbase = tn; 
	m_syncbase_event = se;
	eval_refnode();
}	

void abstract_sync_rule::eval_refnode() { 
	if(!m_refnode && m_target && m_syncbase) {
		typedef lib::node_navigator<time_node> nnhelper;
		m_refnode = nnhelper::get_common_ancestor(m_target->sync_node(), 
			m_syncbase->sync_node());
	}
}

sync_rule::time_type 
abstract_sync_rule::to_ref(time_type instance) const {
	if(!instance.is_definite()) return instance;
	assert(m_syncbase && m_refnode);
	qtime_type tc(m_syncbase->sync_node(), instance);
	return tc.to_ancestor(m_refnode);
}

sync_rule::time_type 
abstract_sync_rule::from_ref(time_type instance) const {
	assert(m_target && m_refnode);
	if(!instance.is_definite()) return instance;
	qtime_type tc(m_refnode, instance);
	return tc.to_descendent(m_target->sync_node());
}

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
std::string abstract_sync_rule::to_string() {
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
#endif 

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
		AM_DBG logger::get_logger()->trace("model_rule::new_instance(%ld) (%s)", instance(), to_string().c_str());	
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

void model_rule::cancel_instance(qtime_type timestamp, time_type instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->error("model_rule::cancel_instance(%ld) failed. List is empty (%s)", 
			instance(), to_string().c_str());
		unlock();
		return;	
	}
	time_type ref_instance = to_ref(instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), ref_instance);
	if(it != m_instances.end())
		m_instances.erase(it);
	else 
		logger::get_logger()->error("model_rule::cancel_instance(%ld) failed (%s)", 
			instance(), to_string().c_str());	
	m_target->sync_update(timestamp);
	unlock();
}

void model_rule::update_instance(qtime_type timestamp, time_type instance, time_type old_instance) {
	if(locked()) return;
	lock();
	if(m_instances.empty()) {
		logger::get_logger()->error("model_rule::update_instance(%ld, %ld) failed. List is empty (%s)", 
			instance(), old_instance(), to_string().c_str());
		unlock();
		return;	
	}
	time_type old_ref_instance = to_ref(old_instance);
	time_list::iterator it = std::find(m_instances.begin(), m_instances.end(), old_ref_instance);
	if(it != m_instances.end())
		(*it) = to_ref(instance);
	else 
		logger::get_logger()->error("model_rule::update_instance(%ld, %ld) failed (%s)", 
			instance(), old_instance(), to_string().c_str());	
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
	// XXX: Consider exception: 
	// do not remove event instances that created the 'current' interval
	m_instances.clear();
}

void event_rule::add_instance(qtime_type timestamp, time_type instance, int data) {
	if(m_selector != data) return;
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->trace("event_rule::add_instance(%ld, %d) (%s)[%d]", instance(), data, to_string().c_str(), m_selector);	
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

void event_rule::add_instance(qtime_type timestamp, time_type instance, const std::string& data) {
	if(m_str_selector != data) return;
	if(locked()) return;
	lock();
	if(m_trace)
		AM_DBG logger::get_logger()->trace("event_rule::add_instance(%ld, %s) (%s)[%s]", instance(), data.c_str(), to_string().c_str(), m_str_selector.c_str());	
	m_instances.push_back(to_ref(instance));
	m_target->sync_update(timestamp);
	unlock();
}

//////////////////////////////////
// trigger_rule implementation

void trigger_rule::new_instance(qtime_type timestamp, time_type instance) {
	m_target->sync_update(timestamp);
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
		events[st("click")] = tn_activate_event;
		events[st("marker")] = tn_marker_event;
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
		case tn_marker_event: return "marker";
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
	}
	return "unknown";
}
