/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
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



