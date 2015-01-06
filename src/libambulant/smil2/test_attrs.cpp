// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/config/config.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// The currently active tests attributes.
// Some values as set by default
// The map is updated when the user selects a new filter file
static
std::map<std::string, std::string> active_tests_attrs_map;
bool active_test_attrs_map_inited;

// The currently active custom tests attributes.
// The map is filled when the user selects a new filter file
static
std::map<std::string, bool> active_custom_tests_attrs_map;

// The current set of numeric language preferences
static
std::map<std::string, float> active_language_map;

inline std::string get_test_attribute(const std::string& attr) {
	std::map<std::string, std::string>::iterator it = active_tests_attrs_map.find(attr);
	return (it != active_tests_attrs_map.end())?(*it).second:"";
}

// Create a tests helper for the provided node and for the document custom tests
test_attrs::test_attrs(const lib::node *n)
:	m_node(n),
	m_custom_tests(n->get_context()->get_custom_tests()) {
	m_logger = lib::logger::get_logger();

	if(active_tests_attrs_map.empty())
		set_default_tests_attrs();

	// debug statements
	const char *pid = m_node->get_attribute("id");
	m_id = (pid?pid:"no-id");
	m_tag = m_node->get_local_name();
}

// Returns true when this node should be included
bool test_attrs::selected() const {
	const char *value = 0;

	// systemLanguage
	value = m_node->get_attribute("systemLanguage");
	if (!value)
		value = m_node->get_attribute("system-language");
	if(value && !test_system_language(value))
		return false;

	// systemAudioDesc
	value = m_node->get_attribute("systemAudioDesc");
	if(value && !test_on_off_attr("systemAudioDesc", value))
		return false;

	// systemBitrate
	value = m_node->get_attribute("systemBitrate");
	if (!value)
		value = m_node->get_attribute("system-bitrate");
	if(value && !test_system_bitrate(value))
		return false;

	// systemCaptions
	value = m_node->get_attribute("systemCaptions");
	if (!value)
		value = m_node->get_attribute("system-captions");
	if(value && !test_on_off_attr("systemCaptions", value))
		return false;

	// systemCPU
	value = m_node->get_attribute("systemCPU");
	if(value && !test_exact_str_attr("systemCPU", value))
		return false;

	// systemOperatingSystem
	value = m_node->get_attribute("systemOperatingSystem");
	if(value && !test_exact_str_attr("systemOperatingSystem", value))
		return false;

	// systemOverdubOrSubtitle
	value = m_node->get_attribute("systemOverdubOrSubtitle");
	if (!value)
		value = m_node->get_attribute("system-overdub-or-caption");
	if(value && !test_exact_str_attr("systemOverdubOrSubtitle", value))
		return false;

	// systemScreenDepth
	value = m_node->get_attribute("systemScreenDepth");
	if (!value)
		value = m_node->get_attribute("system-screen_depth");
	if(value && !test_system_screen_depth(value))
		return false;

	// systemScreenSize
	value = m_node->get_attribute("systemScreenSize");
	if (!value)
		value = m_node->get_attribute("system-screen-size");
	if(value && !test_system_screen_size(value))
		return false;

	// systemComponent
	value = m_node->get_attribute("systemComponent");
	if(value && !test_system_component(value))
		return false;

	// customTest
	value = m_node->get_attribute("customTest");
	if(value && !test_custom_attribute(value))
		return false;

	// systemRequired, rather different from the others
	value = m_node->get_attribute("systemRequired");
	if (!value)
		value = m_node->get_attribute("system-required");
	if(value && !test_system_required(value, m_node->get_context()))
		return false;

	return true;
}

// systemLanguage ::= (languageTag (S? ',' S? languageTag)*)?
// return true when any in the list stars with the argument
bool test_attrs::test_system_language(const char *value) {
	return get_system_language_weight(value) > 0;
}

bool test_attrs::test_system_component(const char *value) {
	std::string s = get_test_attribute("systemComponent");
	if(s.empty()) return false;
	std::list<std::string> list;
	lib::split_trim_list(s, list, ' ');
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		if((*it) == value) return true;
	}
	return false;
}

bool test_attrs::test_system_required(const char *value, const lib::node_context *ctx) {
	std::list<std::string> list;
	lib::split_trim_list(value, list, '+');
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		if(!ctx->is_supported_prefix((*it))) return false;
	}
	return true;
}

bool test_attrs::test_system_bitrate(const char *value) {
	std::string s = get_test_attribute("systemBitrate");
	if(s.empty()) return false;
	int sys_bitrate = atoi(s.c_str());
	int sel_value = atoi(value);
	return sys_bitrate >= sel_value;
}

bool test_attrs::test_on_off_attr(const std::string& attr,const char *value) {
	std::string s = get_test_attribute(attr);
	return (s.empty() || (s != "on" && s != "off"))?false:(s == value);
}

bool test_attrs::test_exact_str_attr(const std::string& attr,const char *value) {
	std::string s = get_test_attribute(attr);
	return s.empty()?false:(s == value);
}

bool test_attrs::test_exact_str_list_attr(const std::string& attr,const char *value) {
	std::string s = get_test_attribute(attr);
	if(s.empty()) return false;
	std::list<std::string> list;
	lib::split_trim_list(s, list, ' ');
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		if((*it) == value) return true;
	}
	return false;
}

// systemScreenDepth
bool test_attrs::test_system_screen_depth(const char *value) {
	std::string s = get_test_attribute("systemScreenDepth");
	if(s.empty()) return false;
	int sys_bpp = atoi(s.c_str());
	int sel_bpp = atoi(value);
	return sys_bpp >= sel_bpp;
}

bool test_attrs::test_system_screen_size(const char *value) {
	std::string s = get_test_attribute("systemScreenSize");
	if(s.empty()) return false;
	lib::tokens_vector sys_v(s.c_str(), "Xx");
	lib::tokens_vector sel_v(value, "Xx");
	if(sys_v.size() != 2 || sel_v.size() != 2)
		return false;
	return (atoi(sys_v[0].c_str())>atoi(sel_v[0].c_str())) &&
		(atoi(sys_v[1].c_str())>atoi(sel_v[1].c_str()));
}

bool test_attrs::test_custom_attribute(const char *value) const {
	std::string s = lib::trim(value);
	if(s.empty()) return true;
	std::list<std::string> list;
	lib::split_trim_list(s, list, ' ');
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		std::string id = lib::to_c_lower(*it);
		// What's the state of this attr as defined by the filter?
		std::map<std::string, bool>::const_iterator oit =
			active_custom_tests_attrs_map.find(id);
		if(!(oit == active_custom_tests_attrs_map.end())) {
			// The attr is defined in the filter
			// Can this be ovveriden
			std::map<std::string, custom_test>::const_iterator cit =
				m_custom_tests->find(id);
			if(cit == m_custom_tests->end()) {
				// Not specified in the doc, assume override is true
				// if the filter specifies false return false
				if(!(*oit).second) return false;
				else continue;
			} else {
				// Specified in the document
				if((*cit).second.override) {
					// override is true
					if(!(*oit).second) return false;
					else continue;
				} else {
					// override is false
					if(!(*cit).second.state) return false;
					else continue;
				}
			}
		} else {
			// not specified in the filter
			// is this specified in the doc?
			std::map<std::string, custom_test>::const_iterator cit =
				m_custom_tests->find(id);
			// if not specified in the document or specified but its defaultValue is false, return false
			if(cit == m_custom_tests->end() || !(*cit).second.state) return false;
			else continue;
		}
	}
	// all present and evaluated to true
	return true;
}

/////////////////////////
//

// static
bool test_attrs::load_test_attrs(const std::string& filename) {
	lib::tree_builder builder(lib::get_builtin_node_factory());
	if(!builder.build_tree_from_file(filename.c_str())) {
		lib::logger::get_logger()->error(gettext("While loading settings: %s: Could not create DOM tree"), filename.c_str());
		return false;
	}

	// Clear maps
	active_tests_attrs_map.clear();
    active_test_attrs_map_inited = false;
	active_custom_tests_attrs_map.clear();

	// load default first; some will be overriden below
	set_default_tests_attrs();

	const lib::node* root = builder.get_tree();
	lib::node::const_iterator it;
	lib::node::const_iterator end = root->end();
	for(it = root->begin(); it != end; it++) {
		if(!(*it).first) continue;
		const lib::node *n = (*it).second;
		const std::string& tag = n->get_local_name();
		if(tag == "systemTest" || tag == "property") {
			const char *name = n->get_attribute("name");
			const char *value = n->get_attribute("value");
			if(name && value) {
				active_tests_attrs_map[name] = value;
				AM_DBG lib::logger::get_logger()->debug("systemTest %s: %s", name, value);
				if (std::string(name) == "systemLanguage") {
					clear_languages();
					add_language(value, 1.0f);
				}
			}
		} else if(tag == "customTest") {
			const char *name = n->get_attribute("name");
			const char *value = n->get_attribute("value");
			std::string sn = lib::trim(name);
			std::string sv = lib::trim(value);
			if(!sn.empty() && !sv.empty()) {
				sn = lib::to_c_lower(sn);
				active_custom_tests_attrs_map[sn] = (sv == "true")?true:false;
				AM_DBG lib::logger::get_logger()->debug("customTest %s: %s",
					sn.c_str(), (sv == "true")?"true":"false");
			}
		}
	}
	return true;
}

bool load_test_attrs(const char *filename) {
	return test_attrs::load_test_attrs(filename);
}

// static
void test_attrs::set_default_tests_attrs() {
    if (active_test_attrs_map_inited) return;
    active_test_attrs_map_inited = true;

	active_tests_attrs_map["systemAudioDesc"] = "on";
	active_tests_attrs_map["systemBitrate"] = "56000";
	active_tests_attrs_map["systemCaptions"] = "on";
	active_tests_attrs_map["systemOverdubOrSubtitle"] = "overdub";
#if defined(__i386__) || defined(_M_IX86)
	active_tests_attrs_map["systemCPU"] = "x86";
#elif defined(__x86_64__) || defined(_M_IA64)
	active_tests_attrs_map["systemCPU"] = "x86_64";
#elif defined(__POWERPC__)
	active_tests_attrs_map["systemCPU"] = "ppc";
#elif defined(__arm__) || defined(__ARM__)
	active_tests_attrs_map["systemCPU"] = "arm";
#else
	active_tests_attrs_map["systemCPU"] = "unknown";
#endif
	active_tests_attrs_map["systemLanguage"] = "en";
	add_language("en",1.0f);
#if defined(AMBULANT_PLATFORM_MACOS)
	active_tests_attrs_map["systemOperatingSystem"] = "macos";
#elif defined(AMBULANT_PLATFORM_WIN32)
	active_tests_attrs_map["systemOperatingSystem"] = "win32";
#elif defined(AMBULANT_PLATFORM_LINUX)
	active_tests_attrs_map["systemOperatingSystem"] = "linux";
#else
	active_tests_attrs_map["systemOperatingSystem"] = "unknown";
#endif
	active_tests_attrs_map["systemScreenSize"] = "1024X1280";
	active_tests_attrs_map["systemScreenDepth"] = "32";
	set_current_system_component_value(AM_SYSTEM_COMPONENT("Ambulant"), true);
	set_current_system_component_value(AM_SYSTEM_COMPONENT("Version2.0"), true);
	set_current_system_component_value(AM_SYSTEM_COMPONENT("Version" AMBULANT_VERSION), true);
	set_current_system_component_value(AM_SYSTEM_COMPONENT("Version" AMBULANT_VERSION "Exact"), true);
	set_current_system_component_value(AM_SYSTEM_COMPONENT("SeamlessPlayback"), true);
}

class smil2::state_test_methods_impl : public common::state_test_methods {
  public:
	bool smil_audio_desc() const {
		return test_attrs::test_on_off_attr("systemAudioDesc", "on");
	}
	int smil_bitrate() const {
		return atoi(get_test_attribute("systemBitrate").c_str());
	}
	bool smil_captions() const {
		return test_attrs::test_on_off_attr("systemCaptions", "on");
	}
	bool smil_component(std::string uri) const {
		return test_attrs::test_system_component(uri.c_str());
	}
	bool smil_custom_test(std::string name) const {
		lib::logger::get_logger()->trace("smil-customTest() not implemented yet");
		return true;
	}
	std::string smil_cpu() const {
		return get_test_attribute("systemCPU");
	}
	float smil_language(std::string lang) const {
		return test_attrs::get_system_language_weight(lang);
	}
	std::string smil_operating_system() const {
		return get_test_attribute("systemOperatingSystem");
	}
	std::string smil_overdub_or_subtitle() const {
		return get_test_attribute("systemOverdubOrSubtitle");
	}
	bool smil_required(std::string uri) const {
		lib::logger::get_logger()->trace("smil-required() not implemented yet");
		return true;
	}
	int smil_screen_depth() const {
		return atoi(get_test_attribute("systemScreenDepth").c_str());
	}
	int smil_screen_height() const {
		std::string s = get_test_attribute("systemScreenSize");
		if(s.empty()) return 0;
		lib::tokens_vector sys_v(s.c_str(), "Xx");
		if(sys_v.size() != 2)
			return 0;
		return atoi(sys_v[0].c_str());
	}
	int smil_screen_width() const {
		std::string s = get_test_attribute("systemScreenSize");
		if(s.empty()) return 0;
		lib::tokens_vector sys_v(s.c_str(), "Xx");
		if(sys_v.size() != 2)
			return 0;
		return atoi(sys_v[1].c_str());
	}
};

common::state_test_methods *
test_attrs::get_state_test_methods()
{
	static smil2::state_test_methods_impl *singleton;

	if (singleton == NULL) {
		if(active_tests_attrs_map.empty())
			set_default_tests_attrs();
		singleton = new smil2::state_test_methods_impl();
	}
	return singleton;
}

// API for embedders and extenders that want to fiddle with components and
// custom tests
bool
test_attrs::get_current_custom_test_value(std::string name)
{
	if (active_custom_tests_attrs_map.count(name))
		return active_custom_tests_attrs_map[name];
	return false;
}

void
test_attrs::set_current_custom_test_value(std::string name, bool value)
{
	active_custom_tests_attrs_map[name] = value;
	// XXX should raise contentControlChange
}

bool
test_attrs::get_current_system_component_value(std::string name)
{
	return test_system_component(name.c_str());
}

void
test_attrs::set_current_system_component_value(std::string name, bool enabled)
{
	if(active_tests_attrs_map.empty())
		set_default_tests_attrs();
	std::string s = get_test_attribute("systemComponent");
	std::list<std::string> list;
	std::list<std::string> newlist;
	lib::split_trim_list(s, list, ' ');
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		if((*it) != name)
			newlist.push_back((*it));
	}
	if (enabled)
		newlist.push_back(name);
	std::string value;
	for (it = newlist.begin(); it!= newlist.end(); it++) {
		if (it != newlist.begin())
			value += ' ';
		value += *it;
	}

	active_tests_attrs_map["systemComponent"] = value;
	// XXX should raise contentControlChange
}

void
test_attrs::set_current_screen_size(int height, int width)
{
	char buf[32];
#if defined(AMBULANT_PLATFORM_WIN32)
	sprintf_s(buf, sizeof buf, "%dx%d", height, width);
#else
	snprintf(buf, sizeof buf, "%dx%d", height, width);
#endif
	active_tests_attrs_map["systemScreenSize"] = std::string(buf);
	// XXX should raise contentControlChange
}

void
test_attrs::clear_languages()
{
	active_language_map.clear();
	// XXXX Should we insert language from systemTests???
	// XXX should raise contentControlChange
}

void
test_attrs::add_language(std::string langname, float weight)
{
	AM_DBG lib::logger::get_logger()->trace("add_language('%s', %f)", langname.c_str(), weight);
	active_language_map[langname] = weight;
	// XXX should raise contentControlChange
}

float
test_attrs::get_system_language_weight(std::string lang)
{
	AM_DBG lib::logger::get_logger()->trace("get_system_language_weight('%s')", lang.c_str());
	while (active_language_map.count(lang) == 0) {
		// See if we can split the language (nl-be -> nl, for example)
		std::string::size_type dashPos = lang.rfind('-');
		if (dashPos == std::string::npos) {
			AM_DBG lib::logger::get_logger()->trace("get_system_language_weight('%s') not found -> 0.0", lang.c_str());
			return 0.0f;
		}
		lang = lang.substr(0, dashPos-1);
	}
	AM_DBG lib::logger::get_logger()->trace("get_system_language_weight('%s') -> %f", lang.c_str(), active_language_map[lang]);
	return active_language_map[lang];
}

