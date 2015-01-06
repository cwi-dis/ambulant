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

#ifndef AMBULANT_LIB_SAX_TYPES_H
#define AMBULANT_LIB_SAX_TYPES_H

#include "ambulant/config/config.h"

#include <string>

// pair
#include <utility>

// list
#include <list>

# include <ostream>

namespace ambulant {

namespace lib {

/// The type of string used to represent XML data.
typedef std::basic_string<char> xml_string;

/// The type used to represent a tag or attribute name.
/// First is the namespace uri and second is the local name.
typedef std::pair<xml_string, xml_string> q_name_pair;

/// The type used to represent and attribute name/value pair.
/// First is the qualified name of the attribute and second
/// is the value of the attribute.
typedef std::pair<q_name_pair, xml_string> q_attribute_pair;

/// The type used to represent a list of attribute pairs.
typedef std::list<q_attribute_pair> q_attributes_list;

} // namespace lib

} // namespace ambulant

inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::q_name_pair& n) {
	os << n.second;
	return os;
}

#endif // AMBULANT_LIB_SAX_TYPES_H
