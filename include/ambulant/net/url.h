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

#ifndef AMBULANT_NET_URL_H
#define AMBULANT_NET_URL_H

#include "ambulant/config/config.h"

#include <string>
#include <list>

#include "ambulant/lib/string_util.h"

namespace ambulant {

namespace net {

class url {

	// String type used by this impplementation
	typedef std::basic_string<char> string;
	
	// Size type
	typedef std::basic_string<char>::size_type size_type;
	
	// Short type representing a protocol port.
	typedef unsigned short short_type;
	
    // The protocol to use (ftp, http, nntp, ... etc.) 
    string m_protocol;

    // The host name to connect to.
    string m_host;

    // The protocol port to connect to.
    short_type m_port;

	// The path part of this url.
    string m_path;
    
	// The query part of this url.
    string m_query;

	// The ref or fragment.
    string m_ref;
    
  public:
 	url(); 
 
	url(const string& spec); 
	url(const string& protocol, const string& host, const string& path); 
		
	url(const string& protocol, const string& host, int port, 
		const string& path); 
	
	url(const string& protocol, const string& host, int port, 
		const string& path, const string& query, const string& ref); 
		
	const string& get_protocol() const {
		return m_protocol;
	}
	
	const string& get_host() const {
		return m_host;
	}
	
	short_type get_port() const {
		return m_port;
	}
	
	const string& get_ref() const {
		return m_ref;
	}
	
	const string& get_query() const {
		return m_query;
	}
	
	const string& get_path() const {
		return m_path;
	}
	
	string get_file() const;
	
	std::string url::repr() const;
	
 	static void init_statics();
 	
  private:
	// protocols to ports map
 	// static std::map<string, short_type > s_ports;
 
	// workaround for g++ 2.95
	struct handler_pair { 
		const char *first; 
		void (url::*second)(ambulant::lib::scanner& sc, const std::string& pat);
	};
	
  	//typedef void (url::*HANDLER)(ambulant::lib::scanner& sc, const std::string& pat);
	static std::list<handler_pair*> s_handlers;
	
	void set_parts(ambulant::lib::scanner& sc, const std::string& pat);
	
	// split url string representation
 	void set_from_spec(const string& spec);
 
	// pat: "n://n:n/"
	void set_from_host_port_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n://n/"
	void set_from_host_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n:///"
	void set_from_localhost_file_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "/n"
	void set_from_unix_path(ambulant::lib::scanner& sc, const std::string& pat);
	
	// pat: "n:n" or "n:/n"
	void set_from_windows_path(ambulant::lib::scanner& sc, const std::string& pat);
	
};

/////////////////////////
// inline implementation
inline 
url::url() 
:	m_port(0) {
}

inline 
url::url(const string& spec) 
:	m_port(0) {
	set_from_spec(spec);
}
	
inline 
url::url(const string& protocol, const string& host, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(0),
	m_path(path) {
}

inline		
url::url(const string& protocol, const string& host, int port, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path) {
}

inline
url::url(const string& protocol, const string& host, int port, 
	const string& path, const string& query, const string& ref) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path), 
	m_query(query), 
	m_ref(ref) {
}

inline 
url::string url::get_file() const {
	std::string file = get_path();
	if(!m_query.empty()) {
		file += '?';
		file += m_query;
	}
	return file;
}

} // namespace net
 
} // namespace ambulant


#ifndef AMBULANT_NO_IOSTREAMS
inline 
std::ostream& operator<<(std::ostream& os, const ambulant::net::url& u) {
	return os << u.repr();
}
#endif


#endif // AMBULANT_NET_URL_H
