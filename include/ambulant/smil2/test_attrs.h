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

#ifndef AMBULANT_SMIL2_TEST_ATTRS_H
#define AMBULANT_SMIL2_TEST_ATTRS_H

#include "ambulant/config/config.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/state.h"

#include <string>
#include <map>

namespace ambulant {

namespace lib {
class logger;
class document;
} // namespace lib

namespace smil2 {

using lib::custom_test;
class state_test_methods_impl;

// Macro for creating an Ambulant systemComponent URI
#define AM_SYSTEM_COMPONENT(s) "http://www.ambulantplayer.org/component/" s

class AMBULANTAPI test_attrs {
  public:
	test_attrs(const lib::node *n);

	// Returns true when the target node is selected.
	bool selected() const;

	static bool load_test_attrs(const std::string& filename);
	static void set_default_tests_attrs();

	const std::string& get_tag() const { return m_tag;}
	const std::string& get_id() const { return m_id;}

	static common::state_test_methods *get_state_test_methods();

	// API for embedders and extenders that want to fiddle with components and
	// custom tests
	/// Get a (boolean-valued) custom test attribute value by name
	static bool get_current_custom_test_value(std::string name);
	/// Set the value for a custom test attribute by name
	static void set_current_custom_test_value(std::string name, bool value);
	/// Get a (boolean-valued) systemComponent value by name
	static bool get_current_system_component_value(std::string name);
	/// Set the value for a systemComponent by name
	static void set_current_system_component_value(std::string name, bool enabled);
	/// Set the current screen size
	static void set_current_screen_size(int height, int width);
	/// Clear the list of user-preferred languages
	static void clear_languages();
	/// Add a language, with weight, to the list of user-preferred languages
	static void add_language(std::string langname, float weight);
	/// Return a preference factor [0.0,1.0] for the given language
	static float get_system_language_weight(std::string lang);

  protected:
	typedef std::string::size_type size_type;
	static bool test_on_off_attr(const std::string& attr,const char *value);
	static bool test_exact_str_attr(const std::string& attr,const char *value);
	static bool test_exact_str_list_attr(const std::string& attr,const char *value);

	static bool test_system_language(const char *lang);
	static bool test_system_component(const char *value);
	static bool test_system_bitrate(const char *value);
	static bool test_system_screen_depth(const char *value);
	static bool test_system_screen_size(const char *value);
	static bool test_system_required(const char *value, const lib::node_context *ctx);
	bool test_custom_attribute(const char *value) const;

	// the target node
	const lib::node *m_node;
	const std::map<std::string, custom_test>* m_custom_tests;


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	// tracing
	std::string m_id;
	std::string m_tag;

#ifdef _MSC_VER
#pragma warning(pop)
#endif


	lib::logger *m_logger;
	friend class state_test_methods_impl;
};

} // namespace smil2

} // namespace ambulant

AMBULANTAPI bool load_test_attrs(const char *filename);

#endif // AMBULANT_SMIL2_TEST_ATTRS_H
