
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_NET_URL_H
#define AMBULANT_NET_URL_H

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
 
  	typedef void (url::*HANDLER)(ambulant::lib::reg_scanner& sc, const std::string& pat);
	static std::list< std::pair<std::string, HANDLER> > s_handlers;
	
	void set_parts(ambulant::lib::reg_scanner& sc, const std::string& pat);
	
	// split url string representation
 	void set_from_spec(const string& spec);
 
	// pat: "n://n:n/"
	void set_from_host_port_uri(ambulant::lib::reg_scanner& sc, const std::string& pat); 
	
	// pat: "n://n/"
	void set_from_host_uri(ambulant::lib::reg_scanner& sc, const std::string& pat); 
	
	// pat: "n:///"
	void set_from_localhost_file_uri(ambulant::lib::reg_scanner& sc, const std::string& pat); 
	
	// pat: "/n"
	void set_from_unix_path(ambulant::lib::reg_scanner& sc, const std::string& pat);
	
	// pat: "n:n" or "n:/n"
	void set_from_windows_path(ambulant::lib::reg_scanner& sc, const std::string& pat);
	
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

inline 
std::ostream& operator<<(std::ostream& os, const ambulant::net::url& u) {
	return os << u.repr();
}

#endif // AMBULANT_NET_URL_H

 
 