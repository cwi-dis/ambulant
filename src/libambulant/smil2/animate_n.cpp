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

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define IGNORE_ATTR_COMP true

using namespace ambulant;
using namespace smil2;

////////////////////////////////////
// animate_node
//
// An animate_node is the base class for all animation node flavors

animate_node::animate_node(context_type *ctx, const node *n, animate_attrs *aattrs)
:	time_node(ctx, n, tc_none, false), 
	m_aattrs(aattrs) {
}

animate_node::~animate_node() {
	delete m_aattrs;
}

void animate_node::prepare_interval() {
}

void animate_node::read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
}

bool animate_node::set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
	return false;
}

void animate_node::apply_self_effect(animate_registers& regs) const {
}

////////////////////////////////////
// linear_values animation

// A linear_values_animation is applicable for linear-attributes
// linear-attributes define: addition, subtraction and scaling
// A linear_values_animation may be continuous or discrete 
// The interpolation mode (calcMode) of a continuous linear_values animation may be linear, paced or spline

// F: the simple function
// T: the type of the attribute
// The type T must define addition, subtraction and scaling
template <class F, class T>
class linear_values_animation : public animate_node {
  public:
	linear_values_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	~linear_values_animation();
	
	void prepare_interval();
	
  protected:
	bool verify_key_times(std::vector<double>& keyTimes);
	
	F m_simple_f;
	animate_f<F> *m_animate_f;
	std::vector<T> m_values;
	
};

template <class F, class T>
linear_values_animation<F, T>::linear_values_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs), m_animate_f(0)  {
	m_aattrs->get_values(m_values);
}

template <class F, class T>
linear_values_animation<F, T>::~linear_values_animation() {
	delete m_animate_f;
}

template <class F, class T>
void linear_values_animation<F, T>::prepare_interval() {
	time_type dur = calc_dur();
	time_type sfdur = dur;
	const time_attrs* ta = get_time_attrs();
	if(dur.is_definite() && ta->auto_reverse()) sfdur /= 2;
	if(m_aattrs->get_calc_mode() == "paced") {
		m_simple_f.paced_init(sfdur(), m_values);
	} else {
		std::vector<double> keyTimes;
		m_aattrs->get_key_times(keyTimes);
		bool keyTimesValid = keyTimes.empty() || verify_key_times(keyTimes);
		if(!keyTimes.empty() && keyTimesValid) {
			m_simple_f.init(sfdur(), m_values, keyTimes);
		} else	
			m_simple_f.init(sfdur(), m_values);
	}
	m_simple_f.set_auto_reverse(ta->auto_reverse());
	m_simple_f.set_accelerate(ta->get_accelerate(), ta->get_decelerate());	
	time_type ad = m_interval.end - m_interval.begin;
	m_animate_f = new animate_f<F>(m_simple_f, dur(), ad(), m_aattrs->is_accumulative());
}

template <class F, class T>
bool linear_values_animation<F, T>::verify_key_times(std::vector<double>& keyTimes) {
	bool keyTimesValid = true;
	if(keyTimes.empty()) return true;
	if(keyTimes.front() != 0.0 || keyTimes.size() != m_values.size())
		keyTimesValid = false;
	if(m_aattrs->get_calc_mode() != "discrete" && keyTimes.back() != 1.0)
		keyTimesValid = false;
	for(size_t i=1;i<keyTimes.size() && keyTimesValid ;i++) 	
		keyTimesValid = (keyTimes[i]>keyTimes[i-1]);
	if(!keyTimesValid)
		m_logger->warn("%s[%s] invalid key times", m_attrs.get_tag().c_str(), m_attrs.get_id().c_str());
	AM_DBG {
		if(!keyTimes.empty() && keyTimesValid) {
			std::string str;
			for(size_t i=0;i<keyTimes.size();i++) {
				char sz[16];sprintf(sz,"%.3f;", keyTimes[i]); str += sz;
			}
			m_logger->trace("%s[%s] keyTimes: %s", 
				m_attrs.get_tag().c_str(), m_attrs.get_id().c_str(), str.c_str());
		}
	}
	return keyTimesValid;
}

////////////////////////////////////
// underlying_to_animation

// A underlying_to_animation is applicable for linear-attributes
// linear-attributes define: addition, subtraction and scaling

// F: the simple function
// T: the type of the attribute
// The type T must define addition, subtraction and scaling
template <class T>
class underlying_to_animation : public animate_node {
  public:
	underlying_to_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	~underlying_to_animation();
	
	void prepare_interval();
		
  protected:
	typedef underlying_to_f<T> F;
	F m_simple_f;
	animate_f<F> *m_animate_f;
	T m_value;
};

template <class T>
underlying_to_animation<T>::underlying_to_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs), m_animate_f(0)  {
	std::vector<T> v;
	m_aattrs->get_values(v);
	m_value = v[0];
}

template <class T>
underlying_to_animation<T>::~underlying_to_animation() {
	delete m_animate_f;
}

template <class T>
void underlying_to_animation<T>::prepare_interval() {
	time_type dur = calc_dur();
	time_type sfdur = dur;
	const time_attrs* ta = get_time_attrs();
	if(dur.is_definite() && ta->auto_reverse()) sfdur /= 2;
	m_simple_f.init(sfdur(), m_value);
	m_simple_f.set_auto_reverse(ta->auto_reverse());
	m_simple_f.set_accelerate(ta->get_accelerate(), ta->get_decelerate());
	time_type ad = m_interval.end - m_interval.begin;
	m_animate_f = new animate_f<F>(m_simple_f, dur(), ad(), m_aattrs->is_accumulative());
}

////////////////////////////////////
// regdim_animation
//
// A regdim_animation may be used for all region dim animations except for "to" animations
// 
// Animateable region/subregion attributes: "left", "top", "width", "height", "right", "bottom"

template <class F>
class regdim_animation : public linear_values_animation<F, common::region_dim> {
  public:
	regdim_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, common::region_dim>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.rd = dst->get_region_dim(m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::region_dim rd = dst->get_region_dim(m_aattrs->get_target_attr(), false);
		if(rd != regs.rd || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %s", 
					m_aattrs->get_target_attr().c_str(), t, ::repr(regs.rd).c_str());
			}
			dst->set_region_dim(m_aattrs->get_target_attr(), regs.rd);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		common::region_dim rd = m_animate_f->at(t);
		if(m_aattrs->is_additive())
			regs.rd += rd; // add
			
		else
			regs.rd = rd; // override
	}
};

class underlying_to_regdim_animation : public underlying_to_animation<common::region_dim> {
  public:
	underlying_to_regdim_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<common::region_dim>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.rd = dst->get_region_dim(m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::region_dim rd = dst->get_region_dim(m_aattrs->get_target_attr(), false);
		if(rd != regs.rd || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %s", 
					m_aattrs->get_target_attr().c_str(), t, ::repr(regs.rd).c_str());
			}		
			dst->set_region_dim(m_aattrs->get_target_attr(), regs.rd);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		common::region_dim rd = m_animate_f->at(t, regs.rd);
		regs.rd = rd; // override
	}
};

////////////////////////////////////
// color_animation
//
// A color_animation may be used for all backgroundColor/color animations except for "to" animations
// 
// Animateable region/subregion attributes: "backgroundColor", "color"


template <class F>
class color_animation : public linear_values_animation<F, lib::color_t> {
  public:
	color_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, lib::color_t>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.cl = dst->get_region_color(m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		lib::color_t cl = dst->get_region_color(m_aattrs->get_target_attr(), false);
		if(cl != regs.cl || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> 0x%x", 
					m_aattrs->get_target_attr().c_str(), t, regs.cl);
			}				
			dst->set_region_color(m_aattrs->get_target_attr(), regs.cl);
			return true;
		}
		return false;
	}
	
	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		lib::color_t cl = m_animate_f->at(t);
		if(m_aattrs->is_additive())
			regs.cl += cl; // add
		else
			regs.cl = cl; // override
	}
};

class underlying_to_color_animation : public underlying_to_animation<lib::color_t> {
  public:
	underlying_to_color_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<lib::color_t>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.cl = dst->get_region_color(m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		lib::color_t cl = dst->get_region_color(m_aattrs->get_target_attr(), false);
		if(cl != regs.cl || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> 0x%x", 
					m_aattrs->get_target_attr().c_str(), t, regs.cl);
			}				
			dst->set_region_color(m_aattrs->get_target_attr(), regs.cl);
			return true;
		}
		return false;
	}
	
	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		regs.cl = m_animate_f->at(t, regs.cl); // override
	}
};

////////////////////////////////////

template <class F>
class zindex_animation : public linear_values_animation<F, common::zindex_t> {
  public:
	zindex_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, common::zindex_t>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.zi = dst->get_region_zindex(true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::zindex_t zi = dst->get_region_zindex(false);
		if(zi != regs.zi || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %d", 
					m_aattrs->get_target_attr().c_str(), t, regs.zi);
			}				
			dst->set_region_zindex(regs.zi);
			return true;
		}
		return false;
	}
	
	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		common::zindex_t zi = m_animate_f->at(t);
		if(m_aattrs->is_additive())
			regs.zi += zi; // add
		else
			regs.zi = zi; // override
	}
};

class underlying_to_zindex_animation : public underlying_to_animation<common::zindex_t> {
  public:
	underlying_to_zindex_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<int>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.zi = dst->get_region_zindex(true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::zindex_t zi = dst->get_region_zindex(false);
		if(zi != regs.zi || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %d", 
					m_aattrs->get_target_attr().c_str(), t, regs.zi);
			}				
			dst->set_region_zindex(regs.zi);
			return true;
		}
		return false;
	}
	
	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		common::zindex_t zi = m_animate_f->at(t, regs.zi);
		regs.zi = zi; // override
	}
};
////////////////////////////////////
// values_motion_animation
//
// A values_motion_animation may be used for all position animations except for "to" and "path" animations
// 
// Animateable region/subregion attributes: "position"

template <class F>
class values_motion_animation : public linear_values_animation<F, lib::point> {
  public:
	values_motion_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, lib::point>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		common::region_dim left = dst->get_region_dim("left", true);
		common::region_dim top = dst->get_region_dim("top", true);
		regs.pt.x = left.get_as_int();
		regs.pt.y = top.get_as_int();
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		region_dim left = dst->get_region_dim("left", false);
		region_dim top = dst->get_region_dim("top", false);
		lib::point pt(left.get_as_int(), top.get_as_int());
		if(pt != regs.pt || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %s", 
					m_aattrs->get_target_attr().c_str(), t, ::repr(regs.pt).c_str());
			}			
			dst->set_region_dim("left", common::region_dim(regs.pt.x));			
			dst->set_region_dim("top", common::region_dim(regs.pt.y));			
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		lib::point pt = m_animate_f->at(t);
		if(m_aattrs->is_additive())
			regs.pt += pt; // add
		else
			regs.pt = pt; // override
	}
};

class underlying_to_motion_animation : public underlying_to_animation<lib::point> {
  public:
	underlying_to_motion_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<lib::point>(ctx, n, aattrs) {}
	
	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		common::region_dim left = dst->get_region_dim("left", true);
		common::region_dim top = dst->get_region_dim("top", true);
		regs.pt.x = left.get_as_int();
		regs.pt.y = top.get_as_int();
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		region_dim left = dst->get_region_dim("left", false);
		region_dim top = dst->get_region_dim("top", false);
		lib::point pt(left.get_as_int(), top.get_as_int());
		if(pt != regs.pt || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->trace("%s(%ld) -> %s", 
					m_aattrs->get_target_attr().c_str(), t, ::repr(regs.pt).c_str());
			}		
			dst->set_region_dim("left", common::region_dim(regs.pt.x));			
			dst->set_region_dim("top", common::region_dim(regs.pt.y));			
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		lib::point pt = m_animate_f->at(t, regs.pt);
		regs.pt = pt; // override
	}
};

////////////////////////////////////
// animate_node factory functions

// private static 
animate_node* animate_node::new_regdim_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef common::region_dim attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new regdim_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_regdim_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new regdim_animation<F>(ctx, n, aattrs);
}

// private static 
animate_node* animate_node::new_color_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef lib::color_t attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new color_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_color_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new color_animation<F>(ctx, n, aattrs);
}

// private static 
animate_node* animate_node::new_zindex_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef common::zindex_t attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new zindex_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_zindex_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new zindex_animation<F>(ctx, n, aattrs);
}

// private static 
animate_node* animate_node::new_position_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef lib::point attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new values_motion_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_motion_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new values_motion_animation<F>(ctx, n, aattrs);
}

// public static 
animate_node* animate_node::new_instance(context_type *ctx, const node *n, const node* tparent) {
	animate_attrs *aattrs = new animate_attrs(n, tparent);
	
	// Placeholder
	if(aattrs->get_animate_type() == "invalid")
		return new animate_node(ctx, n, aattrs);
	
	// Implemented animations
	if(aattrs->get_target_attr_type() == "reg_dim") {
		return new_regdim_animation(ctx, n, aattrs);
	} else if(aattrs->get_target_attr_type() == "color") {
		return new_color_animation(ctx, n, aattrs);
	} else if(aattrs->get_target_attr() == "z-index") {
		return new_zindex_animation(ctx, n, aattrs);
	} else if(aattrs->get_target_attr() == "position") {
		return new_position_animation(ctx, n, aattrs);
	}
	
	// Not implemented
	return new animate_node(ctx, n, aattrs);
}




