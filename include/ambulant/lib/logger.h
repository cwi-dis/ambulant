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



#ifndef AMBULANT_LIB_LOGGER_H
#define AMBULANT_LIB_LOGGER_H

#include "ambulant/config/config.h"

////////////////////////
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS

#ifndef AMBULANT_NO_IOSTREAMS
#include <iostream>
#	ifndef AMBULANT_NO_STRINGSTREAM
#	include <sstream>
#	endif
#else
#include <ostream.h>
#endif

#endif // AMBULANT_NO_IOSTREAMS_HEADERS
////////////////////////

#include <string>
#include <stdarg.h>
#include <stdio.h>

#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/amstream.h"

namespace ambulant {

namespace lib {

typedef void (*show_message_type)(const char *format, ...);

/// Logging message handler.
/// Normal use if this class is through logger::get_logger()->show()
/// and friends, this will send a message with the given level to the user.
/// All messages below the level set with set_level() are ignored, the
/// messages at higher level are sent either to a log file or a log window.
/// In addition, messages of level SHOW, ERROR and FATAL are also passed
/// to the function set with set_show_message(), which is intended to 
/// produce an alert dialog or something similar.
///
/// An embedding application can choose where log output goes by calling
/// set_std_ostream() to send messages to a std::stream, or set_ostream()
/// to send them to an object implementing the lib::ostream protocol.
class logger {
  public:
	/// Log levels, least important first.
	enum log_level { 
		LEVEL_DEBUG,	///< Message for developers only
		LEVEL_TRACE,	///< Detailed informative message
		LEVEL_SHOW,		///< Informative message, shown to user in a dialog
		LEVEL_WARN,		///< Warning message, operations will continue
		LEVEL_ERROR,	///< Error message, operations will terminate
		LEVEL_FATAL		///< Fatal error, program will terminate
	};
		
	logger(const std::string& name);
	~logger();
	
	/// static factory function to create or get a logger.
	static logger* get_logger(const char *name = NULL);

	/// static factory function to create or get a logger.
	static logger* get_logger(const char *name, int pos);
	
	/// set minimum level for all loggers.
	static void set_loggers_level(int level);
	
	/// log a message at level DEBUG.
 	void debug(const char *format, ...);
	
	/// log a message at level TRACE.
 	void trace(const char *format, ...);
	
	/// log a message at level SHOW.
  	void show(const char *format, ...);
	
	/// log a message at level WARN.
  	void warn(const char *format, ...);
	
	/// log a message at level ERROR.
  	void error(const char *format, ...);
	
	/// log a message at level FATAL.
  	void fatal(const char *format, ...);
	
	/// log a message at level DEBUG.
  	static void assert_expr(bool expr, const char *format, ...);
	
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
  	template <class T>
 	void show(const T& obj) { 
 		log_obj(LEVEL_SHOW, obj);
 	}
	
 	// specialization for strings
 	void debug(const std::string& s) {
 		log_cstr(LEVEL_DEBUG, s.c_str());
  	} 
   	void trace(const std::string& s) {
 		log_cstr(LEVEL_TRACE, s.c_str());
  	} 
   	void show(const std::string& s) {
 		log_cstr(LEVEL_SHOW, s.c_str());
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
	
	/// core logging function.
	void log_cstr(int level, const char *buf);
	
	/// Logs any object defining the operator<<
	template <class T>
	void log_obj(int level, const T& obj);

	// helper logging function
	void log_va_list(int level, const char *format, va_list argList);
	
	/// Is output for this level suppressed?
	bool suppressed(int level);
	
	// Suppress output below the given level.
	void set_level(int level); 

// exclude the following stuff when no streams
#ifndef AMBULANT_NO_IOSTREAMS
	/// Send log output to a std::stream.
	void set_std_ostream(std::ostream& os); 
#endif
	/// Set the alert message handler.
	void set_show_message(show_message_type handler);
	
	/// Send log output to an object implementing the ostream protocol.
	/// This becomes the owner of pos,
	/// when this is deleted will delete pos.
	void set_ostream(ostream* pos) {
		if(m_pos) delete m_pos;
		m_pos = pos;
	} 
	
	/// Get the name for a given level.
	static const char* get_level_name(int level);
  private:
	
	// this logger members
	critical_section m_cs;
	std::string m_name;	
	ostream* m_pos;
	show_message_type m_show_message;
	int m_level;
	
	// configuration and output format
	static int default_level;
	static bool logdate;
	static bool logtime;
	static bool logname;
	static bool loglevel;
	
};



//////////////////////////////
// Inline part of the implementation

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

inline bool logger::suppressed(int level) {
	return level < m_level;
}

#ifdef AMBULANT_PLATFORM_WIN32

// A machine-dependent function to show a popup message
extern void show_message(const char *format, ...);

#else 

// dummy for rest of platforms
inline void show_message(const char *format, ...) {}

#endif

} // namespace lib

} // namespace ambulant

#define LOGGER_ASSERT(exp) if(!(exp)) ambulant::lib::logger::assert_expr(exp, #exp)


#endif //
