
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

/* 
 * A url class represents a Uniform Resource
 * Locator, a pointer to a "resource" on the World
 * Wide Web.
 *
 */
 
#ifndef AMBULANT_NET_URL_H
#define AMBULANT_NET_URL_H

#include <string>

namespace ambulant {

namespace net {

class url {

	// String type used by this impplementation
	typedef std::basic_string<char> string_type;
	
	// Short type representing a protocol port.
	typedef unsigned short short_type;
	
    // The protocol to use (ftp, http, nntp, ... etc.) 
    string_type m_protocol;

    // The host name to connect to.
    string_type m_host;

    // The protocol port to connect to.
    short_type m_port;

	// The specified file name on the host.
    string_type m_file;

	// The ref segment.
    string_type m_ref;
    
	// The query part of this url.
    string_type m_query;

	// The authority part of this url.
    string_type m_authority;

	// The path part of this url.
    string_type m_path;

  public:
  
	url(string_type spec) 
	:	m_port(-1) {
		// split parts of the spec
	}
		
	url(string_type protocol, string_type host, int port, string_type file) 
	:	m_protocol(protocol),
		m_host(host),
		m_port(static_cast<short_type>(port)),
		m_file(file) {
	}
	
	// Returns true when this url is valid.
	bool is_good() const {
		return false;
	}
	
	const string_type& get_protocol() const {
		return m_protocol;
	}
	
	const string_type& get_host() const {
		return m_host;
	}
	
	short_type get_port() const {
		return m_port;
	}
	
	const string_type& get_file() const {
		return m_file;
	}
	
	const string_type& get_ref() const {
		return m_ref;
	}
	
	const string_type& get_query() const {
		return m_query;
	}
	
	const string_type& get_authority() const {
		return m_authority;
	}
	
	const string_type& get_path() const {
		return m_path;
	}
	
};
 
 
} // namespace net
 
} // namespace ambulant

#endif // AMBULANT_NET_URL_H

 
 