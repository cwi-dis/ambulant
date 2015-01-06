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

#ifndef AMBULANT_COMMON_SCHEMA_H
#define AMBULANT_COMMON_SCHEMA_H

#include "ambulant/config/config.h"

#include <set>

#include "ambulant/lib/sax_types.h"

namespace ambulant {

namespace common {

/// An enumeration representing the time container type.
/// range of values : par | seq | excl | none
enum time_container_type {tc_par, tc_seq, tc_excl, tc_none};

/// Return the string representing a type_container_type.
const char* time_container_type_as_str(time_container_type t);

/// An enumeration representing layout node types.
enum layout_type {l_layout, l_rootlayout, l_toplayout, l_region, l_regpoint, l_media, l_none};

/// A class encapsulating the XML Schema used by the application.
/// Components that want to be isolated from the exact XML Schema
/// may query an instance of this class for the properties they
/// are interested for.
///
/// An implementation of this class may use external resources,
/// hard coded info, or the XML Schema file itself,
/// to build the data structures it requires.
///
/// The current implementation is rather half-hearted and hardcoded.
class schema {
  public:
	/// Returns the schema instance used.
	static const schema* get_instance();

	/// Returns a ref to the set of time elements.
	/// Currently local names.
	const std::set<std::string>& get_time_elements() const {
		return m_time_elements;
	}

	/// Returns the time_container_type of the element with tag qname.
	time_container_type get_time_type(const lib::xml_string& qname) const;

	/// Returns true if this item is discrete (as opposed to continuous). Obsolete.
	bool is_discrete(const lib::xml_string& qname) const;

	/// Returns a ref to the set of layout elements.
	/// Currently local names.
	const std::set<std::string>& get_layout_elements() const {
		return m_layout_elements;
	}

	/// Returns the layout_type of the element with tag qname.
	layout_type get_layout_type(const lib::xml_string& qname) const;

	/// Allow schema_factory classes to create instances.
	friend class schema_factory;

	/// Return true if an element with tag qname is an animation.
	bool is_animation(const lib::xml_string& qname) const;

	/// Return true if an element with tag qname is a state command.
	bool is_statecommand(const lib::xml_string& qname) const;

	/// Return true if an element with tag qname is a prefetch.
	bool is_prefetch(const lib::xml_string& qname) const;

	schema();
	~schema();

  private:
	std::set<std::string> m_time_elements;
	std::set<std::string> m_discrete;
	std::set<std::string> m_continuous;
	std::set<std::string> m_animations;
	std::set<std::string> m_layout_elements;
	std::set<std::string> m_statecommands;
	std::set<std::string> m_prefetch;
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_SCHEMA_H
