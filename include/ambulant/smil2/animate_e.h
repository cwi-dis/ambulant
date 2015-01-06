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

#ifndef AMBULANT_SMIL2_ANIMATE_E_H
#define AMBULANT_SMIL2_ANIMATE_E_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/mtsync.h"

#include <string>
#include <map>
#include <list>

namespace ambulant {

namespace lib {
	class event;
	class event_processor;
}

namespace common {
	class animation_destination;
}

namespace smil2 {

class smil_layout_manager;
class animate_node;

// a class responsible to apply the animation effect
class animation_engine {
  public:
	animation_engine(lib::event_processor* evp, smil_layout_manager *layout);
	~animation_engine();

	void started(animate_node *anode);
	void stopped(animate_node *anode);

	void reset();

  private:
	// Animators for a generalized attribute
	// animations are inserted in activation/doc order
	typedef std::list<animate_node*> attribute_animators_t;

	// Animators for a node
	typedef std::map<std::string, attribute_animators_t> node_animators_t;

	// All active animators
	typedef std::map<const lib::node*, node_animators_t> doc_animators_t;

	doc_animators_t m_animators;

	void _update();
	void _update_node(const lib::node *target, node_animators_t& animators);
	void _update_attr(const std::string& attr, attribute_animators_t& animators,
		common::animation_destination *dst);
	void _stopped(animate_node *anode);

	lib::event_processor *m_event_processor;
	smil_layout_manager *m_layout;
	bool m_is_node_dirty;

	int m_counter;
	bool _has_animations() const { return m_counter>0;}

	void update_callback();
	void _schedule_update();
    void _ensure_schedule_update();
	lib::critical_section m_lock;
	lib::event *m_update_event;
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_E_H
