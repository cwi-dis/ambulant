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

#include "ambulant/lib/node.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/logger.h"
#include "ambulant/smil2/test_attrs.h"

using namespace ambulant;
using namespace smil2;

static std::map<std::string, std::string> tests_attrs_map;

static 
std::map<std::string, custom_test> custom_tests_map;

inline std::string get_test_attribute(const std::string& attr) {
	std::map<std::string, std::string>::iterator it = tests_attrs_map.find(attr);
	return (it != tests_attrs_map.end())?(*it).second:"";
}

test_attrs::test_attrs(const lib::node *n) 
:	m_node(n) {
	m_logger = lib::logger::get_logger();
	
	if(tests_attrs_map.empty())
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
	if(value && !test_system_language(value))
		return false;
	
	// systemAudioDesc
	value = m_node->get_attribute("systemAudioDesc");
	if(value && !test_on_off_attr("systemAudioDesc", value))
		return false;
	
	// systemBitrate
	value = m_node->get_attribute("systemBitrate");
	if(value && !test_system_bitrate(value))
		return false;
	
	// systemCaptions
	value = m_node->get_attribute("systemCaptions");
	if(value && !test_on_off_attr("systemCaptions", value))
		return false;
	
	// systemCPU
	value = m_node->get_attribute("systemCPU");
	if(value && !test_exact_str_attr("systemCPU", value))
		return false;
		
	// systemOperatingSystem 
	value = m_node->get_attribute("systemOperatingSystem ");
	if(value && !test_exact_str_attr("systemOperatingSystem ", value))
		return false;
		
	// systemOverdubOrSubtitle 
	value = m_node->get_attribute("systemOverdubOrSubtitle ");
	if(value && !test_exact_str_attr("systemOverdubOrSubtitle ", value))
		return false;
	
	// systemScreenDepth
	value = m_node->get_attribute("systemScreenDepth");
	if(value && !test_system_screen_depth(value))
		return false;
	
	// systemScreenSize
	value = m_node->get_attribute("systemScreenSize");
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
	
	return true;
}

// systemLanguage ::= (languageTag (S? ',' S? languageTag)*)?
// return true when any in the list stars with the argument
bool test_attrs::test_system_language(const char *value) const {
	std::string langs = get_test_attribute("systemLanguage");
	if(langs.empty()) return false;
	std::list<std::string> list;
	lib::split_trim_list(langs, list);
	std::list<std::string>::const_iterator it;
	for(it = list.begin(); it!=list.end();it++) {
		if(lib::starts_with(*it, value)) return true;
	}
	return false;
}

bool test_attrs::test_system_component(const char *value) const {
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

bool test_attrs::test_system_bitrate(const char *value) const {
	std::string s = get_test_attribute("systemBitrate");
	if(s.empty()) return false;
	int sys_bitrate = atoi(s.c_str());
	int sel_value = atoi(value);
	return sys_bitrate >= sel_value;
}

bool test_attrs::test_on_off_attr(const std::string& attr,const char *value) const {
	std::string s = get_test_attribute(attr);
	return (s.empty() || (s != "on" && s != "off"))?false:(s == value);
}

bool test_attrs::test_exact_str_attr(const std::string& attr,const char *value) const {
	std::string s = get_test_attribute(attr);
	return s.empty()?false:(s == value);
}

bool test_attrs::test_exact_str_list_attr(const std::string& attr,const char *value) const {
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
bool test_attrs::test_system_screen_depth(const char *value) const {
	std::string s = get_test_attribute("systemScreenDepth");
	if(s.empty()) return false;
	int sys_bpp = atoi(s.c_str());
	int sel_bpp = atoi(value);
	return sys_bpp >= sel_bpp;
}

bool test_attrs::test_system_screen_size(const char *value) const {
	std::string s = get_test_attribute("systemScreenSize");
	if(s.empty()) return false;
	lib::tokens_vector sys_v(s.c_str(), "Xx");
	lib::tokens_vector sel_v(s.c_str(), "Xx");
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
		std::map<std::string, custom_test>::const_iterator cit =
			custom_tests_map.find(*it);
		// if missing default to false
		if(cit == custom_tests_map.end()) return false;
		
		// if state is false return false
		const custom_test& ct = (*cit).second;
		if(!ct.state) return false;
	}
	// all present and evaluated to true
	return true;
}

/////////////////////////
//

// static
bool test_attrs::load_test_attrs(const std::string& filename) {
	lib::tree_builder builder;
	if(!builder.build_tree_from_file(filename.c_str())) {
		lib::logger::get_logger()->error(
			"Could not build tree for file: %s", filename.c_str());
		return false;
	}
	
	tests_attrs_map.clear();
	
	const lib::node* root = builder.get_tree();
	lib::node::const_iterator it;
	lib::node::const_iterator end = root->end();
	for(it = root->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
//		bool start_element = pair.first;
		const lib::node *n = pair.second;
		const std::string& tag = n->get_local_name();
		if(tag == "systemTest" || tag == "property") {
			const char *name = n->get_attribute("name");
			const char *value = n->get_attribute("value");
			if(name && value)
				tests_attrs_map[name] = value;
		} else if(tag == "customTest") {
			const char *name = n->get_attribute("name");
			const char *value = n->get_attribute("value");
			std::string sn = lib::trim(name);
			std::string sv = lib::trim(value);
			if(!sn.empty() && !sv.empty()) {
				std::map<std::string, custom_test>::iterator cit =
					custom_tests_map.find(sn);
				if(cit != custom_tests_map.end())
					(*cit).second.state = (sv == "true")?true:false;
			}
		}
	}	
	return true;
}

// static
void test_attrs::set_default_tests_attrs() {
	tests_attrs_map["systemAudioDesc"] = "on";
	tests_attrs_map["systemBitrate"] = "56000";
	tests_attrs_map["systemCaptions"] = "on";
	tests_attrs_map["systemCPU"] = "unknown";
	tests_attrs_map["systemLanguage"] = "en";
#if defined(AMBULANT_PLATFORM_MACOS)
	tests_attrs_map["systemOperatingSystem"] = "macos";
#elif defined(AMBULANT_PLATFORM_WIN32)
	tests_attrs_map["systemOperatingSystem"] = "win32";
#elif defined(AMBULANT_PLATFORM_WIN32_WCE)
	tests_attrs_map["systemOperatingSystem"] = "wince";
#elif defined(AMBULANT_PLATFORM_LINUX)
	tests_attrs_map["systemOperatingSystem"] = "linux";
#else
	tests_attrs_map["systemOperatingSystem"] = "unknown";
#endif
	tests_attrs_map["systemScreenSize"] = "1024X1280";
	tests_attrs_map["systemScreenDepth"] = "32";
}

////////////////////////////////////////

//static
void test_attrs::read_custom_attributes(const lib::document *doc) {
	const lib::node* ca = doc->locate_node("/smil/head/customAttributes");
	if(!ca) return;
	lib::node::const_iterator it;
	lib::node::const_iterator end = ca->end();
	for(it = ca->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		bool start_element = pair.first;
		const lib::node *n = pair.second;
		const std::string& tag = n->get_local_name();
		if(tag != "customTest") continue;
		const char *p = n->get_attribute("id");
		if(start_element && p) {
			custom_test t;
			t.id = p;
			p = n->get_attribute("defaultState");
			std::string s = p?p:"";
			t.state = (s == "true")?true:false;
			p = n->get_attribute("title");
			t.title = p?p:"";
			p = n->get_attribute("override");
			s = p?p:"";
			t.override = (s=="visible")?true:false;
			p = n->get_attribute("uid");
			t.uid = p?p:""; 
			custom_tests_map[t.id] = t;
		}
	}
}


