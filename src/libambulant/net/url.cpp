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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/net/url.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/filesys.h"
 
#include <string>
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <sstream>
#endif


using namespace ambulant;

#ifndef AMBULANT_PLATFORM_WIN32_WCE
const std::string url_delim = ":/?#,";
#else
const std::string url_delim = ":/?#\\,";
#endif

// static 
//std::list< std::pair<std::string, net::url::HANDLER> > net::url::s_handlers;
// workaround for g++ 2.95
std::list< net::url::handler_pair* > net::url::s_handlers;

// static
void net::url::init_statics() {

	// workaround for g++ 2.95
	static handler_pair h1 = {"n://n:n/", &url::set_from_host_port_uri};
 	s_handlers.push_back(&h1);
 	
	static handler_pair h1a = {"n://n:n", &url::set_from_host_port_uri};
 	s_handlers.push_back(&h1a);
 	
	static handler_pair h2 = {"n://n/", &url::set_from_host_uri};
 	s_handlers.push_back(&h2);
 	
	static handler_pair h2a = {"n://n", &url::set_from_host_uri};
 	s_handlers.push_back(&h2a);
 	
	static handler_pair h3 = { "n:///", &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h3);
 	
	static handler_pair h4 = { "n:///", &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h4);

	static handler_pair h4a = { "n:,", &url::set_from_data_uri};
 	s_handlers.push_back(&h4a);

	static handler_pair h5 = {"/n", &url::set_from_unix_path};
 	s_handlers.push_back(&h5);

	static handler_pair h6 = {"n:n", &url::set_from_windows_path};
 	s_handlers.push_back(&h6);
 	
	static handler_pair h7 = {"n:/n", &url::set_from_windows_path};
 	s_handlers.push_back(&h7);
 	
	static handler_pair h8 = {"\\n", &url::set_from_wince_path};
 	s_handlers.push_back(&h8);
	
	static handler_pair h9 = {"", &url::set_from_relative_path};
 	s_handlers.push_back(&h9);
	
	/*
	typedef std::pair<std::string, HANDLER> pair;
 	s_handlers.push_back(pair("n://n:n/",&url::set_from_host_port_uri));
  	s_handlers.push_back(pair("n://n/",&url::set_from_host_uri));
  	s_handlers.push_back(pair("n:///",&url::set_from_localhost_file_uri));
   	s_handlers.push_back(pair("/n",&url::set_from_unix_path));
 	s_handlers.push_back(pair("n:n",&url::set_from_windows_path));
  	s_handlers.push_back(pair("n:/n",&url::set_from_windows_path));
  	*/
 }
 
net::url::url() 
:	m_absolute(true),
	m_port(0),
	m_pathsep("/")
{
}
 
net::url::url(const string& spec) 
:	m_port(0),
	m_pathsep("/")
{
	set_from_spec(spec);
}
	 
net::url::url(const string& protocol, const string& host, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(0),
	m_path(path),
	m_pathsep("/")
{
	m_absolute = (m_protocol != "");
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path),
	m_pathsep("/")
{
	m_absolute = (m_protocol != "");
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path, const string& query, const string& ref) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path),
	m_pathsep("/"), 
	m_query(query), 
	m_ref(ref)
{
	m_absolute = (m_protocol != "");
}
 
net::url::string net::url::get_file() const {
	std::string file = get_path();
	if(!m_query.empty()) {
		file += '?';
		file += m_query;
	}
	return file;
}

void net::url::set_from_spec(const string& spec) {
	lib::scanner sc(spec, url_delim);
	sc.tokenize();
	std::string sig = sc.get_tokens();
	//std::list< std::pair<std::string, HANDLER> >::iterator it;
	std::list<handler_pair*>::iterator it;
	for(it=s_handlers.begin();it!=s_handlers.end();it++) {
		handler_pair *ph = (*it);
		if(*(ph->first) == '\0' || lib::starts_with(sig, ph->first)) {
			//HANDLER h = (*it).second;
			(this->*(ph->second))(sc, ph->first);
			return;
		}
	}
	lib::logger::get_logger()->error("url::set_from_spec: Cannot parse \"%s\"", spec.c_str());
}

// pat: "n://n:n/"
void net::url::set_from_host_port_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	m_port = short_type(atoi(sc.val_at(6).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://n/"
void net::url::set_from_host_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	set_parts(sc, pat);
	if(m_protocol == "http")
		m_port = 80;
	else if(m_protocol == "ftp")
		m_port = 21;
}

// pat: "n:///" for file:///
void net::url::set_from_localhost_file_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = "localhost";
	m_port = 0;
	set_parts(sc, pat);
}

// pat: "/n"
void net::url::set_from_unix_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
}

// pat: "n:n" or "n:/n"
void net::url::set_from_windows_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
	m_pathsep = "/\\";
}

// pat: "\\n"
void net::url::set_from_wince_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
	m_pathsep = "/\\";
}

void net::url::set_from_relative_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = false;
	m_protocol = "";
	m_host = "";
	m_port = 0;
	set_parts(sc, pat);
	AM_DBG lib::logger::get_logger()->trace("url::set_from_relative_path: \"%s\" -> \"%s\"", pat.c_str(), m_path.c_str());
}

// pat: "data:,"
void net::url::set_from_data_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "data";
	m_host = "";
	m_port = 0;
	m_path = sc.join(3);  // Skip data:,
}

void net::url::set_parts(lib::scanner& sc, const std::string& pat) {
	const std::string& toks = sc.get_tokens();
	size_type n = toks.length();
	size_type i1 = pat.length();
	size_type i2 = toks.find_last_of('?');
	size_type i3 = toks.find_last_of('#');
	i2 = (i2 == std::string::npos)?n:i2;
	i3 = (i3 == std::string::npos)?n:i3;
	size_type i4 = i2<i3?i2:i3;
	m_path = sc.join(i1, i4);
	m_query = sc.join(i2+1, i3);
	m_ref = sc.join(i3+1);
}

bool net::url::is_local_file() const
{
	if (m_protocol == "file" && (m_host == "localhost" || m_host == ""))
		return true;
	if (!m_absolute && m_protocol == "") {
		// We're not sure.
		lib::logger::get_logger()->warn("url::is_local_file: assume True for relative url: \"%s\"", repr(*this).c_str());
		return true;
	}
	return false;
}
	
std::string net::url::get_url() const
{
	std::string rv = repr(*this);
	if (!m_absolute)
		lib::logger::get_logger()->warn("url::get_url(): URL not absolute: \"%s\"", rv.c_str());
	return rv;
}

net::url net::url::join_to_base(const net::url &base) const
{
	// Note: this hasn't been checked against RFCxxxx. We pick up protocol, host, port
	// and initial pathname from base. Alll other items from base are ignored.
	if (m_absolute) return *this;
	std::string basepath = base.get_file();
	std::string newpath = get_file();
	if (newpath == "") {
		// New path is, for instance, only #anchor.
		newpath = basepath; 
	} else if (newpath[0] != '/') {
		// New_path is not absolute. Prepend base of basepath
		newpath = lib::filesys::join(lib::filesys::get_base(basepath, base.m_pathsep), newpath, m_pathsep);
		//newpath = lib::filesys::join(basepath, newpath, "/");
	}
	AM_DBG lib::logger::get_logger()->trace("url::join_to_base: old \"%s\" base \"%s\" newpath \"%s\"",
		repr(*this).c_str(), repr(base).c_str(), newpath.c_str());
	return net::url(
		base.get_protocol(),
		base.get_host(),
		base.get_port(),
		newpath,
		m_query,
		m_ref);
}

bool net::url::same_document(const net::url &base) const
{
	return (m_protocol == base.m_protocol &&
		m_host == base.m_host &&
		m_port == base.m_port &&
		m_path == base.m_path &&
		m_query == base.m_query);
}


///////////////
// module private static initializer

class url_static_init {
  public:
	url_static_init() {
		net::url::init_statics();
	}
};

static url_static_init url_static_init_inst;
