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
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/region_info.h"

#include "ambulant/lib/logger.h"

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

animation_engine::animation_engine(lib::event_processor* evp, smil_layout_manager *layout) 
:	m_event_processor(evp), 
	m_layout(layout) {
}

animation_engine::~animation_engine() {
}

void animation_engine::started(animate_node *anode) {
	const lib::node *target = anode->get_animation_target();
	if(!target) return;
	node_animators_t& na = m_animators[target];
	attribute_animators_t& aa = na[anode->get_animation_attr()];
	aa.push_back(anode);
	m_counter++;
	if(m_update_event == 0) schedule_update();
}

void animation_engine::stopped(animate_node *anode) {
	const lib::node *target = anode->get_animation_target();
	if(!target) return;
	const std::string aattr = anode->get_animation_attr();
	node_animators_t& na = m_animators[target];
	attribute_animators_t& aa = na[aattr];
	aa.remove(anode);
	m_counter--;
	
	// XXX: reset node attr if needed
	// XXX: the interface should allow attr reset not node reset
	if(aa.empty()) {
		common::animation_destination *dst = m_layout->get_animation_destination(target);
		dst->reset(); // this should specify what
		common::animation_notification *anotif = m_layout->get_animation_notification(target);
		if(anotif) anotif->animated();
	}
}

void animation_engine::update() {
	doc_animators_t::iterator it;
	for(it = m_animators.begin();it != m_animators.end();it++) 
		update_node((*it).first, (*it).second);
}

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

void animation_engine::update_attr(const std::string& attr, attribute_animators_t& animators, 
	common::animation_destination *dst) {
	attribute_animators_t::iterator it;
	for(it = animators.begin();it != animators.end();it++) {
		if((*it)->apply_value(dst)) m_is_node_dirty = true;
	}
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




