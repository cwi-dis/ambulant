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

#include "ambulant/net/url.h"
 
#include <string>

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <sstream>
#endif

#include "ambulant/lib/string_util.h"

using namespace ambulant;

const std::string url_delim = ":/?#";

// static 
//std::list< std::pair<std::string, net::url::HANDLER> > net::url::s_handlers;
// workaround for g++ 2.95
std::list< net::url::handler_pair* > net::url::s_handlers;

// static
void net::url::init_statics() {

	// workaround for g++ 2.95
	static handler_pair h1 = {std::string("n://n:n/"), &url::set_from_host_port_uri};
 	s_handlers.push_back(&h1);
 	
	static handler_pair h2 = {std::string("n://n/"), &url::set_from_host_uri};
 	s_handlers.push_back(&h2);
 	
	static handler_pair h3 = { std::string("n:///"), &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h3);
 	
	static handler_pair h4 = { std::string("n:///"), &url::set_from_localhost_file_uri};
 	s_handlers.push_back(&h4);
 	
	static handler_pair h5 = {std::string("/n"), &url::set_from_unix_path};
 	s_handlers.push_back(&h5);
 	
	static handler_pair h6 = {std::string("n:n"), &url::set_from_windows_path};
 	s_handlers.push_back(&h6);
 	
	static handler_pair h7 = {std::string("n:/n"), &url::set_from_windows_path};
 	s_handlers.push_back(&h7);
	
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

void net::url::set_from_spec(const string& spec) {
	lib::scanner sc(spec, url_delim);
	sc.tokenize();
	std::string sig = sc.get_tokens();
	//std::list< std::pair<std::string, HANDLER> >::iterator it;
	std::list<handler_pair*>::iterator it;
	for(it=s_handlers.begin();it!=s_handlers.end();it++) {
		handler_pair *ph = (*it);
		if(lib::starts_with(sig, ph->first)) {
			//HANDLER h = (*it).second;
			(this->*(ph->second))(sc, ph->first);
			break;
		}
	}
}

// pat: "n://n:n/"
void net::url::set_from_host_port_uri(lib::scanner& sc, const std::string& pat) {
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	m_port = short_type(atoi(sc.val_at(6).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://n/"
void net::url::set_from_host_uri(lib::scanner& sc, const std::string& pat) {
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
	m_protocol = sc.val_at(0);
	m_host = "localhost";
	m_port = 0;
	set_parts(sc, pat);
}

// pat: "/n"
void net::url::set_from_unix_path(lib::scanner& sc, const std::string& pat) {
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
}

// pat: "n:n" or "n:/n"
void net::url::set_from_windows_path(lib::scanner& sc, const std::string& pat) {
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
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

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
std::string net::url::repr() const {
	std::ostringstream os;
	if(m_protocol == "file") {
		os << m_protocol << "://" << 
			((m_host=="localhost")?"":m_host) << "/" << m_path;
	} else {
		os << m_protocol << "://" << m_host << "/" << m_path;
	}
	if(!m_ref.empty()) 
		os << "#" << m_ref;
	if(!m_query.empty()) 
		os << "?" << m_query;
	return os.str();
}
#endif

///////////////
// module private static initializer

class url_static_init {
  public:
	url_static_init() {
		net::url::init_statics();
	}
};

static url_static_init url_static_init_inst;
