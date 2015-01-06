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
	m_aattrs(aattrs)
{
	get_values();
}

animate_node::~animate_node() {
	delete m_aattrs;
}

void animate_node::get_values() {
}

void animate_node::prepare_interval() {
	get_values();
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
	void get_values();
  protected:
	bool verify_key_times(std::vector<double>& keyTimes);

	F m_simple_f;
	animate_f<F> *m_animate_f;
	std::vector<T> m_values;

};

template <class F, class T>
linear_values_animation<F, T>::linear_values_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs), m_animate_f(0)  {
}

template <class F, class T>
linear_values_animation<F, T>::~linear_values_animation() {
	delete m_animate_f;
}

template <class F, class T>
void linear_values_animation<F, T>::get_values()
{
	m_aattrs->get_values(m_values);
}

template <class F, class T>
void linear_values_animation<F, T>::prepare_interval() {
    m_values.clear();
	get_values();
	time_type dur = calc_dur();
	time_type sfdur = dur;
	const time_attrs* ta = get_time_attrs();
	if(dur.is_definite() && ta->auto_reverse()) sfdur /= 2;
	if(m_aattrs->get_calc_mode() == "paced") {
		// ignore key times
		m_simple_f.paced_init(sfdur(), m_values);
	} else {
		// use key times when valid and when not ignorable
		std::vector<double> keyTimes;
		m_aattrs->get_key_times(keyTimes);
//gcc2.95 Internal compiler error in `assign_stack_temp_for_type `
//		bool keyTimesValid = keyTimes.empty() || verify_key_times(keyTimes);
		bool keyTimesValid = false;
		if(keyTimes.empty() || verify_key_times(keyTimes))
			keyTimesValid = true;
		if(!keyTimes.empty() && keyTimesValid &&
			(dur.is_definite() || m_aattrs->get_calc_mode() == "discrete")) {
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
		m_logger->trace("<%s id=\"%s\">: invalid key times", m_attrs.get_tag().c_str(), m_attrs.get_id().c_str());
	AM_DBG {
		if(!keyTimes.empty() && keyTimesValid) {
			std::string str;
			for(size_t i=0;i<keyTimes.size();i++) {
//gcc2.95 Internal compiler err.char sz[32];
				char sz[64];
				sprintf(sz,"%.3f;", keyTimes[i]);str += sz;
			}
			m_logger->debug("%s[%s] keyTimes: %s",
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
	void get_values();
  protected:
	typedef underlying_to_f<T> F;
	F m_simple_f;
	animate_f<F> *m_animate_f;
	T m_value;
};

template <class T>
underlying_to_animation<T>::underlying_to_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs), m_animate_f(0)  {
}

template <class T>
underlying_to_animation<T>::~underlying_to_animation() {
	delete m_animate_f;
}

template <class T>
void underlying_to_animation<T>::get_values()
{
	std::vector<T> v;
	m_aattrs->get_values(v);
	m_value = v[0];
}

template <class T>
void underlying_to_animation<T>::prepare_interval() {
	get_values();
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
	regdim_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, common::region_dim>(ctx, n, aattrs)
	{}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.rd = dst->get_region_dim(this->m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::region_dim rd = dst->get_region_dim(this->m_aattrs->get_target_attr(), false);
		if(rd != regs.rd || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = this->m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> %s",
					this->m_aattrs->get_target_attr().c_str(), t, ::repr(regs.rd).c_str());
			}
			dst->set_region_dim(this->m_aattrs->get_target_attr(), regs.rd);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		common::region_dim rd = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
			regs.rd += rd; // add

		else
			regs.rd = rd; // override
        AM_DBG lib::logger::get_logger()->debug("regdim_animation: now at %s",regs.rd.repr().c_str());

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
				lib::logger::get_logger()->debug("%s(%ld) -> %s",
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
       AM_DBG lib::logger::get_logger()->debug("underlying_to_regdim_animation: (t=%d) now at %s", (int)t, regs.rd.repr().c_str());
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
	color_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, lib::color_t>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.cl = dst->get_region_color(this->m_aattrs->get_target_attr(), true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		lib::color_t cl = dst->get_region_color(this->m_aattrs->get_target_attr(), false);
		if(cl != regs.cl || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = this->m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> 0x%x",
					this->m_aattrs->get_target_attr().c_str(), t, regs.cl);
			}
			dst->set_region_color(this->m_aattrs->get_target_attr(), regs.cl);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		lib::color_t cl = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
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
				lib::logger::get_logger()->debug("%s(%ld) -> 0x%x",
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
	zindex_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, common::zindex_t>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.zi = dst->get_region_zindex(true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		common::zindex_t zi = dst->get_region_zindex(false);
		if(zi != regs.zi || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = this->m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> %d",
					this->m_aattrs->get_target_attr().c_str(), t, regs.zi);
			}
			dst->set_region_zindex(regs.zi);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		common::zindex_t zi = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
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
				lib::logger::get_logger()->debug("%s(%ld) -> %d",
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
	values_motion_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
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
				lib::timer::time_type t = this->m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> %s",
					this->m_aattrs->get_target_attr().c_str(), t, ::repr(regs.pt).c_str());
			}
			dst->set_region_dim("left", common::region_dim(regs.pt.x));
			dst->set_region_dim("top", common::region_dim(regs.pt.y));
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		lib::point pt = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
			regs.pt += pt; // add
		else
			regs.pt = pt; // override
        AM_DBG lib::logger::get_logger()->debug("animate_motion: now at (%d, %d)", regs.pt.x, regs.pt.y);
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
				lib::logger::get_logger()->debug("%s(%ld) -> %s",
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

class soundalign_animation : public animate_node {
  public:
	soundalign_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	~soundalign_animation();

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const;
	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const;
	void apply_self_effect(animate_registers& regs) const;
	void get_values();
  private:
	std::vector<common::sound_alignment> m_values;
};


soundalign_animation::soundalign_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
:	animate_node(ctx, n, aattrs)
{
	assert(m_values.size() == 1);
}

soundalign_animation::~soundalign_animation()
{
}

void
soundalign_animation::get_values()
{
	m_aattrs->get_values(m_values);
}

void
soundalign_animation::read_dom_value(common::animation_destination *dst, animate_registers& regs) const
{
	regs.sa = dst->get_region_soundalign(true);
}

bool
soundalign_animation::set_animated_value(common::animation_destination *dst, animate_registers& regs) const
{
	common::sound_alignment sa = dst->get_region_soundalign(false);
	if (sa != regs.sa) {
		dst->set_region_soundalign(regs.sa);
		return true;
	}
	return false;
}

void
soundalign_animation::apply_self_effect(animate_registers& regs) const
{
	regs.sa = m_values[0];
}

////////////////////////////////////
// regdim_animation
//
// A panzoom_animation may be used for all panZoom animations except for "to" animations
//
template <class F>
class panzoom_animation : public linear_values_animation<F, common::region_dim_spec> {
  public:
	panzoom_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, common::region_dim_spec>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.panzoom = dst->get_region_panzoom(true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		const common::region_dim_spec& rds = dst->get_region_panzoom(false);
		if(rds != regs.panzoom || IGNORE_ATTR_COMP) {
			dst->set_region_panzoom(regs.panzoom);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		common::region_dim_spec panzoom = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
			regs.panzoom += panzoom; // add

		else
			regs.panzoom = panzoom; // override
	}
};

class underlying_to_panzoom_animation : public underlying_to_animation<common::region_dim_spec> {
  public:
	underlying_to_panzoom_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<common::region_dim_spec>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.panzoom = dst->get_region_panzoom(true);
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		const common::region_dim_spec panzoom = dst->get_region_panzoom(false);
		if(panzoom != regs.panzoom || IGNORE_ATTR_COMP) {
			dst->set_region_panzoom(regs.panzoom);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) {
			AM_DBG lib::logger::get_logger()->debug("panzoom_anim: m_animate_f==NULL");
			return;
		}
		lib::timer::time_type t = m_timer->elapsed();
		common::region_dim_spec panzoom = m_animate_f->at(t, regs.panzoom);
		AM_DBG lib::logger::get_logger()->debug("panzoom_anim: timer=0x%x t=%d", (void*)m_timer, t);
		regs.panzoom = panzoom; // override
	}
};

////////////////////////////////////

template <class F>
class opacity_animation : public linear_values_animation<F, double> {
  public:
	opacity_animation(time_node_context *ctx, const node *n, animate_attrs *aattrs)
	:	linear_values_animation<F, double>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.dv = dst->get_region_opacity(this->m_aattrs->get_target_attr(), true)*100;
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		double dv = dst->get_region_opacity(this->m_aattrs->get_target_attr(), false)*100;
		if(dv != regs.dv || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = this->m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> %f",
					this->m_aattrs->get_target_attr().c_str(), t, regs.dv);
			}
			dst->set_region_opacity(this->m_aattrs->get_target_attr(), regs.dv/100);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!this->m_animate_f) return;
		lib::timer::time_type t = this->m_timer->elapsed();
		double dv = this->m_animate_f->at(t);
		if(this->m_aattrs->is_additive())
			regs.dv += dv; // add
		else
			regs.dv = dv; // override
	}
};

class underlying_to_opacity_animation : public underlying_to_animation<double> {
  public:
	underlying_to_opacity_animation(context_type *ctx, const node *n, animate_attrs *aattrs)
	:	underlying_to_animation<double>(ctx, n, aattrs) {}

	void read_dom_value(common::animation_destination *dst, animate_registers& regs) const {
		regs.dv = dst->get_region_opacity(m_aattrs->get_target_attr(), true)*100;
	}

	bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const {
		double dv = dst->get_region_opacity(m_aattrs->get_target_attr(), false)*100;
		if(dv != regs.dv || IGNORE_ATTR_COMP) {
			AM_DBG {
				lib::timer::time_type t = m_timer->elapsed();
				lib::logger::get_logger()->debug("%s(%ld) -> %f",
					m_aattrs->get_target_attr().c_str(), t, regs.dv);
			}
			dst->set_region_opacity(m_aattrs->get_target_attr(), regs.dv/100);
			return true;
		}
		return false;
	}

	void apply_self_effect(animate_registers& regs) const {
		if(!m_animate_f) return;
		lib::timer::time_type t = m_timer->elapsed();
		double dv = m_animate_f->at(t, regs.dv);
		regs.dv = dv; // override
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

animate_node* animate_node::new_soundalign_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef common::sound_alignment attr_t;
	assert(aattrs->is_discrete());
	typedef discrete_map_f<attr_t> F;
	return new soundalign_animation(ctx, n, aattrs);
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

// private static
animate_node* animate_node::new_panzoom_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef common::region_dim_spec attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new panzoom_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_panzoom_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new panzoom_animation<F>(ctx, n, aattrs);
}

// private static
animate_node* animate_node::new_opacity_animation(context_type *ctx, const node *n, animate_attrs *aattrs) {
	typedef double attr_t;
	if(aattrs->is_discrete()) {
		typedef discrete_map_f<attr_t> F;
		return new opacity_animation<F>(ctx, n, aattrs);
	} else if(aattrs->get_animate_type() == "to") {
		return new underlying_to_opacity_animation(ctx, n, aattrs);
	}
	typedef linear_map_f<attr_t> F;
	return new opacity_animation<F>(ctx, n, aattrs);
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
	} else if(aattrs->get_target_attr() == "soundAlign") {
		return new_soundalign_animation(ctx, n, aattrs);
	} else if (aattrs->get_target_attr() == "panZoom") {
		return new_panzoom_animation(ctx, n, aattrs);
	} else if (aattrs->get_target_attr_type() == "opacity" ) {
		return new_opacity_animation(ctx, n, aattrs);
	}

	// Not implemented
	return new animate_node(ctx, n, aattrs);
}
