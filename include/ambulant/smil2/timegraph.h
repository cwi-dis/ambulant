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

#ifndef AMBULANT_SMIL2_TIMEGRAPH_H
#define AMBULANT_SMIL2_TIMEGRAPH_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/smil2/test_attrs.h"

#include <string>
#include <map>


namespace ambulant {

// gcc 3.4 complains about this, since it does not seems to be used i uncommented it.

//template <class N>
//class lib::node_navigator;

namespace smil2 {

// Builds the time tree and the time graph.
// Wraps the time root.

class timegraph : public time_traits {
  public:
	timegraph(time_node::context_type *ctx,
		const lib::document *doc,
		const common::schema *sch);
	~timegraph();

	time_node* get_root() { return m_root;}
	const time_node* get_root() const { return m_root;}
	const std::set<std::string>& get_state_change_args() {return m_state_change_args; }

	time_node* detach_root();
	std::map<int, time_node*>* detach_dom2tn();

  private:
	typedef node_navigator<const lib::node> const_nnhelper;
	time_node* build_time_tree(const lib::node *root);
	void build_priorities();
	void build_time_graph();
	void build_timers_graph();
	void build_trans_out_graph();
	time_node* create_time_node(const lib::node *n, time_node* tparent) const;
	time_node *get_node_with_id(const std::string& ident) const;
	time_node *get_node_with_id(const std::string& ident, time_node *tn) const;

	// helpers for creating sync rules
	void add_begin_sync_rules(time_node *tn);
	void add_end_sync_rules(time_node *tn);
	sync_rule* create_impl_syncbase_begin_rule(time_node *tn);
	sync_rule* create_impl_syncbase_rule(time_node *tn, time_type offset);

	const lib::node* select_switch_child(const node* sn) const;

	time_node::context_type *m_context;
	const common::schema *m_schema;
	const std::map<std::string, custom_test>* m_custom_tests;
	time_node* m_root;
	std::map<std::string, time_node*> m_id2tn;
	std::map<int, time_node*> *m_dom2tn;
	lib::logger *m_logger;
	std::set<std::string> m_state_change_args;
};


} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_TIMEGRAPH_H
