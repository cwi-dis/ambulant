// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

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
#include "ambulant/lib/textptr.h"

#include <string>
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <sstream>
#endif

using namespace ambulant;

bool ambulant::net::url::s_strict = false;
const std::string url_delim = ":/?#,";
// Characters to be escaped in pathnames. Note that ~ and ? have special meanings in
// http: urls, but not specifically in file: urls.
const std::string file_url_escape_reqd = " <>{}|\\^[]`";
const std::string file_url_escape = file_url_escape_reqd + "%";
const std::string file_url_escape_frag = file_url_escape + "#";

//
// Helper routines to convert local file pathnames to url-style paths
// and vice-versa
//
#ifdef AMBULANT_PLATFORM_WIN32

#include "ambulant/lib/win32/win32_error.h"
#include <wininet.h>

static std::string
filepath2urlpath(const std::string& fparg, bool handle_frag=false)
{
	std::string filepath = fparg;
	size_t urlbufsize = filepath.size()*3+7; // Worst case: all characters escaped
	LPTSTR urlbuf = (LPTSTR)malloc(urlbufsize*sizeof(TCHAR));
	DWORD urlbufsizearg = (DWORD)urlbufsize;
	assert(urlbuf);
	urlbuf[0] = 0;
	std::string fragment;
	if (handle_frag) {
		// Unfortunately passing ICU_BROWSER_MODE to ICU doesn't work:-(
		// Remove the fragment by hand, re-apply it later.
		size_t fragpos = filepath.find('#');
		if (fragpos != std::string::npos) {
			fragment = filepath.substr(fragpos);
			filepath = filepath.substr(0, fragpos-1);
		}
	}
	if (!InternetCanonicalizeUrl(lib::textptr(filepath.c_str()), urlbuf, &urlbufsizearg, 0)) {
		DWORD dw = GetLastError();
		lib::win32::win_report_error(filepath.c_str(), dw);
		urlbuf[0] = 0;
	}
	std::string rv = lib::textptr(urlbuf);
	// Work around stupid bug in InternetCanonicalizeURL: it forgets a slash.
	if (rv.substr(0, 7) == "file://" && rv[7] != '/')
		rv = "file:///" + rv.substr(7);
	// Now replace backslashes and turn everything into lower case
	std::string::iterator i;
	for(i=rv.begin(); i!=rv.end(); i++) {
		char c = *i;
		if (c == '\\') 
			*i = '/';
		else
			*i = tolower(c);
	}
	free(urlbuf);
	// Finally re-apply the fragment id, if there is one
	if (fragment != "") {
		rv = rv + fragment;
	}
	return rv;
}

static std::string
urlpath2filepath(const std::string& urlpath)
{
	size_t filebufsize = urlpath.size()+1;
	LPTSTR filebuf = (LPTSTR)malloc(filebufsize*sizeof(TCHAR));
	DWORD filebufsizearg = (DWORD)filebufsize;
	assert(filebuf);
	filebuf[0] = 0;
	lib::textptr tp(urlpath.c_str());
	if (!InternetCanonicalizeUrl(tp, filebuf, 
			&filebufsizearg, ICU_DECODE | ICU_NO_ENCODE)) {
		DWORD dw = GetLastError();
		lib::win32::win_report_error(urlpath.c_str(), dw);
		filebuf[0] = 0;
	}
	std::string rv = lib::textptr(filebuf);
	// Work around stupid bug in InternetCanonicalizeURL: it forgets a slash.
	if (rv.substr(0, 8) == "file:///")
		rv = rv.substr(8);
	else if (rv[0] == '/' && rv[2] == ':')
		rv = rv.substr(1);
	// Finally replace slashes by backslashes
	std::string::iterator i;
	for(i=rv.begin(); i!=rv.end(); i++) {
		char c = *i;
		if (c == '/') 
			*i = '\\';
	}
	free(filebuf);
	return rv;
}
#else
// Unix implementation
static std::string
filepath2urlpath(const std::string& filepath, bool handle_frag=false)
{
	std::string::const_iterator i;
	std::string rv;
	const std::string &esc = file_url_escape;
	if (handle_frag) esc = file_url_escape_frag;
	for(i=filepath.begin(); i!=filepath.end(); i++) {
		char c = *i;
		if ( file_url_escape.find(c) != std::string::npos ) {
			char buf[4];
			sprintf(buf, "%%%02.2x", (unsigned)c);
			rv += buf;
		} else {
			rv += c;
		}
	}
	return rv;
}

static std::string
urlpath2filepath(const std::string& urlpath)
{
	std::string::const_iterator i;
	std::string rv;
	for(i=urlpath.begin(); i!=urlpath.end(); i++) {
		char c = *i;
		if ( c == '%' ) {
			char buf[3];
			unsigned utfval;
			buf[0] = *++i;
			buf[1] = *++i;
			buf[2] = '\0';
			if (sscanf(buf, "%x", &utfval) == 1) {
				rv += (char)utfval;
			} else {
				// Put the original string back. What else can we do...
				rv += '%';
				rv += buf;
			}
		} else {
			rv += c;
		}
	}
	return rv;
}
#endif // AMBULANT_PLATFORM_WIN32

// static 
//std::list< std::pair<std::string, net::url::HANDLER> > net::url::s_handlers;
// workaround for g++ 2.95
std::list< net::url_handler_pair* > s_handlers;

// static
void net::url::init_statics() {

	// workaround for g++ 2.95
	static url_handler_pair h1 = {"n://n:d/", &url::set_from_host_port_uri};
	s_handlers.push_back(&h1);
	
	static url_handler_pair h1a = {"n://n:d", &url::set_from_host_port_uri};
	s_handlers.push_back(&h1a);
	
	static url_handler_pair h1b = {"n://dn:d/", &url::set_from_numhost_port_uri};
	s_handlers.push_back(&h1b);
	
	static url_handler_pair h1c = {"n://dn:d", &url::set_from_numhost_port_uri};
	s_handlers.push_back(&h1c);
	
	static url_handler_pair h2 = {"n://n/", &url::set_from_host_uri};
	s_handlers.push_back(&h2);
	
	static url_handler_pair h2a = {"n://n", &url::set_from_host_uri};
	s_handlers.push_back(&h2a);
	
	static url_handler_pair h2b= {"n://dn/", &url::set_from_numhost_uri};
	s_handlers.push_back(&h2b);
	
	static url_handler_pair h2c = {"n://dn", &url::set_from_numhost_uri};
	s_handlers.push_back(&h2c);
	
	static url_handler_pair h3 = { "n:///", &url::set_from_localhost_file_uri};
	s_handlers.push_back(&h3);
	
	static url_handler_pair h4 = { "n:///", &url::set_from_localhost_file_uri};
	s_handlers.push_back(&h4);

	static url_handler_pair h4a = { "n:,", &url::set_from_data_uri};
	s_handlers.push_back(&h4a);

	static url_handler_pair h5 = {"/n", &url::set_from_absolute_path};
	s_handlers.push_back(&h5);
	
	static url_handler_pair h6 = {"n:", &url::set_from_scheme};
	s_handlers.push_back(&h6);
	
	static url_handler_pair h9 = {"", &url::set_from_relative_path};
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
 
// static
AMBULANTAPI net::url 
net::url::from_filename(const std::string& spec, bool handle_frag)
{
	return net::url(filepath2urlpath(spec, handle_frag));
}

// static
AMBULANTAPI net::url
net::url::from_filename(const char *spec, bool handle_frag)
{
	return net::url(filepath2urlpath(spec, handle_frag));
}

// Private: check URL for character escaping
void net::url::_checkurl() const
{
	if (m_path.find_first_of(file_url_escape_reqd) != std::string::npos)
		lib::logger::get_logger()->warn("%s: URL contains illegal characters", get_url().c_str());
}
net::url::url() 
:	m_absolute(true),
	m_port(0)
{
}
 
net::url::url(const string& spec) 
:	m_port(0)
{
	set_from_spec(spec);
}
	 
net::url::url(const string& protocol, const string& host, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(0),
	m_path(path)
{
	m_absolute = (m_protocol != "");
	if (s_strict) _checkurl();
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path)
{
	m_absolute = (m_protocol != "");
	if (s_strict) _checkurl();
}

net::url::url(const string& protocol, const string& host, int port, 
	const string& path, const string& query, const string& ref) 
:	m_protocol(protocol),
	m_host(host),
	m_port(short_type(port)),
	m_path(path), 
	m_query(query), 
	m_ref(ref)
{
	m_absolute = (m_protocol != "");
	if (s_strict) _checkurl();
}
 
net::url::string net::url::get_file() const {
	std::string file = get_path();
	// Workaround: we might have split a local file at the ?.
	if(!m_query.empty()) {
		file += '?';
		file += m_query;
	}
	
	return urlpath2filepath(file);
}

void net::url::set_from_spec(const string& spec) {
	lib::scanner sc(spec, url_delim);
	sc.tokenize();
	std::string sig = sc.get_tokens();
	//std::list< std::pair<std::string, HANDLER> >::iterator it;
	std::list<url_handler_pair*>::iterator it;
	for(it=s_handlers.begin();it!=s_handlers.end();it++) {
		url_handler_pair *ph = (*it);
		if(*(ph->first) == '\0' || lib::starts_with(sig, ph->first)) {
			//HANDLER h = (*it).second;
			(this->*(ph->second))(sc, ph->first);
			return;
		}
	}
	lib::logger::get_logger()->error(gettext("%s: Cannot parse URL"), spec.c_str());
}

// pat: "n://n:d/"
void net::url::set_from_host_port_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.val_at(4);
	m_port = short_type(atoi(sc.val_at(6).c_str()));
	set_parts(sc, pat);
}
	
// pat: "n://dn:d/"
void net::url::set_from_numhost_port_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.join(4, 6);
	m_port = short_type(atoi(sc.val_at(7).c_str()));
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

// pat: "n://dn/"
void net::url::set_from_numhost_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = sc.join(4, 6);
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
	// The initial / in the pathname has been eaten
}

// pat: "/n"
void net::url::set_from_absolute_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "file";
	m_host = "localhost";
	m_port = 0;
	m_path = sc.get_src();
	if (s_strict) _checkurl();
}

void net::url::set_from_relative_path(lib::scanner& sc, const std::string& pat) {
	m_absolute = false;
	m_protocol = "";
	m_host = "";
	m_port = 0;
	set_parts(sc, pat);
	AM_DBG lib::logger::get_logger()->debug("url::set_from_relative_path: \"%s\" -> \"%s\"", pat.c_str(), m_path.c_str());
}

// pat: "data:,"
void net::url::set_from_data_uri(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = "data";
	m_host = "";
	m_port = 0;
	m_path = sc.join(3);  // Skip data:,
	if (s_strict) _checkurl();
}

// pat: "scheme:,"
void net::url::set_from_scheme(lib::scanner& sc, const std::string& pat) {
	m_absolute = true;
	m_protocol = sc.val_at(0);
	m_host = "";
	m_port = 0;
	m_path = sc.join(2);  // Skip scheme:
	// Unsure: should we do #fragment processing for unknown URLs?
	// initially I thought not, but the ambulantpdbt: scheme needs it...
	int hpos = m_path.find('#');
	if (hpos != std::string::npos) {
		m_ref = m_path.substr(hpos+1);
		m_path = m_path.substr(0, hpos);
	}
	if (s_strict) _checkurl();
}

void net::url::set_parts(lib::scanner& sc, const std::string& pat) {
	const std::string& toks = sc.get_tokens();
	size_type n = toks.length();
	size_type i1 = pat.length();
	// Most patterns include the initial / of the pathname, but we want it in
	// the pathname.
	if (i1 > 0 && pat[i1-1] == '/') i1--;
	size_type i2 = toks.find_last_of('?');
	size_type i3 = toks.find_last_of('#');
	i2 = (i2 == std::string::npos)?n:i2;
	i3 = (i3 == std::string::npos)?n:i3;
	size_type i4 = i2<i3?i2:i3;
	m_path = sc.join(i1, i4);
	m_query = sc.join(i2+1, i3);
	m_ref = sc.join(i3+1);
	if (s_strict) _checkurl();
}

bool net::url::is_local_file() const
{
	if (m_protocol == "file" && (m_host == "localhost" || m_host == ""))
		return true;
	if (!m_absolute && m_protocol == "") {
		// We're not sure.
		AM_DBG lib::logger::get_logger()->trace("url::is_local_file: assume True for relative url: \"%s\"", repr(*this).c_str());
		return true;
	}
	return false;
}
	
std::string net::url::get_url() const
{
	std::string rv = repr(*this);
	if (!m_absolute)
		lib::logger::get_logger()->trace("url::get_url(): URL not absolute: \"%s\"", rv.c_str());
	return rv;
}

net::url net::url::get_base() const
{
	std::string path = get_path();
	std::string basepath = lib::filesys::get_base(path);
	net::url rv = net::url(
		get_protocol(),
		get_host(),
		get_port(),
		basepath);
	if (m_absolute) rv.m_absolute = true;
	return rv;
}

net::url net::url::join_to_base(const net::url &base) const
{
	// Note: this hasn't been checked against RFCxxxx. We pick up protocol, host, port
	// and initial pathname from base. Alll other items from base are ignored.
	if (m_absolute) return *this;
	std::string basepath = base.get_path();
	std::string newpath = get_path();
	if (newpath == "") {
		// New path is, for instance, only #anchor.
		newpath = basepath; 
	} else if (newpath[0] != '/') {
		// New_path is not absolute. Prepend base of basepath
		basepath = lib::filesys::get_base(basepath);
		// Convert basepath from Windows to URL, if needed.
		// XXXX Incomplete?
//		if (base.m_absolute && basepath[0] != '/')
//			basepath = "/" + basepath;
		std::string::iterator cp;
		for (cp=basepath.begin(); cp != basepath.end(); cp++) {
			char c = *cp;
			if (c == '\\') *cp = '/';
		}
		newpath = lib::filesys::join(basepath, newpath);
		// Now ad
		//newpath = lib::filesys::join(basepath, newpath, "/");
	}
	AM_DBG lib::logger::get_logger()->debug("url::join_to_base: old \"%s\" base \"%s\" newpath \"%s\"",
		repr(*this).c_str(), repr(base).c_str(), newpath.c_str());
	net::url rv = net::url(
		base.get_protocol(),
		base.get_host(),
		base.get_port(),
		newpath,
		m_query,
		m_ref);
	if (base.m_absolute)
		rv.m_absolute = true;
	return rv;
}

net::url net::url::get_document() const
{
	net::url rv = net::url(m_protocol, m_host, m_port, m_path, m_query, "");
	rv.m_absolute = m_absolute;
	return rv;
}

bool net::url::same_document(const net::url &base) const
{
	return (m_protocol == base.m_protocol &&
		m_host == base.m_host &&
		m_port == base.m_port &&
		m_path == base.m_path &&
		m_query == base.m_query);
}

// Very limited guesstype (for now), only guesses some HTML documents.
std::string
net::url::guesstype() const
{
	unsigned int dotpos = m_path.find_last_of(".");
	if (dotpos == std::string::npos) return "";
	std::string ext = m_path.substr(dotpos);
	
	if (ext == ".htm" || ext == ".HTM" || ext == ".html" || ext == ".HTML")
		return "text/html";
	if (ext == ".smi" || ext == ".SMI" || ext == ".smil" || ext == ".SMIL")
		return "application/smil";
	if (ext == ".mp3" || ext == ".MP3")
		return "audio/mpeg";
	if (ext == ".wav" || ext == ".WAV")
		return "audio/wav";
	return "";
}

#if defined(AMBULANT_PLATFORM_UNIX)

// Places where to look for (cached) datafiles
const char *datafile_locations[] = {
	".",		// Placeholder, to be replaced by set_datafile_directory()
	".",
	"..",
	"Extras",
	"../Extras",
#ifdef	AMBULANT_DATADIR
	AMBULANT_DATADIR ,
#else
	"/usr/local/share/ambulant",
#endif
	NULL
};

std::string datafile_directory;

void
net::url::set_datafile_directory(std::string pathname)
{
	datafile_directory = pathname;
	datafile_locations[0] = datafile_directory.c_str();
}

std::pair<bool, net::url>
net::url::get_local_datafile() const
{
	const char* result = NULL;
	if (!is_local_file()) return std::pair<bool, net::url>(false, net::url(*this));
		
	if (! is_absolute()) {
		string rel_path = get_path();
		const char **dir;
		for(dir = datafile_locations; *dir; dir++) {
			string abs_path(*dir);
			abs_path += "/" + rel_path;
			if (access(abs_path.c_str(), 0) >= 0) {
				if (abs_path[0] != '/') {
					char buf[1024];
					if (getcwd(buf, sizeof buf)) {
						std::string curdir(buf);
						abs_path = curdir + "/" + abs_path;
					}
				}
				result = abs_path.c_str();
				break;
			}
		}
	} else if (is_local_file() 
		   && access (get_file().c_str(), 0) >= 0) {
		result = get_file().c_str();
	}
	
	if (!result) return std::pair<bool, net::url>(false, net::url(*this));
	
	return std::pair<bool, net::url>(true, net::url("file", "", result));
}
#else // AMBULANT_PLATFORM_UNIX

// Hack: if it is not Unix it must be windows:-)

// Places where to look for (cached) datafiles
static const char *datafile_locations[] = {
	"Extras\\",
	"..\\..\\Extras\\",
	"",
	"..\\",
	"..\\Extras\\",
	NULL
};

static std::string datafile_directory;

void
net::url::set_datafile_directory(std::string pathname)
{
	datafile_directory = pathname;
}

std::pair<bool, net::url>
net::url::get_local_datafile() const
{
	// XXXX Needs work!
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	// Too lazy to convert char to wide char right now
	return std::pair<bool, net::url>(false, net::url(*this));
#else
	if (datafile_directory == "") {
		set_datafile_directory(lib::win32::get_module_dir());
	}
	const char* result = NULL;
	string path;
	if (!is_local_file()) return std::pair<bool, net::url>(false, net::url(*this));
	
	if (! is_absolute()) {
		string rel_path = get_path();
		const char **dir;
		for(dir = datafile_locations; *dir; dir++) {
			path = datafile_directory + *dir;
			path += rel_path;
			if (lib::win32::file_exists(path)) {
				result = path.c_str();
				break;
			}
		}
	} else if (is_local_file()) {
		path = get_file();
		if (lib::win32::file_exists(path))
			result = path.c_str();
	}
	
	if (!result) return std::pair<bool, net::url>(false, net::url(*this));
	std::string *pathname = new std::string(result);
	return std::pair<bool, net::url>(true, net::url("file", "", *pathname));
#endif
}
#endif //AMBULANT_PLATFORM_UNIX
///////////////
// module private static initializer

class url_static_init {
  public:
	url_static_init() {
		net::url::init_statics();
	}
};

static url_static_init url_static_init_inst;
