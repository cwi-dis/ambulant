/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
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
