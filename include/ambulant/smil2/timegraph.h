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

#ifndef AMBULANT_SMIL2_TIMEGRAPH_H
#define AMBULANT_SMIL2_TIMEGRAPH_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/time_node.h"

#include <string>
#include <map>

#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

namespace ambulant {

namespace smil2 {


class lib::document;
class common::schema;
class lib::node;
template <class N>
class lib::node_navigator;

// Builds the time tree and the time graph.
// Wraps the time root.
 
class timegraph : public time_traits {
  public:
 public:
	timegraph(time_node::context_type *ctx, const lib::document *doc, const common::schema *sch);
	~timegraph();
	
	time_node* get_root() { return m_root;}
	const time_node* get_root() const { return m_root;}
	
	time_node* detach_root();
	std::map<int, time_node*>* detach_dom2tn();
	
#ifndef AMBULANT_NO_IOSTREAMS
	void dump(std::ostream& os);
#endif

  private:
    typedef node_navigator<const lib::node> const_nnhelper;
	time_node* build_time_tree(const lib::node *root);
	void build_time_graph();
	time_node* create_time_node(const lib::node *n) const;
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
	time_node* m_root;
	std::map<std::string, time_node*> m_id2tn;
	std::map<int, time_node*> *m_dom2tn;
};


} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TIMEGRAPH_H
