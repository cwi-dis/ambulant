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

#ifndef AMBULANT_COMMON_SCHEMA_H
#define AMBULANT_COMMON_SCHEMA_H

#include "ambulant/config/config.h"

#include <set>

#include "ambulant/lib/sax_types.h"

// A class encapsulating the XML Schema used by the application.
// Components that want to be isolated from the exact XML Schema
// may query an instance of this class for the properties they 
// are interested for.
//
// An implementation of this class may use external resources, 
// hard coded info, or the XML Schema file itself,
// to build the data structures it requires. 

namespace ambulant {

namespace common {

// An enumeration representing the time container type
// range of values : par | seq | excl | none  
enum time_container_type {tc_par, tc_seq, tc_excl, tc_none};
const char* time_container_type_as_str(time_container_type t);

// An enumeration representing layout types
enum layout_type {l_layout, l_rootlayout, l_toplayout, l_region, l_regpoint, l_media, l_none};
const char* layout_type_as_str(layout_type t);

class schema {
  public:
	// Returns the schema instance used.
	static const schema* get_instance();
	
	// Returns a ref to the set of time elements.
	// Currently local names.
	const std::set<std::string>& get_time_elements() const {
		return m_time_elements;
	}
	
	// Returns the time container type of the element with QName.
	// A type is one of : par | seq | excl | none  
	time_container_type get_time_type(const lib::q_name_pair& qname) const;
	
	bool is_discrete(const lib::q_name_pair& qname) const;
	
	// Returns a ref to the set of layout elements.
	// Currently local names.
	const std::set<std::string>& get_layout_elements() const {
		return m_layout_elements;
	}
	
	// Returns the time container type of the element with QName.
	// A type is one of : par | seq | excl | none  
	layout_type get_layout_type(const lib::q_name_pair& qname) const;
	
	// Allow schema_factory classes to create instances.
	friend class schema_factory;
	
	bool is_animation(const lib::q_name_pair& qname) const;
	
 	schema();
 	~schema();
	
 private: 	
	std::set<std::string> m_time_elements;
	std::set<std::string> m_discrete;
	std::set<std::string> m_continuous;
	std::set<std::string> m_animations;
	std::set<std::string> m_layout_elements;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_SCHEMA_H
