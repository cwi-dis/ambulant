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

#include "ambulant/lib/logger.h"

#include <map>

#include <time.h>
#include <stdarg.h>

using namespace ambulant;

// Options:
// One global logger configured appropriately.
// A set of named global loggers configured appropriately.
// A logger per file as: lib::logger::get_logger(__FILE__);


// Set default_level to a higher level than debug 
// to suppress output at levels below

// static
int lib::logger::default_level = lib::logger::LEVEL_DEBUG;


// Set the fefault loggers ostream
// Set this std::cout, to a file or socket ostream or whatever
// This is the default, each specific named logger may 
// be configured sepaately

// static
std::ostream* lib::logger::default_pos = &std::cout;

// Output logging date as xx/xx/xx
//static 
bool lib::logger::logdate = false;

// Output logging time as xx:xx:xx
//static 
bool lib::logger::logtime = true;

// Output logger name as [name]
// static 
bool lib::logger::logname = false;

// Output level name 
// Level name is one of (DEBUG, TRACE, WARN, ERROR, FATAL, UNKNOWN)
// static 
bool lib::logger::loglevel = true;

////////////////////////////////
// Module private bracket

// Manages init/free app logger
class loggers_manager {
  public:
	~loggers_manager();

	// loggers repository
	std::map<std::string, lib::logger*> loggers;	
};

// free all loggers
// loggers should no be used in the destructors
// of objects that will be detroyed after the appl 'exits'.
loggers_manager::~loggers_manager() {
	std::map<std::string, lib::logger*>::iterator it;
	for(it=loggers.begin();it!=loggers.end();it++)
		delete (*it).second;
}

static 
loggers_manager loggers_singleton;

////////////////////////////////

const std::string app_logger_name = "app_logger";

// one usage: lib::logger::get_logger(__FILE__);
// if used this way we should remove critical section
//static 
lib::logger* lib::logger::get_logger(const char *name) {
	std::string lname = (name==NULL || name[0]==0)?app_logger_name:name;
	std::map<std::string, logger*>::iterator it;
	it = loggers_singleton.loggers.find(lname);
	if(it != loggers_singleton.loggers.end()) {
		return (*it).second;
	}
	logger *nl = new logger(lname);
	loggers_singleton.loggers[lname] = nl;
	return nl;
}

// one usage: lib::logger::get_logger(__FILE__, __LINE__);
// if used this way we should remove critical section
//static 
lib::logger* lib::logger::get_logger(const char *name, int pos) {
	std::string lname = (name==NULL || name[0]==0)?app_logger_name:name;
	char sz[16];sprintf(sz, ":%d", pos);
	lname += sz;
	return get_logger(lname.c_str());
}

// static 
void lib::logger::set_loggers_level(int level) {
	logger::default_level = level;
}

void lib::logger::debug(const char *format, ...) {
	if(suppressed(LEVEL_DEBUG))
		return;
	va_list	args;
	va_start(args, format);
	log_va_list(LEVEL_DEBUG, format, args);
	va_end(args);
}

void lib::logger::trace(const char *format, ...) {
	if(suppressed(LEVEL_TRACE))
		return;
	va_list	args;
	va_start(args, format);
	log_va_list(LEVEL_TRACE, format, args);
	va_end(args);
}

void lib::logger::warn(const char *format, ...) {
	if(suppressed(LEVEL_WARN))
		return;
	va_list	args;
	va_start(args, format);
	log_va_list(LEVEL_WARN, format, args);
	va_end(args);
}

void lib::logger::error(const char *format, ...) {
	if(suppressed(LEVEL_ERROR))
		return;
	va_list	args;
	va_start(args, format);
	log_va_list(LEVEL_ERROR, format, args);
	va_end(args);
}

void lib::logger::fatal(const char *format, ...) {
	if(suppressed(LEVEL_FATAL))
		return;
	va_list	args;
	va_start(args, format);
	log_va_list(LEVEL_FATAL, format, args);
	va_end(args);
	abort();
}

void lib::logger::log_va_list(int level, const char *format, va_list args) {
	char buf[2048] = "";
	vsprintf(buf, format, args);
	log_cstr(level, buf);
}

// Output format/hook
void lib::logger::log_cstr(int level, const char *buf) {
	if(suppressed(level))
		return;
	struct tm *lt = NULL;
	if(logger::logdate || logger::logtime) {
		time_t t = time(NULL);
		lt = localtime(&t);
	}
	std::ostream& os = *m_pos;
	m_cs.enter();
	if(logger::logdate)
		os << (1900 + lt->tm_year) << "/" << (1 + lt->tm_mon) << "/" << lt->tm_mday << " ";
	if(logger::logtime)
		os << lt->tm_hour << ":" << lt->tm_min << ":" << lt->tm_sec << " ";
	if(loglevel)
		os << get_level_name(level) << "\t";
		
	os << buf;
	
	if(logger::logname)
		os << " [" <<  m_name << "]" ;
		
	os << std::endl;
	os.flush();
	m_cs.leave();
}

// static
const char* 
lib::logger::get_level_name(int level) {
	switch(level) {
		case LEVEL_DEBUG: return "DEBUG";
		case LEVEL_TRACE: return "TRACE";
		case LEVEL_WARN: return "WARN";
		case LEVEL_ERROR: return "ERROR";
		case LEVEL_FATAL: return "FATAL";
		default: return "UNKNOWN";
	}
}



