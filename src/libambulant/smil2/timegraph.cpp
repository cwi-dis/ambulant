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

//#define AM_DBG if(1)
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
	AM_DBG m_logger->debug("Time nodes created: %d", time_node::get_node_counter());
	build_priorities();
	build_time_graph();
	build_timers_graph();
	build_trans_out_graph();
}

timegraph::~timegraph() {
	if(m_root) {
		delete m_root;
		AM_DBG m_logger->debug("Undeleted time nodes: %d", time_node::get_node_counter());
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
			bool dynamic_cc = common::preferences::get_preferences()->m_dynamic_content_control;
			static bool warned = false;
			if (!warned && dynamic_cc) {
				warned = true;
				lib::logger::get_logger()->trace("WARNING: dynamic content control not implemented for switch");
			}
			if(start_element) {
				switch_stack.push(n);
				select_stack.push(select_switch_child(n));
			} else  {
				switch_stack.pop();
				select_stack.pop();
			}
		}
		// <a actuate="onLoad"/> we treat as area. This is not pretty, but it
		// works.
		bool is_onload_a = false;
		if (tag == "a" && n->down() == NULL) {
			const char *actuate = n->get_attribute("actuate");
			if (actuate && strcmp(actuate, "onLoad") == 0)
				is_onload_a = true;
		}
		// if not a time element then continue to next
		if(te.find(tag) == te.end() && !is_onload_a) continue;

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
		bool dynamic_cc = common::preferences::get_preferences()->m_dynamic_content_control;
		if(!dynamic_cc && !ta.selected()) {
			// skip content
			AM_DBG m_logger->debug("Filtering out node: %s[%s]",
				ta.get_tag().c_str(), ta.get_id().c_str());
			it++;
			while((*it).second != n) it++;
			continue;
		}

		if(start_element) {
			// create a time node for each start element
			time_node *tn = create_time_node(n, stack.empty()?0:stack.top());

			// add 'a' parent as a child
			if(n->up() && n->up()->get_local_name() == "a") {
				time_node *tnp = create_time_node(n->up(), tn);
				tn->append_child(tnp);
				const char *pidp = n->get_attribute("id");
				if(pidp) m_id2tn[pidp] = tn;
			}

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
				//assert(time_root == 0); // xxxbo: comment out temporarily. 
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
		m_schema->get_time_type(n->get_local_name());
	if(tct == tc_seq)
		tn = new seq(m_context, n);
	else if(tct == tc_par)
		tn = new par(m_context, n);
	else if(tct == tc_excl)
		tn = new excl(m_context, n);
	else if(m_schema->is_animation(n->get_local_name()))
		tn = animate_node::new_instance(m_context, n, tparent->dom_node());
	else
		tn = new time_node(m_context, n, tc_none, m_schema->is_discrete(n->get_local_name()));
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
		if(tn->is_link() && tn->get_time_attrs()->get_actuate() == actuate_onrequest) {
			tn->set_want_activate_event(true);
		}
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
		lib::timer_control *timer = 0;
		if(tn->is_root()) timer = new lib::timer_control_impl(m_context->get_timer(), 1.0, false);
		else timer = new lib::timer_control_impl(tn->up()->get_timer(), 1.0, false);
		tn->set_timer(timer);
	}
}

// Adds sync rules for out transitions
void timegraph::build_trans_out_graph() {
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		if(!(*it).first) continue;
		time_node *tn = (*it).second;
		const time_attrs *ta = tn->get_time_attrs();
		if(!ta->get_trans_out()) continue;
		time_type offset = -ta->get_trans_out_dur();
		// We should now arrange so that the out transition
		// starts before the node is removed.
		// To do this we need to consider its freeze behaviour and its context
		AM_DBG m_logger->debug("%s[%s] transOut with fill: %s start:%ld ms before remove",
			ta->get_tag().c_str(), ta->get_id().c_str(), repr(ta->get_fill()).c_str(), offset());
		if(ta->get_fill() == fill_remove) {
			sync_rule *sr = new transout_rule(tn, tn_end, offset);
			tn->set_transout_rule(sr);
		} else if(ta->get_fill() == fill_freeze) {
			// XXXX This is a hack to make the default work, at least
			// What really should happen depends on the parent, etc.
			sync_rule *sr = new transout_rule(tn, tn_end, offset);
			tn->set_transout_rule(sr);
		} // else not implemented yet
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
		if((svs.type == sv_offset || svs.type == sv_indefinite)
			//XXXX Fix for #2950428 (negative begin time).
//				&& (!parent->is_seq() || (parent->is_seq() && svs.offset>=0)
		) {
			long offset = svs.offset <= 0 ? 0 : svs.offset;
			sync_rule *sr = create_impl_syncbase_rule(tn, offset);
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
			sync_event event = sync_event_from_str(svs.event);
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) {
				// Special case code for beginEvent on interior smiltext nodes:
				// these are implemented as marker events (because interior smiltext
				// nodes have no corresponding time_node).
				if (event == tev_event) {
					// Get the node referenced by base
					const lib::node *n = tn->dom_node();
					assert(n);
					const lib::node_context *ctx = n->get_context();
					assert(ctx);
					const lib::node *dom_base = ctx->get_node(svs.base);
					// Find its smiltext ancestor
					while (dom_base && dom_base->get_local_name() != "smilText")
						dom_base = dom_base->up();
					// Create the marker event
					if (!dom_base) {
						m_logger->trace("%s: not a time node, not a smiltext node", svs.base.c_str());
						continue;
					}
					AM_DBG m_logger->debug("Add tn_marker_event rule, domnode=%s, id=%s", dom_base->get_sig().c_str(), svs.base.c_str());
					assert(m_dom2tn);
					base = (*m_dom2tn)[dom_base->get_numid()];
					sync_rule *sr = new event_rule(base, tn_marker_event, svs.offset, svs.base);
					tn->add_begin_rule(sr);
					continue;
				}
				continue;
			}
			if(event == tn_activate_event)
				base->set_want_activate_event(true);
			else if (event == tn_focusin_event)
				base->set_want_focusin_event(true);
			else if (event == tn_focusout_event)
				base->set_want_focusout_event(true);
			else if (event == tn_inbounds_event)
				base->set_want_inbounds_event(true);
			else if (event == tn_outofbounds_event)
				base->set_want_outofbounds_event(true);
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
		} else if(svs.type == sv_state_change) {
			AM_DBG m_logger->debug("Adding state change event to 0x%x\n", m_root);
			m_state_change_args.insert(svs.sparam);
			sync_rule *sr = new event_rule(m_root, state_change_event, svs.offset, svs.sparam);
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
			sync_event event = sync_event_from_str(svs.event);
			time_node *base = svs.base.empty()?tn:get_node_with_id(svs.base, tn);
			if(!base) {
				// Special case code for beginEvent on interior smiltext nodes:
				// these are implemented as marker events (because interior smiltext
				// nodes have no corresponding time_node).
				if (event == tev_event) {
					// Get the node referenced by base
					const lib::node *n = tn->dom_node();
					assert(n);
					const lib::node_context *ctx = n->get_context();
					assert(ctx);
					const lib::node *dom_base = ctx->get_node(svs.base);
					// Find its smiltext ancestor
					while (dom_base && dom_base->get_local_name() != "smilText")
						dom_base = dom_base->up();
					// Create the marker event
					if (!dom_base) {
						m_logger->trace("%s: not a time node, not a smiltext node", svs.base.c_str());
						continue;
					}
					AM_DBG m_logger->debug("Add tn_marker_event rule, domnode=%s, id=%s", dom_base->get_sig().c_str(), svs.base.c_str());
					assert(m_dom2tn);
					base = (*m_dom2tn)[dom_base->get_numid()];
					sync_rule *sr = new event_rule(base, tn_marker_event, svs.offset, svs.base);
					tn->add_end_rule(sr);
					continue;
				}
				continue;
			}
			if(event == tn_activate_event)
				base->set_want_activate_event(true);
			else if (event == tn_focusin_event)
				base->set_want_focusin_event(true);
			else if (event == tn_focusout_event)
				base->set_want_focusout_event(true);
			else if (event == tn_inbounds_event)
				base->set_want_inbounds_event(true);
			else if (event == tn_outofbounds_event)
				base->set_want_outofbounds_event(true);
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
		} else if(svs.type == sv_state_change) {
			AM_DBG m_logger->debug("Adding state change event to 0x%x\n", m_root);
			m_state_change_args.insert(svs.sparam);
			sync_rule *sr = new event_rule(m_root, state_change_event, svs.offset, svs.sparam);
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

static bool
nodeLangSortPredicate(const node* lhs, const node* rhs)
{
	const char *lhsLang = lhs->get_attribute("systemLanguage");
	if (lhsLang == NULL) lhsLang = lhs->get_attribute("system-language");
	float lhsPrio = 0.0f;
	if (lhsLang) {
		lhsPrio = test_attrs::get_system_language_weight(lhsLang);
	}

	const char *rhsLang = rhs->get_attribute("systemLanguage");
	if (rhsLang == NULL) rhsLang = rhs->get_attribute("system-language");
	float rhsPrio = 0.0f;
	if (rhsLang) {
		rhsPrio = test_attrs::get_system_language_weight(rhsLang);
	}

	return lhsPrio >= rhsPrio;
}

const lib::node*
timegraph::select_switch_child(const node* sn) const {
	// Note: this implementation reutnrs only a single switch child. if that child
	// is not usable for some other reason than selected() (i.e. unknown URL) it
	// will not fallback to the next node.
	std::list<const node*> cl;
	std::list<const node*>::const_iterator it;
	sn->get_children(cl);
	// If allowReorder is true we reorder the children based on language
	// preference.
	const char *reorder = sn->get_attribute("allowReorder");
	if (reorder && strcmp(reorder, "yes") == 0) {
		AM_DBG lib::logger::get_logger()->debug("select_switch_child(%s): reordering children", sn->get_sig().c_str());
		cl.sort(nodeLangSortPredicate);
	}
	for(it=cl.begin();it!=cl.end();it++) {
		if ((*it)->is_data_node()) continue;
		test_attrs ta(*it);
		if(!ta.selected()) {
			AM_DBG m_logger->debug("Filtering out node: %s[%s]",
				ta.get_tag().c_str(), ta.get_id().c_str());
		} else return (*it);
	}
	return 0;
}

