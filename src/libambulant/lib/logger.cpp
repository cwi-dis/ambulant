/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/logger.h"
#include "ambulant/lib/mtsync.h"

////////////////////////////
// module private part

using namespace ambulant;

// Manages init/free app logger
class logger_wrapper {
  public:
  typedef lib::stdout_logger<lib::critical_section> stdout_logger;
  
	logger_wrapper() :	m_logger(new stdout_logger()){}
	~logger_wrapper()
		{ if(m_logger != 0) delete m_logger;}
		
	lib::logger* get_logger() { return m_logger;}
	void set_logger(lib::logger *p) {
		if(m_logger != 0) delete m_logger; 
		m_logger = p;
	}

	lib::logger* operator->() { return m_logger;}

  private:
	lib::logger *m_logger;
};

static logger_wrapper wrapper;


////////////////////////////
// public logging functions

void ambulant::set_app_logger(lib::logger *p) {
	wrapper.set_logger(p);
}


void ambulant::log_error_event(const char *format, ...) {
	if(wrapper.get_logger() == 0) return;
	char buf[2048] = "";
	va_list	pArg;
	va_start(pArg, format);
	vsprintf(buf, format, pArg);
	va_end(pArg);
	wrapper->log_error_event(buf);
}

void ambulant::log_warning_event(const char *format, ...) {
	if(wrapper.get_logger() == 0) return;
	char buf[2048] = "";
	va_list	pArg;
	va_start(pArg, format);
	vsprintf(buf, format, pArg);
	va_end(pArg);
	wrapper->log_warning_event(buf);
}

void ambulant::log_trace_event(const char *format, ...) {
	if(wrapper.get_logger() == 0) return;
	char buf[2048] = "";
	va_list	pArg;
	va_start(pArg, format);
	vsprintf(buf, format, pArg);
	va_end(pArg);
	wrapper->log_trace_event(buf);
}

void ambulant::log_trace_event(const std::string& str) {
	wrapper->log_trace_event(str.c_str());
}

