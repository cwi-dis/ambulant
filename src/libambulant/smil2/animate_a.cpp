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

#include "ambulant/smil2/animate_a.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/lib/logger.h"

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;


animate_attrs::animate_attrs(const lib::node *n, const lib::node* tparent)
:	m_node(n),
	m_tparent(tparent), 
	m_target(0) {
	m_tag = m_node->get_local_name();
	locate_target_element();
	locate_target_attr();
	m_animtype = find_anim_type();
	read_enum_atttrs();
	AM_DBG lib::logger::get_logger()->trace("%s.%s --> %s.%s", 
		m_tag.c_str(), m_animtype.c_str(), m_target_type.c_str(), m_attrname.c_str());
	
}

animate_attrs::~animate_attrs() {
}

void animate_attrs::locate_target_element() {
	const char *p = m_node->get_attribute("targetElement");
	if(p) {
		m_target = m_node->get_context()->get_node(p);
		if(!m_target) {
			lib::logger::get_logger()->error("Failed to locate \'%s\' target node with id=\'%s\'", m_tag.c_str(), p);
			return;
		}
	} else {
		m_target = m_tparent;
	}
	if(m_target) {
		std::string ttag = m_target->get_local_name();
		if(ttag == "region") m_target_type = "region";
		else if(ttag == "area") m_target_type = "area";
		else m_target_type = "subregion";
	} else {
		lib::logger::get_logger()->error("%s --> ?", m_tag.c_str());
	}
}

void animate_attrs::locate_target_attr() {
	if(m_tag == "animateMotion") {
		m_attrname = "position";
		m_attrtype = "point";
		return;
	}
	const char *p = m_node->get_attribute("attributeName");
	if(!p) {
		lib::logger::get_logger()->error("Missing mantatory attribute \'attributeName\' for %s", m_tag.c_str());
		return;
	}
	m_attrname = p;
	static char *reg_dim_names[] = {"left", "top", "width", "height", "right", "bottom"};
	static int n = sizeof(reg_dim_names)/sizeof(const char *);
	for(int i=0;i<n;i++) {
		if(m_attrname == reg_dim_names[i]) {
			m_attrtype = "reg_dim";
			break;
		}
	}
	if(!m_attrtype.empty()) return;
	if(m_attrname == "backgroundColor") {
		m_attrtype = "color";
	} else if(m_attrname == "z-index") {
		m_attrtype = "int";
	} else if(m_attrname == "soundLevel") {
		m_attrtype = "positive_double";
	} else if(m_attrname == "coords") {
		m_attrtype = "string";
	} else if(m_attrname == "color") {
		m_attrtype = "color";
	} else {
		lib::logger::get_logger()->error("Not animateable attribute '%s' for '%s'", 
			m_attrname.c_str(), m_tag.c_str());
	}
}

// Returns one of: invalid, values, from-to, from-by, to, by
const char* animate_attrs::find_anim_type() {
	const char *pvalues = m_node->get_attribute("values");
	const char *ppath = m_node->get_attribute("path");
	if(pvalues || ppath) return "values";
	const char *pfrom = m_node->get_attribute("from");
	const char *pto = m_node->get_attribute("to");
	const char *pby = m_node->get_attribute("by");
	if(!pto && !pby) return "invalid";
	if(pfrom) {
		if(pto) return "from-to";
		else if(pby) return "from-by";
	} else {
		if(pto) return "to";
		else if(pby) return "by";
	}
	return "invalid";
}

void animate_attrs::read_enum_atttrs() {
	const char *p = m_node->get_attribute("additive");
	if(p && strcmp(p, "sum") == 0 /* && attr additive */) m_additive = true;
	else if(m_animtype == "by" /* && attr additive */) m_additive = true;
	else m_additive = false;
	
	p = m_node->get_attribute("accumulate");
	if(p && strcmp(p, "sum") == 0 /* && attr additive */) m_accumulate = true;
	else m_accumulate = false;
	
	p = m_node->get_attribute("calcMode");
	if(p) m_calc_mode = p;
	else if(m_tag == "animateMotion") m_calc_mode = "paced";
	else m_calc_mode = "linear";
}

// placeholder
inline int safeatoi(const char *p) {
	if(!p) return 0;
	return atoi(p);
}

void animate_attrs::get_values(std::vector<int>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(safeatoi((*it).c_str()));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pfrom));
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		int v1 = safeatoi(pfrom);
		int dv = safeatoi(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(0);
		v.push_back(safeatoi(pby));
	}
}

// XXX: placeholder, the values inserted are parced as integers 
void animate_attrs::get_values(std::vector<common::region_dim>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(safeatoi((*it).c_str()));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pfrom));
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		int v1 = safeatoi(pfrom);
		int dv = safeatoi(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(0);
		v.push_back(safeatoi(pby));
	}
}



