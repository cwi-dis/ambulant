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

#ifndef AMBULANT_SMIL2_ANIMATE_A_H
#define AMBULANT_SMIL2_ANIMATE_A_H

#include "ambulant/config/config.h"

#include <string>
#include <vector>

namespace ambulant {

namespace lib {
	class node;
}

namespace common {
	class region_dim;
}

namespace smil2 {

class animate_attrs {
  public:	
	animate_attrs(const lib::node *n, const lib::node* tparent);
	~animate_attrs();
	const std::string& get_tag() const { return m_tag;}
	const std::string& get_id() const { return m_id;}
	const lib::node *get_target() const { return m_target;}
	const std::string& get_target_type() const { return m_target_type;}
	const std::string& get_target_attr() const { return m_attrname;}
	const std::string& get_target_attr_type() const { return m_attrtype;}
	const std::string& get_animate_type() const { return m_animtype;}
	bool is_additive() const { return m_additive;}
	bool is_accumulate() const { return m_accumulate;}
	const std::string& get_calc_mode() const { return m_calc_mode;}
	
	void get_values(std::vector<int>& v);
	void get_values(std::vector<common::region_dim>& v);
	
  private:
	
	void locate_target_element();
	void locate_target_attr();
	const char* find_anim_type();
	void read_enum_atttrs();
	
	const lib::node *m_node;
	const lib::node *m_tparent;
	std::string m_tag;
	std::string m_id;
	
	const lib::node *m_target;
	std::string m_target_type;
	std::string m_attrname;
	std::string m_attrtype;
	std::string m_animtype;
	
	bool m_additive;
	bool m_accumulate;
	std::string m_calc_mode;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_A_H
