
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/net/url.h"
 
#include <string>
#include <sstream>
#include "ambulant/lib/string_util.h"

using namespace ambulant;

const std::string url_delim = ":/?#";

// static 
std::list< std::pair<std::string, net::url::HANDLER> > net::url::s_handlers;
 
// static
void net::url::init_statics() {
	typedef std::pair<std::string, HANDLER> pair;
 	s_handlers.push_back(pair("n://n:n/",&url::set_from_host_port_uri));
  	s_handlers.push_back(pair("n://n/",&url::set_from_host_uri));
  	s_handlers.push_back(pair("n:///",&url::set_from_localhost_file_uri));
 }

void net::url::set_from_spec(const string& spec) {
	lib::reg_scanner sc(spec, url_delim);
	std::string sig = sc.get_toks();
	std::list< std::pair<std::string, HANDLER> >::iterator it;
	for(it=s_handlers.begin();it!=s_handlers.end();it++) {
		if(lib::starts_with(sig, (*it).first)) {
			HANDLER h = (*it).second;
			(this->*h)(sc, (*it).first);
			break;
		}
	}
}

// pat: "n://n:n/"
void net::url::set_from_host_port_uri(lib::reg_scanner& sc, const std::string& pat) {
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	m_port = short_type(atoi(sc.val_at(6).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://n/"
void net::url::set_from_host_uri(lib::reg_scanner& sc, const std::string& pat) {
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	set_parts(sc, pat);
	if(m_protocol == "http")
		m_port = 80;
	else if(m_protocol == "ftp")
		m_port = 21;
}

// pat: "n:///" for file:///
void net::url::set_from_localhost_file_uri(lib::reg_scanner& sc, const std::string& pat) {
	m_protocol = sc.val_at(0);
	m_host = "localhost";
	m_port = -1;
	set_parts(sc, pat);
}

void net::url::set_parts(lib::reg_scanner& sc, const std::string& pat) {
	const std::string& toks = sc.get_toks();
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

std::string net::url::repr() const {
	std::ostringstream os;
	os << "Protocol: " << get_protocol() << std::endl;
	os << "Host: " << get_host() << std::endl;
	os << "Port: " << get_port() << std::endl;
	os << "Path: " << get_path() << std::endl;
	os << "Query: " << get_query() << std::endl;
	os << "Ref: " << get_ref() << std::endl;
	return os.str();
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

 
 