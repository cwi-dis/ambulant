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

#ifndef AMBULANT_COMMON_RENDERER_SELECT_H
#define AMBULANT_COMMON_RENDERER_SELECT_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"

// Macro for creating an Ambulant systemComponent URI
#define AM_SYSTEM_COMPONENT(s) "http://www.ambulantplayer.org/component/" s

namespace ambulant {

namespace common {

class playable_factory;

class AMBULANTAPI renderer_select {
  public:
	renderer_select(const lib::node *n)
	:	m_node(n),
		m_url_valid(false),
		m_mimetype_valid(false),
		m_mimetype(""),
		m_renderer_uri_valid(false),
		m_renderer_uri(NULL),
		m_pf(NULL)
	{}
	renderer_select(const char *uri)
	:   m_node(NULL),
		m_url_valid(false),
		m_mimetype_valid(false),
		m_mimetype(""),
		m_renderer_uri_valid(true),
		m_renderer_uri(uri),
		m_pf(NULL)
	{}
	~renderer_select() {}

	const lib::xml_string& get_tag() const { static std::string empty(""); return m_node?m_node->get_local_name():empty; }
	const net::url& get_url();
	const std::string& get_mimetype();
	const char* get_renderer_uri();
	playable_factory *get_playable_factory() { return m_pf; }
	void set_playable_factory(playable_factory *pf) { m_pf = pf; }

  private:
	const lib::node *m_node;
	bool m_url_valid;
	net::url m_url;
	bool m_mimetype_valid;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	std::string m_mimetype;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	bool m_renderer_uri_valid;
	const char *m_renderer_uri;
	playable_factory *m_pf;
};

#if 0
struct renderer_select_template {
	lib::xml_string tags[];
	std::string mimetypes[];
	char *renderer_uris[];
};
#endif

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_RENDERER_SELECT_H
