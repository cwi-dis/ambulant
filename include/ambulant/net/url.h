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

/// Class representing a URL.
class url {

	// String type used by this impplementation
	typedef std::basic_string<char> string;
	
	// Size type
	typedef std::basic_string<char>::size_type size_type;
	
	// Short type representing a protocol port.
	typedef unsigned short short_type;
	
	// True if absolute URL. m_protocol, m_host and m_port are
	// only valid for absolulte URLs.
	bool m_absolute;
	
    // The protocol to use (ftp, http, nntp, ... etc.) 
    string m_protocol;

    // The host name to connect to.
    string m_host;

    // The protocol port to connect to.
    short_type m_port;

	// The path part of this url.
    string m_path;

	// The path separator characters in m_path
	char *m_pathsep;
    
	// The query part of this url.
    string m_query;

	// The ref or fragment.
    string m_ref;
	
    // The mime type
    string m_mime;
	
  public:
  
	/// Default constructor: create an empty URL.
 	url(); 
 
	/// Create a URL from a given string.
	url(const string& spec); 
	
	/// Create a URL from protocol, host and path.
	url(const string& protocol, const string& host, const string& path); 
		
	/// Create a URL from protocol, host, port and path.
	url(const string& protocol, const string& host, int port, 
		const string& path); 
	
	/// Create a URL from protocol, host, port, path, query and fragment.
	url(const string& protocol, const string& host, int port, 
		const string& path, const string& query, const string& ref); 
		
	/// Return the protocol of this URL.
	const string& get_protocol() const {
		return m_protocol;
	}
	
	/// Return the host of this URL.
	const string& get_host() const {
		return m_host;
	}
	
	/// Return the port of this URL.
	short_type get_port() const {
		return m_port;
	}
	
	/// Return the fragment of this URL.
	const string& get_ref() const {
		return m_ref;
	}
	
	/// Return the query of this URL.
	const string& get_query() const {
		return m_query;
	}
	
	/// Return the path of this URL.
	const string& get_path() const {
		return m_path;
	}
	
	/// Return the mimetype of this URL (not implemented).
	const string& get_mime() const {
		return m_mime;
	}
	
	/// Return true if the URL is absolute.
	bool is_absolute() const {
		return m_absolute;
	}
	
	/// Return true if the URL refers to a local file.
	bool is_local_file() const;
	
	/// If the URL refers to a local file: return the filename.
	string get_file() const;
	
	/// Return the whole URL as a string.
	string get_url() const;

	/// Return the whole URL as a string.
	operator string() const { return get_url(); }
	
	/// Join a relative URL to a base URL.
	/// An absolute URL is returned as-is.
	url join_to_base(const url &base) const;
	
	/// Return true if two URLs refer to the same document.
	/// In other words, if they only differ in fragment.
	bool same_document(const url &base) const;
		
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
 
	// pat: "n://n:d/"
	void set_from_host_port_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n://dn:d/"
	void set_from_numhost_port_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n://n/"
	void set_from_host_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n://dn/"
	void set_from_numhost_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "n:///"
	void set_from_localhost_file_uri(ambulant::lib::scanner& sc, const std::string& pat); 
	
	// pat: "/n"
	void set_from_unix_path(ambulant::lib::scanner& sc, const std::string& pat);
	
	// pat: "n:n" or "n:/n"
	void set_from_windows_path(ambulant::lib::scanner& sc, const std::string& pat);
	
	// pat: "\\n"
	void set_from_wince_path(ambulant::lib::scanner& sc, const std::string& pat);
	
	// pat: "n"
	void set_from_relative_path(ambulant::lib::scanner& sc, const std::string& pat);

	// pat: "n:,"
	void set_from_data_uri(ambulant::lib::scanner& sc, const std::string& pat);
	
};

} // namespace net
 
} // namespace ambulant


#if !defined(AMBULANT_PLATFORM_WIN32_WCE)
inline std::string repr(const ambulant::net::url& u) {
	std::string os;
	if (u.is_absolute()) {
		if(u.get_protocol() == "file") {
			os << u.get_protocol() << "://" << 
				((u.get_host()=="localhost")?"":u.get_host()) << "/";
		} else if (u.get_protocol() == "data") {
			os << "data:,";
		} else {
			os << u.get_protocol() << "://" << u.get_host();
			if(u.get_port() != 0) os << ":" << int(u.get_port());
			os <<  "/";
		}
	}
	os << u.get_path();
	if(!u.get_ref().empty()) 
		os << "#" << u.get_ref();
	if(!u.get_query().empty()) 
		os << "?" << u.get_query();
	return os;
}
#else 
inline std::string repr(const ambulant::net::url& u) {
	std::string os;
	if (u.is_absolute())
		os += u.get_protocol() + "//" + u.get_host() + "/" + u.get_path();
	else
		os += u.get_path();
	return os;
}

void set_url_from_spec(ambulant::net::url& u, const char *spec);


#endif


#endif // AMBULANT_NET_URL_H
