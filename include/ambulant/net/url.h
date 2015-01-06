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

#ifndef AMBULANT_NET_URL_H
#define AMBULANT_NET_URL_H

#include "ambulant/config/config.h"

#include <string>
#include <list>

#include "ambulant/lib/string_util.h"

namespace ambulant {

namespace net {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

/// Class representing a URL.
class AMBULANTAPI url {

	// String type used by this impplementation
	typedef std::string string;

	// Size type
	typedef std::string::size_type size_type;

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

	// The query part of this url.
	string m_query;

	// The ref or fragment.
	string m_ref;

	// The mime type
	string m_mime;

  protected:
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

  public:
	/// Default constructor: create an empty URL.
	url();

	/// Factory function: create URL given a URL string
	static url from_url(const std::string& spec) {
		return url(spec);
	}

	/// Factory function: create URL given a filename string
	static url from_filename(const std::string& spec, bool handle_frag=false);

	/// Factory function: create URL given a filename string
	static  url from_filename(const char *spec, bool handle_frag=false);


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

	/// Return true if the path of this URL is empty
	bool is_empty_path() const {
		return m_path == "";
	}

	/// Return true if the URL refers to a local file.
	bool is_local_file() const;

	/// If the URL refers to a local file: return the filename.
	string get_file() const;

	/// Return the whole URL as a string.
	string get_url() const;

//	/// Return the whole URL as a string.
//	operator string() const { return get_url(); }

	/// Join a relative URL to a base URL.
	/// An absolute URL is returned as-is.
	url join_to_base(const url &base) const;

	/// Return a URL pointing to the directory containing this URL
	url get_base() const;

	/// Return a URL with the fragment stripped.
	url get_document() const;

	/// Return a URL with a new fragment.
	url add_fragment(string fragment) const;

	/// Return true if two URLs refer to the same document.
	/// In other words, if they only differ in fragment.
	bool same_document(const url &base) const;

	/// Return the absolute pathname of probably cached datafile
	/// as an URL. Implementation may be platform dependent.
	std::pair<bool, url> get_local_datafile() const;

	/// Guess the mimetype. Supports only very few types, returns
	/// the empty string if it cannot guess.
	std::string guesstype() const;

	/// Set the directory where datafiles (as returned by get_local_datafile())
	/// normally reside on this platform. The argument is a platform-style
	/// pathname, not a URL.
	static void set_datafile_directory(std::string pathname);

	/// Set a flag that determines whether we are strict about URL conformance
	/// (illegal characters, etc) or not
	static void set_strict_url_parsing(bool strict);

	/// Initializer for static member information.
	static void init_statics();

  private:

	// protocols to ports map
	// static std::map<string, short_type > s_ports;

	// Check that URL has correctly escaped characters
	void _checkurl() const;

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
	void set_from_absolute_path(ambulant::lib::scanner& sc, const std::string& pat);

	// pat: "n"
	void set_from_relative_path(ambulant::lib::scanner& sc, const std::string& pat);

	// pat: "n:,"
	void set_from_data_uri(ambulant::lib::scanner& sc, const std::string& pat);

	// pat: "n:"
	void set_from_scheme(ambulant::lib::scanner& sc, const std::string& pat);

};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace net

} // namespace ambulant


inline std::string repr(const ambulant::net::url& u) {
	std::string os;
	if (u.is_absolute()) {
		if(u.get_protocol() == "file") {
			os << u.get_protocol() << "://" <<
				((u.get_host()=="localhost")?"":u.get_host());
		} else if (u.get_protocol() == "data") {
			os << "data:,";
		} else if (u.get_host() != "" ) {
			os << u.get_protocol() << "://" << u.get_host();
			if(u.get_port() != 0) os << ":" << int(u.get_port());
		} else {
			os << u.get_protocol() << ":";
		}
	}
	os << u.get_path();
	if(!u.get_ref().empty())
		os << "#" << u.get_ref();
	if(!u.get_query().empty())
		os << "?" << u.get_query();
	return os;
}

#endif // AMBULANT_NET_URL_H
