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

#ifndef AMBULANT_SMIL2_ANIMATE_A_H
#define AMBULANT_SMIL2_ANIMATE_A_H

#include "ambulant/config/config.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/region_info.h"

#include <string>
#include <vector>

namespace ambulant {

namespace lib {
	class logger;
}

namespace common {
	class region_dim;
}

namespace smil2 {

struct qtuple {double v[4];};

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
	bool is_accumulative() const { return m_accumulate;}
	const std::string& get_calc_mode() const { return m_calc_mode;}

	// Return true when
	// a) calcMode is discrete
	// b) the attribute is not linear (emum or strings)
	// c) for set animations
	bool is_discrete() const;

	void get_key_times(std::vector<double>& v);
	void get_key_splines(std::vector<qtuple>& v);

	void get_values(std::vector<int>& v);
	void get_values(std::vector<double>& v);
	void get_values(std::vector<std::string>& v);
	void get_values(std::vector<common::region_dim>& v);
	void get_values(std::vector<lib::color_t>& v);
	void get_values(std::vector<lib::point>& v);
	void get_values(std::vector<common::sound_alignment>& v);
	void get_values(std::vector<common::region_dim_spec>& v);

  private:

	void locate_target_element();
	void locate_target_attr();
	const char* find_anim_type();
	void read_enum_atttrs();
	void apply_constraints();

	int safeatoi(const char *p);
	double safeatof(const char *p);
	common::region_dim to_region_dim(const std::string& s);
	lib::point to_point(const std::string& s);
	common::region_dim_spec to_rds(const std::string& s);

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

	lib::logger *m_logger;
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_A_H
