/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_LOGGER_H
#define AMBULANT_LIB_LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <stdarg.h>

#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {

class logger {
  public:
	// known log levels
	enum log_level { LEVEL_DEBUG, LEVEL_TRACE, 
		LEVEL_WARN, LEVEL_ERROR, LEVEL_FATAL };
		
	logger(const std::string& name);
	
	// static factory function to create loggers
	static logger* get_logger(const char *name = NULL);
	static logger* get_logger(const char *name, int pos);
	
	// static config function
	static void set_loggers_level(int level);
	
	// logging functions at various levels
 	void debug(const char *format, ...);
 	void trace(const char *format, ...);
  	void warn(const char *format, ...);
  	void error(const char *format, ...);
  	void fatal(const char *format, ...);
	
  	// templates for objects defining the operator<<
	template <class T>
 	void debug(const T& obj) { 
 		log_obj(LEVEL_DEBUG, obj);
 	}
 	template <class T>
 	void trace(const T& obj) { 
 		log_obj(LEVEL_TRACE, obj);
 	}
 	template <class T>
 	void warn(const T& obj) { 
 		log_obj(LEVEL_WARN, obj);
 	}
 	template <class T>
 	void error(const T& obj) { 
 		log_obj(LEVEL_ERROR, obj);
 	}
 	template <class T>
 	void fatal(const T& obj) { 
 		log_obj(LEVEL_FATAL, obj);
 	}
 	
 	// specialization for strings
 	void debug(const std::string& s) {
 		log_cstr(LEVEL_DEBUG, s.c_str());
  	} 
   	void trace(const std::string& s) {
 		log_cstr(LEVEL_TRACE, s.c_str());
  	} 
   	void warn(const std::string& s) {
 		log_cstr(LEVEL_WARN, s.c_str());
  	} 
   	void error(const std::string& s) {
 		log_cstr(LEVEL_ERROR, s.c_str());
  	} 
   	void fatal(const std::string& s) {
 		log_cstr(LEVEL_FATAL, s.c_str());
  	} 
	
	// core logging function
	void log_cstr(int level, const char *buf);
	
	// Logs any object defining the operator<<
	template <class T>
	void log_obj(int level, const T& obj);

	// helper logging function
	void log_va_list(int level, const char *format, va_list argList);
	
	// Is output for this level suppressed?
	bool suppressed(int level);
	
	// config
	void set_level(int level); 
	void set_ostream(std::ostream* pos); 
	
	// Allow ostream operations for loggers.
	// Currently operator<< outputs at trace level.
	// We could provide a manipulator for selecting the level.
	template<class T>
	logger& operator<<(const T& v) {
		m_tr_oss << v;
		return *this;
	}
	logger& operator<<(const char *s) {
		m_tr_oss << s;
		return *this;
	}
	logger& operator<<(logger& (*f)(logger&) ) {
		return f(*this);
	}
	void trace_oss_endl() { 
		trace(m_tr_oss.str());
		m_tr_oss.str("");
	}
	
  private:
	static const char* get_level_name(int level);
	
	// this logger members
	critical_section m_cs;
	std::string m_name;	
	std::ostream* m_pos;
	int m_level;
	std::ostringstream m_tr_oss;
	
	// configuration and output format
	static int default_level;
	static std::ostream* default_pos; 
	static bool logdate;
	static bool logtime;
	static bool logname;
	static bool loglevel;
	
};

//////////////////////////////
// Inline part of the implementation

inline logger::logger(const std::string& name) 
:	m_name(name),
	m_pos(logger::default_pos),
	m_level(logger::default_level) {
}

template <class T>
inline void logger::log_obj(int level, const T& obj) {
	if(suppressed(level))
		return;
	std::ostringstream os;
	os << obj << std::endl;
	log_cstr(level, os.str().c_str());
}

inline void logger::set_level(int level) { 
	m_level = level; 
}

inline void logger::set_ostream(std::ostream* pos) { 
	m_pos = pos; 
}

inline bool logger::suppressed(int level) {
	return level < m_level;
}

inline logger& __cdecl endl(logger& l) {
	l.trace_oss_endl();
	return l;
}	


} // namespace lib

} // namespace ambulant


#endif // 
