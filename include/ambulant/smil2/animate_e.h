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

#ifndef AMBULANT_SMIL2_ANIMATE_E_H
#define AMBULANT_SMIL2_ANIMATE_E_H

#include "ambulant/config/config.h"

#include <string>
#include <map>
#include <list>

namespace ambulant {

namespace lib {
	class node;
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
	
  private:
	// Animators for a generalized attribute
	// animations are inserted in activation/doc order
	typedef std::list<animate_node*> attribute_animators_t;
	
	// Animators for a node
	typedef std::map<std::string, attribute_animators_t> node_animators_t;
	
	// All active animators
	typedef std::map<const lib::node*, node_animators_t> doc_animators_t;
	
	doc_animators_t m_animators;
	
	void update();
	void update_node(const lib::node *target, node_animators_t& animators);
	void update_attr(const std::string& attr, attribute_animators_t& animators, 
		common::animation_destination *dst);
	
	lib::event_processor *m_event_processor;
	smil_layout_manager *m_layout;
	bool m_is_node_dirty;
	
	int m_counter;
	bool has_animations() const { return m_counter>0;}
	
	void update_callback();
	void schedule_update();	
	lib::event *m_update_event;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_E_H
