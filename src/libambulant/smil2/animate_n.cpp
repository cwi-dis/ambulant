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

#include "ambulant/smil2/animate_n.h"
#include "ambulant/smil2/animate_f.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/region_info.h"

#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"

#include "ambulant/lib/logger.h"

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

animate_node::animate_node(context_type *ctx, const node *n, animate_attrs *aattrs)
:	time_node(ctx, n, tc_none, false), 
	m_aattrs(aattrs) {
}

animate_node::~animate_node() {
	delete m_aattrs;
}

void animate_node::prepare_interval() {
}

bool animate_node::apply_value(common::animation_destination *dst) {
	// Verify against the only attribute that is implemented currently by the layout
	lib::color_t oldcolor = dst->get_bgcolor();
	lib::color_t newcolor = 0xff;
	if(oldcolor != newcolor) {
		dst->set_bgcolor(0xFF);
		return true;
	}
	return false;
}

////////////////////////////////////
// animate_reg_dim_node
// "left", "top", "width", "height", "right", "bottom"
// as common::region_dim

template <class F>
class animate_reg_dim_node : public animate_node {
  public:
	animate_reg_dim_node(context_type *ctx, const node *n, animate_attrs *aattrs);
	~animate_reg_dim_node();
	
	void prepare_interval();
	bool apply_value(common::animation_destination *dst);
	
  private:
	//linear_map_f<common::region_dim> m_simple_f;
	//animate_f<linear_map_f<common::region_dim> > *m_animate_f;
	F m_simple_f;
	animate_f<F> *m_animate_f;
};

template <class F>
animate_reg_dim_node<F>::animate_reg_dim_node(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs) {
	std::vector<common::region_dim> v;
	m_aattrs->get_values(v);
	if(aattrs->get_calc_mode() == "paced") {
		init_map_f_paced(calc_dur()(), v, m_simple_f);
	} else {
		init_map_f(calc_dur()(), v, m_simple_f);
	}
}

template <class F>
animate_reg_dim_node<F>::~animate_reg_dim_node() {
	delete m_animate_f;
}

template <class F>
void animate_reg_dim_node<F>::prepare_interval() {
	time_type ad = m_interval.end - m_interval.begin;
	m_animate_f = new animate_f<F>(m_simple_f, ad(), m_aattrs->is_accumulative());
}

template <class F>
bool animate_reg_dim_node<F>::apply_value(common::animation_destination *dst) {
	if(!m_animate_f) return false;
	lib::timer::time_type t = m_timer->elapsed();
	lib::logger::get_logger()->trace("%s(%ld) -> %s", 
		m_aattrs->get_target_attr().c_str(), t, ::repr(m_animate_f->at(t)).c_str());
		
	/*
	if(m_aattrs->is_additive() && !is_effvalue_animation())
		add_value();
	else
		set_value();
	*/
	
	// return true when attr has changed else false
	return false;
}

////////////////////////////////////
// animate_bgcolor_node

template <class F>
class animate_bgcolor_node : public animate_node {
  public:
	animate_bgcolor_node(context_type *ctx, const node *n, animate_attrs *aattrs);
	~animate_bgcolor_node();
	
	void prepare_interval();
	bool apply_value(common::animation_destination *dst);
	
  private:
	F m_simple_f;
	animate_f<F> *m_animate_f;
};

template <class F>
animate_bgcolor_node<F>::animate_bgcolor_node(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs) {
	std::vector<lib::color_t> v;
	m_aattrs->get_color_values(v);
	if(aattrs->get_calc_mode() == "paced") {
		init_map_f_paced(calc_dur()(), v, m_simple_f);
	} else {
		init_map_f(calc_dur()(), v, m_simple_f);
	}
}

template <class F>
animate_bgcolor_node<F>::~animate_bgcolor_node() {
	delete m_animate_f;
}

template <class F>
void animate_bgcolor_node<F>::prepare_interval() {
	time_type ad = m_interval.end - m_interval.begin;
	m_animate_f = new animate_f<F>(m_simple_f, ad(), m_aattrs->is_accumulative());
}

template <class F>
bool animate_bgcolor_node<F>::apply_value(common::animation_destination *dst) {
	if(!m_animate_f) return false;
	lib::timer::time_type t = m_timer->elapsed();
	lib::color_t newcolor = m_animate_f->at(t);
	dst->set_bgcolor(newcolor);
	// XXX: check for additivity
	// XXX: return true when attr has changed else false
	return true;
}

////////////////////////////////////

//static 
animate_node* animate_node::new_instance(context_type *ctx, const node *n, const node* tparent) {
	animate_attrs *aattrs = new animate_attrs(n, tparent);
	if(aattrs->get_target_attr_type() == "reg_dim") {
		if(aattrs->get_calc_mode() == "discrete") {
			typedef discrete_map_f<common::region_dim> F;
			return new animate_reg_dim_node<F>(ctx, n, aattrs);
		} else {
			typedef linear_map_f<common::region_dim> F;
			return new animate_reg_dim_node<F>(ctx, n, aattrs);
		}
	} else if(aattrs->get_target_attr_type() == "bgcolor") {
		if(aattrs->get_calc_mode() == "discrete") {
			typedef discrete_map_f<lib::color_t> F;
			return new animate_bgcolor_node<F>(ctx, n, aattrs);
		} else {
			typedef linear_map_f<lib::color_t> F;
			return new animate_bgcolor_node<F>(ctx, n, aattrs);
		}
	}
	return new animate_node(ctx, n, aattrs);
}




