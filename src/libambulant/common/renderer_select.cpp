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
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/params.h"

using namespace ambulant;
using namespace common;

const net::url&
renderer_select::get_url()
{
	if (!m_url_valid) {
		if (m_node) m_url = m_node->get_url("src");
		m_url_valid = true;
	}
	return m_url;
}

const std::string&
renderer_select::get_mimetype()
{
	if (!m_mimetype_valid) {
		if (m_node) {
			const net::url& url = get_url();
			m_mimetype = url.guesstype();
		}
		m_mimetype_valid = true;
	}
	return m_mimetype;
}

const char*
renderer_select::get_renderer_uri()
{
	if (!m_renderer_uri_valid) {
		smil2::params *p = smil2::params::for_node(m_node);
		if (p)
			m_renderer_uri = p->get_str("renderer");
		m_renderer_uri_valid = true;
		delete p;
	}
	return m_renderer_uri;
}
