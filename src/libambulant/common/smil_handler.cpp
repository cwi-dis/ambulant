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

#include "ambulant/common/smil_handler.h"

#include <string>
#include <map>

namespace ambulant {

namespace lib {

void smil_handler::start_smil(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_smil(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_head(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_head(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_body(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_body(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_layout(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_layout(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_root_layout(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_root_layout(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_top_layout(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_top_layout(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_region(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_region(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_transition(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_transition(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_par(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_par(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_seq(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_seq(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_switch(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_switch(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_excl(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_excl(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_priority_class(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_priority_class(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_ref(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_ref(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_reg_point(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_reg_point(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_prefetch(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_prefetch(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_a(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_a(const q_name_pair& qn) {
	default_end_element(qn);
}

void smil_handler::start_area(const q_name_pair& qn, const q_attributes_list& qattrs) {
	default_start_element(qn, qattrs);
}

void smil_handler::end_area(const q_name_pair& qn) {
	default_end_element(qn);
}


/////////////////////////////////
// default handlers

void smil_handler::default_start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = new node(qn, qattrs);
	} else if(m_current != 0) {
		node *p = new node(qn, qattrs);
		m_current->append_child(p);
		m_current = p;
	}
}

void smil_handler::default_end_element(const q_name_pair& qn) {
	if(m_current != 0)
		m_current = m_current->up();
}

void smil_handler::unknown_start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = new node(qn, qattrs);
	} else if(m_current != 0) {
		node *p = new node(qn, qattrs);
		m_current->append_child(p);
		m_current = p;
	}
}

void smil_handler::unknown_end_element(const q_name_pair& qn) {
	if(m_current != 0)
		m_current = m_current->up();
}


} // namespace lib
 
} // namespace ambulant
