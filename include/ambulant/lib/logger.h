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

#include "ambulant/lib/mtsync.h"

namespace ambulant {

// public ambulant logging functions
extern void log_error_event(const char *format, ...);
extern void log_warning_event(const char *format, ...);
extern void log_trace_event(const char *format, ...);

template<class T>
void log_trace_event(const T& obj);

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
	
  private:
	static const char* get_level_name(int level);
	
	// this logger members
	critical_section m_cs;
	std::string m_name;	
	std::ostream* m_pos;
	int m_level;
	
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


} // namespace lib

} // namespace ambulant

#endif // 
