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

#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/timegraph.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/smil2/animate_n.h"
#include "ambulant/smil2/time_attrs.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/smil2/sync_rule.h"
#include <cmath>
#include <stack>
#include <cstdlib>

//#define AM_DBG if(0)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

timegraph::timegraph(time_node::context_type *ctx, const document *doc, const schema *sch) 
:	m_context(ctx),
	m_schema(sch),
	m_root(0),
	m_dom2tn(0) {
	assert(doc!=0);
	assert(sch!=0);
	m_logger = lib::logger::get_logger();
	m_dom2tn = new std::map<int, time_node*>();
	m_root = build_time_tree(doc->get_root());
	AM_DBG m_logger->trace("Time nodes created: %d", time_node::get_node_counter());
	build_priorities();
	build_time_graph();
	build_timers_graph();
}

timegraph::~timegraph() {
	if(m_root) {
		delete m_root;
		AM_DBG m_logger->trace("Undeleted time nodes: %d", time_node::get_node_counter());
	} // else detached
	if(m_dom2tn)
		delete m_dom2tn;
}


time_node* timegraph::detach_root() {
	time_node* tmp = m_root;
	m_root = 0;
	return tmp;
}

std::map<int, time_node*>* 
timegraph::detach_dom2tn() {
	std::map<int, time_node*>* tmp = m_dom2tn;
	m_dom2tn = 0;
	return tmp;
}

time_node* 
timegraph::build_time_tree(const lib::node *root) {
	const std::set<std::string>& te = m_schema->get_time_elements();
	time_node *time_root = 0;
	std::stack<time_node*> stack;
	std::stack<const lib::node*> switch_stack;
	std::stack<const lib::node*> select_stack;
	lib::node::const_iterator it;
	lib::node::const_iterator end = root->end();
	int localIdCounter = 1;
	for(it = root->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		bool start_element = pair.first;
		const lib::node *n = pair.second;
		const std::string& tag = n->get_local_name();
		
		// keep switch tree ref
		if(tag == "switch") {
			if(start_element) {
				switch_stack.push(n);
				select_stack.push(select_switch_child(n));
			} else  {
				switch_stack.pop();
				select_stack.pop();
			}
		} 
				
		// if not a time element then continue to next
		if(te.find(tag) == te.end()) continue;
		
		// when within a switch and not selected then continue to next
		if(!switch_stack.empty() && (select_stack.top() == 0 ||
			!const_nnhelper::is_descendent(n, select_stack.top()))) {
			// skip node content
			it++;
			while((*it).second != n) it++;
			continue;
		}
		// support inline tests
		test_attrs ta(n);
		if(!ta.selected()) {
			// skip content
			AM_DBG m_logger->trace("Filtering out node: %s[%s]", 
				ta.get_tag().c_str(), ta.get_id().c_str());
			it++;
			while((*it).second != n) it++;
			continue;
		}
		
		if(start_element) {
			// create a time node for each start element
			time_node *tn = create_time_node(n, stack.empty()?0:stack.top());
			
			// read or create node id and add it the map
			std::string ident;
			const char *pid = n->get_attribute("id");
			if(pid) ident = pid;
			else {
				ident += "aid";
				char b[32];sprintf(b,"%u",localIdCounter++);
				ident += b;
			}
			m_id2tn[ident] = tn;
			
			if(stack.empty()) {
				assert(time_root == 0);
				time_root = tn;
			} else {
				// container or media with area or animate elements, etc
				stack.top()->append_child(tn);
			}
			stack.push(tn);
		} else {
			stack.pop();
		}
	}
	return time_root;
}

time_node* 
timegraph::create_time_node(const node* n, time_node* tparent) const {
	time_node *tn = 0;
	time_container_type tct =
		m_schema->get_time_type(n->get_qname());
	if(tct == tc_seq) 
		tn = new seq(m_context, n);
	else if(tct == tc_par) 
		tn = new par(m_context, n);
	else if(tct == tc_excl) 
		tn = new excl(m_context, n);
	else if(m_schema->is_animation(n->get_qname())) 
		tn = animate_node::new_instance(m_context, n, tparent->dom_node());
	else 
		tn = new time_node(m_context, n, tc_none, m_schema->is_discrete(n->get_qname()));
	(*m_dom2tn)[n->get_numid()] = tn;
	return tn;
}

void timegraph::build_time_graph() {
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		if(!(*it).first) continue;
		time_node *tn = (*it).second;
		add_begin_sync_rules(tn);
		add_end_sync_rules(tn);
		if(tn->get_time_attrs()->get_tag()=="area")
			tn->set_want_activate_event(true);
	}
}

void timegraph::build_priorities() {
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		if(!(*it).first) continue;
		time_node *tn = (*it).second;
		if(tn->is_excl()) {
			excl *e = qualify<excl*>(tn);
			e->built_priorities();
		}
	}
}

// Builds the network of timers based on the sync behavior declared in the document
// The spec allows many aspects to be implementation specific  
// XXX: for now assume always "can slip sync behavior".
void timegraph::build_timers_graph() {
	if(!m_context->get_timer())
		return; // not supported by context
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		if(!(*it).first) continue;
		time_node *tn = (*it).second;
		lib::timer *timer = 0;
		if(tn->is_root()) timer = new lib::timer(m_context->get_timer(), 1.0, false);
		else timer = new lib::timer(tn->up()->get_timer(), 1.0, false);
		tn->set_timer(timer);
	}
}

// Adds to the provided time node all begin rules 
void timegraph::add_begin_sync_rules(time_node *tn) {
	// get node begin list
	const time_attrs::sync_list& list = 
		tn->get_time_attrs()->get_begin_list();
	time_attrs::sync_list::const_iterator it;
	time_node *parent = tn->up();
	
	if(!parent) {
		// root is special
		return;
	}
		
	if(list.empty()) {
		// add implicit begin rule
		sync_rule *sr = create_impl_syncbase_begin_rule(tn);
		tn->add_begin_rule(sr);
		return;
	}
	
	// add any specified begin offsets rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if((svs.type == sv_offset || svs.type == sv_indefinite) && 
			(!parent->is_seq() || (parent->is_seq() && svs.offset>=0))) {
			sync_rule *sr = create_impl_syncbase_rule(tn, svs.offset);
			tn->add_begin_rule(sr);
			if(parent->is_seq()) break;
		} 
	}
	
	if(parent->is_seq()) {
		// For children of a sequence, the only legal value for 
		// begin is a single non-negative offset value.
		return;
	}
	
	// add any specified begin syncbase rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if(svs.type == sv_syncbase) {
			time_node *base = get_node_with_id(svs.base, tn);
			if(!base) continue;
			if(svs.event == "begin") {
				sync_rule *sr = new model_rule(base, tn_begin, svs.offset);
				tn->add_begin_rule(sr);
			} else if(svs.event == "end") {
				sync_rule *sr = new model_rule(base, tn_end, svs.offset);
				tn->add_begin_rule(sr);
			}
		}
	}
	
	// add any specified begin event rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if(svs.type == sv_event) {
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) continue;
			sync_event event = sync_event_from_str(svs.event);
			if(event == tn_activate_event)
				base->set_want_activate_event(true);			
			sync_rule *sr = new event_rule(base, event, svs.offset);
			tn->add_begin_rule(sr);
		} else if(svs.type == sv_repeat) {
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) continue;
			sync_rule *sr = new event_rule(base, tn_repeat_event, svs.offset, svs.iparam);
			tn->add_begin_rule(sr);
		} else if(svs.type == sv_accesskey) {
			tn->want_accesskey(true);
			sync_rule *sr = new event_rule(m_root, accesskey_event, svs.offset, svs.iparam);
			tn->add_begin_rule(sr);
		} else if(svs.type == sv_media_marker) {
			sync_rule *sr = new event_rule(tn, tn_marker_event, svs.offset, svs.sparam);
			tn->add_begin_rule(sr);
		}
	}
}

// Adds to the provided time node all end rules 
void timegraph::add_end_sync_rules(time_node *tn) {
	// get node end list
	const time_attrs::sync_list& list = 
		tn->get_time_attrs()->get_end_list();
	time_attrs::sync_list::const_iterator it;
	time_node *parent = tn->up();
	
	// root is special
	if(!parent) return;
	
	if(list.empty()) return;
	
	// add any specified end offsets rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if(svs.type == sv_offset) {
			sync_rule *sr = create_impl_syncbase_rule(tn, svs.offset);
			tn->add_end_rule(sr);
		}
	}
	
	// add any specified end syncbase rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if(svs.type == sv_syncbase) {
			time_node *base = get_node_with_id(svs.base, tn);
			if(!base) continue;
			if(svs.event == "begin") {
				sync_rule *sr = new model_rule(base, tn_begin, svs.offset);
				tn->add_end_rule(sr);
			} else if(svs.event == "end") {
				sync_rule *sr = new model_rule(base, tn_end, svs.offset);
				tn->add_end_rule(sr);
			}
		}
	}
	
	// add any specified end event rules
	for(it = list.begin(); it!= list.end(); it++) {
		const sync_value_struct& svs = *it;
		if(svs.type == sv_event) {
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) continue;
			sync_event event = sync_event_from_str(svs.event);
			if(event == tn_activate_event)
				base->set_want_activate_event(true);
			sync_rule *sr = new event_rule(base, event, svs.offset);
			tn->add_end_rule(sr);
		} else if(svs.type == sv_repeat) {
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) continue;
			sync_rule *sr = new event_rule(base, tn_repeat_event, svs.offset, svs.iparam);
			tn->add_end_rule(sr);
		} else if(svs.type == sv_accesskey) {
			tn->want_accesskey(true);
			sync_rule *sr = new event_rule(m_root, accesskey_event, svs.offset, svs.iparam);
			tn->add_end_rule(sr);
		} else if(svs.type == sv_media_marker) {
			sync_rule *sr = new event_rule(tn, tn_marker_event, svs.offset, svs.sparam);
			tn->add_end_rule(sr);
		}
	}
	
}

sync_rule*
timegraph::create_impl_syncbase_begin_rule(time_node *tn) {
	time_node *parent = tn->up();
	assert(parent!=0);
	sync_rule *sr = 0;
	if(parent->is_par()) {
		sr = new offset_rule(parent, tn_begin, 0);
	} else if(parent->is_seq()) {
		time_node *previous = tn->previous();
		if(!previous) {
			sr = new offset_rule(parent, tn_begin, 0);
		} else {
			sr = new model_rule(previous, tn_end, 0);
		}
	} else if(parent->is_excl()) {
		sr = new offset_rule(parent, tn_begin, time_type::indefinite());
	} else {
		// area, animation
		sr = new offset_rule(parent, tn_begin, 0);
	}
	return sr;
}

sync_rule*
timegraph::create_impl_syncbase_rule(time_node *tn, time_type offset) {
	time_node *parent = tn->up();
	assert(parent!=0);
	sync_rule *sr = 0;
	if(parent->is_par()) {
		sr = new offset_rule(parent, tn_begin, offset);
	} else if(parent->is_seq()) {
		time_node *previous = tn->previous();
		if(!previous) {
			sr = new offset_rule(parent, tn_begin, offset);
		} else {
			sr = new model_rule(previous, tn_end, offset);
		}
	} else if(parent->is_excl()) {
		sr = new offset_rule(parent, tn_begin, offset);
	} else {
		// area, animation
		sr = new offset_rule(parent, tn_begin, offset);
	}
	return sr;
}

time_node* 
timegraph::get_node_with_id(const std::string& ident) const {
	std::map<std::string, time_node*>::const_iterator it = m_id2tn.find(ident);
	return (it != m_id2tn.end())?(*it).second:0;
}

time_node* 
timegraph::get_node_with_id(const std::string& ident, time_node *tn) const {
	std::map<std::string, time_node*>::const_iterator it = m_id2tn.find(ident);
	if(it != m_id2tn.end()) return (*it).second;
	// check also for special logical syncbases 
	if(ident == "prev") return tn->previous();
	return 0;
}

const lib::node* 
timegraph::select_switch_child(const node* sn) const {
	std::list<const node*> cl;
	std::list<const node*>::const_iterator it;
	sn->get_children(cl);
	for(it=cl.begin();it!=cl.end();it++) {
		test_attrs ta(*it);
		if(!ta.selected()) {
			AM_DBG m_logger->trace("Filtering out node: %s[%s]", 
				ta.get_tag().c_str(), ta.get_id().c_str());
		} else return (*it);
	}
	return 0;
}

