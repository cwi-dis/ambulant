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

#include "ambulant/smil2/animate_e.h"
#include "ambulant/smil2/animate_n.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/region_info.h"

#include "ambulant/lib/logger.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

animation_engine::animation_engine(lib::event_processor* evp, smil_layout_manager *layout)
:	m_event_processor(evp),
	m_layout(layout),
	m_counter(0),
	m_update_event(0) {
}

animation_engine::~animation_engine() {
}

void animation_engine::reset() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("animation_engine::reset()");
	doc_animators_t::iterator it;
	for(it = m_animators.begin();it != m_animators.end();it++) {
		node_animators_t& nani = (*it).second;
		node_animators_t::iterator nit;
		for(nit = nani.begin();nit != nani.end();nit++)	 {
			attribute_animators_t& aani = (*nit).second;
			while (!aani.empty()) {
				animate_node *animator = *aani.begin();
				_stopped(animator);
			}
		}
	}
	m_update_event = NULL;
	m_lock.leave();
}

// Register the provided animator as active
// XXX: The animator may animate multiple attrs as for example in animateMotion
void animation_engine::started(animate_node *animator) {
	const lib::node *target = animator->get_animation_target();
	if(!target) return;
	m_lock.enter();
	node_animators_t& na = m_animators[target];

	attribute_animators_t& aa = na[animator->get_animation_attr()];
	aa.push_back(animator);

	m_counter++;
    _ensure_schedule_update();

	AM_DBG {
		lib::logger::get_logger()->debug("animation_engine: %s started targeting %s attr=%s (%d animations active)",
			animator->dom_node()->get_sig().c_str(),
			target->get_sig().c_str(),
			animator->get_animation_attr().c_str(), m_counter);
	}
	m_lock.leave();
}

// Remove animator from the active animations
void animation_engine::stopped(animate_node *animator) {
	m_lock.enter();
	_stopped(animator);
	m_lock.leave();
}

// Remove animator from the active animations
void animation_engine::_stopped(animate_node *animator) {
	const lib::node *target = animator->get_animation_target();
	if(!target) return;
	const std::string aattr = animator->get_animation_attr();
	AM_DBG lib::logger::get_logger()->debug("animation_engine: %s stopped targeting %s attr=%s", animator->dom_node()->get_sig().c_str(), target->get_sig().c_str(), aattr.c_str());
	node_animators_t& na = m_animators[target];
	attribute_animators_t& aa = na[aattr];
    int old_size = aa.size();
	aa.remove(animator);
    if (aa.size() == old_size) {
        AM_DBG lib::logger::get_logger()->debug("animation_engine::stopped(%s): not in animations for %s attr=%s",animator->dom_node()->get_sig().c_str(), target->get_sig().c_str(), aattr.c_str());
        return;
    }
	m_counter--;
	AM_DBG lib::logger::get_logger()->debug("animation_engine: %d active animations left", m_counter);
	if(aa.empty()) {
		common::animation_destination *dst = m_layout->get_animation_destination(target);
		if (dst == NULL) {
			lib::logger::get_logger()->trace("Implementation error: animation_destination NULL for %s", target->get_sig().c_str());
			return;
		}
		animate_registers regs;
		animator->read_dom_value(dst, regs);
		m_is_node_dirty = animator->set_animated_value(dst, regs);
		if(m_is_node_dirty)	 {
			common::animation_notification *anotif = m_layout->get_animation_notification(target);
			if(anotif) anotif->animated();
		}
	}
}

// Evaluate all active animations
void animation_engine::_update() {
	AM_DBG lib::logger::get_logger()->debug("Updating animators");
	doc_animators_t::iterator it;
	for(it = m_animators.begin();it != m_animators.end();it++)
		_update_node((*it).first, (*it).second);
}

// Evaluate all node animations and then apply them
void animation_engine::_update_node(const node *target, node_animators_t& animators) {
	m_is_node_dirty = false;
	common::animation_destination *dst = m_layout->get_animation_destination(target);
	if (dst == NULL) {
		lib::logger::get_logger()->trace("Implementation error: animation_destination NULL for %s", target->get_sig().c_str());
		return;
	}
	node_animators_t::iterator it;
	for(it = animators.begin();it != animators.end();it++)
		_update_attr((*it).first, (*it).second, dst);
	if(m_is_node_dirty) {
		common::animation_notification *anotif = m_layout->get_animation_notification(target);
		if(anotif) anotif->animated();
	}
}

// Evaluate attribute animations taking into account additivity
void animation_engine::_update_attr(const std::string& attr, attribute_animators_t& animators,
	common::animation_destination *dst) {
	if(animators.empty()) return;
	// get the dom value
	// apply animations in list order: set or add to value
	// all animations are associated with the same type
	animate_node *animator = animators.front();
	animate_registers regs;
	animator->read_dom_value(dst, regs);
	attribute_animators_t::iterator it;
	for(it = animators.begin();it != animators.end();it++)
		(*it)->apply_self_effect(regs);
	m_is_node_dirty = animator->set_animated_value(dst, regs);

	// XXX: Until the layout or the protocol with the layout is fixed
	// return always true e.g. always dirty
	m_is_node_dirty = true;
}

void animation_engine::update_callback() {
	m_lock.enter();
	if(!m_update_event) {
		lib::logger::get_logger()->debug("animation_engine: update_callback() called with m_update_event == NULL");
		m_lock.leave();
		return;
	}
	if(_has_animations()) {
		_update();
		_schedule_update();
	} else {
		AM_DBG lib::logger::get_logger()->debug("animation_engine: stop scheduling update");
		m_update_event = 0;
	}
	m_lock.leave();
}

void animation_engine::_schedule_update() {
	AM_DBG lib::logger::get_logger()->debug("animation_engine: schedule update");
	m_update_event = new lib::no_arg_callback_event<animation_engine>(this,
		&animation_engine::update_callback);
	m_event_processor->add_event(m_update_event, 50, lib::ep_med);
}

void animation_engine::_ensure_schedule_update() {
	if(m_update_event == 0) _schedule_update();
}



