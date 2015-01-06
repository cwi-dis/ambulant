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

#ifndef AMBULANT_LIB_LOGGER_H
#define AMBULANT_LIB_LOGGER_H

#include "ambulant/config/config.h"

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/amstream.h"

// Define the next to enable logging how video packets move through the
// program, if AMBULANT_LOGFILE_LATENCY environment points to an output file.
// #define LOGGER_VIDEOLATENCY "videolatency"

namespace ambulant {

namespace lib {

typedef void (*show_message_type)(int level, const char *message);

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
class AMBULANTAPI logger {
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

	/// Constructor.
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

	/// templates for objects defining the operator<<
	template <class T>
	void debug(const T& obj) {
		log_obj(LEVEL_DEBUG, obj);
	}

	/// templates for objects defining the operator<<
	template <class T>
	void trace(const T& obj) {
		log_obj(LEVEL_TRACE, obj);
	}

	/// templates for objects defining the operator<<
	template <class T>
	void warn(const T& obj) {
		log_obj(LEVEL_WARN, obj);
	}

	/// templates for objects defining the operator<<
	template <class T>
	void error(const T& obj) {
		log_obj(LEVEL_ERROR, obj);
	}

	/// templates for objects defining the operator<<
	template <class T>
	void fatal(const T& obj) {
		log_obj(LEVEL_FATAL, obj);
	}

	/// templates for objects defining the operator<<
	template <class T>
	void show(const T& obj) {
		log_obj(LEVEL_SHOW, obj);
	}

	/// specialization for strings.
	void debug(const std::string& s) {
		log_cstr(LEVEL_DEBUG, s.c_str());
	}

	/// specialization for strings.
	void trace(const std::string& s) {
		log_cstr(LEVEL_TRACE, s.c_str());
	}

	/// specialization for strings.
	void show(const std::string& s) {
		log_cstr(LEVEL_SHOW, s.c_str());
	}

	/// specialization for strings.
	void warn(const std::string& s) {
		log_cstr(LEVEL_WARN, s.c_str());
	}

	/// specialization for strings.
	void error(const std::string& s) {
		log_cstr(LEVEL_ERROR, s.c_str());
	}

	/// specialization for strings.
	void fatal(const std::string& s) {
		log_cstr(LEVEL_FATAL, s.c_str());
	}

	/// core logging function.
	void log_cstr(int level, const char *buf);

	/// Logs any object defining the operator<<
	template <class T>
	void log_obj(int level, const T& obj);

	/// helper logging function, printf-like.
	void log_va_list(int level, const char *format, va_list argList);

	/// Is output for this level suppressed?
	bool suppressed(int level);

	/// Suppress output below the given level.
	void set_level(int level);

// exclude the following stuff when no streams
	/// Send log output to a std::stream.
	void set_std_ostream(std::ostream& os);

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	std::string m_name;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
	return false;
}

} // namespace lib

} // namespace ambulant

#define LOGGER_ASSERT(exp) if(!(exp)) ambulant::lib::logger::assert_expr(exp, #exp)


#endif //
