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

// Register the provided animator as active
// XXX: The animator may animate multiple attrs as for example in animateMotion
void animation_engine::started(animate_node *animator) {
	const lib::node *target = animator->get_animation_target();
	if(!target) return;
	node_animators_t& na = m_animators[target];
	
	attribute_animators_t& aa = na[animator->get_animation_attr()];
	aa.push_back(animator);
	
	m_counter++;
	if(m_update_event == 0) schedule_update();
	
	AM_DBG {
		const time_attrs* ata = animator->get_time_attrs();
		lib::logger::get_logger()->debug("%s[%s] started targeting %s.%s", 
			ata->get_tag().c_str(), ata->get_id().c_str(), 
			 target->get_local_name().c_str(), 
			animator->get_animation_attr().c_str());
	}
}

// Remove animator from the active animations
void animation_engine::stopped(animate_node *animator) {
	const lib::node *target = animator->get_animation_target();
	if(!target) return;
	const std::string aattr = animator->get_animation_attr();
	node_animators_t& na = m_animators[target];
	attribute_animators_t& aa = na[aattr];
	aa.remove(animator);
	m_counter--;
	if(aa.empty()) {
		common::animation_destination *dst = m_layout->get_animation_destination(target);
		animate_registers regs;
		animator->read_dom_value(dst, regs);
		m_is_node_dirty = animator->set_animated_value(dst, regs);
		if(m_is_node_dirty)  {
			common::animation_notification *anotif = m_layout->get_animation_notification(target);
			if(anotif) anotif->animated();
		}
	}
}

// Evaluate all active animations
void animation_engine::update() {
	AM_DBG lib::logger::get_logger()->debug("Updating animators");
	doc_animators_t::iterator it;
	for(it = m_animators.begin();it != m_animators.end();it++) 
		update_node((*it).first, (*it).second);
}

// Evaluate all node animations and then apply them
void animation_engine::update_node(const node *target, node_animators_t& animators) {
	m_is_node_dirty = false;
	common::animation_destination *dst = m_layout->get_animation_destination(target);
	node_animators_t::iterator it;
	for(it = animators.begin();it != animators.end();it++) 
		update_attr((*it).first, (*it).second, dst);
	if(m_is_node_dirty) {
		common::animation_notification *anotif = m_layout->get_animation_notification(target);
		if(anotif) anotif->animated();
	} 
}
	
// Evaluate attribute animations taking into account additivity
void animation_engine::update_attr(const std::string& attr, attribute_animators_t& animators, 
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
	if(!m_update_event) return;
	if(has_animations()) {
		update();
		schedule_update();
	} else {
		m_update_event = 0;
	}
}

void animation_engine::schedule_update() {
	m_update_event = new lib::no_arg_callback_event<animation_engine>(this, 
		&animation_engine::update_callback);
	m_event_processor->add_event(m_update_event, 100);
}




