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

#ifndef _IOSTREAM_
#include <iostream>
#endif

#include <string>

namespace ambulant {

// forward declaration
namespace lib { class logger;}

// public ambulant logging functions
extern void set_app_logger(lib::logger *p);
extern void log_error_event(const char *format, ...);
extern void log_warning_event(const char *format, ...);
extern void log_trace_event(const char *format, ...);

extern void log_trace_event(const std::string& str);

namespace lib {

// simple logger interface (no message ids, categories etc)
class logger
	{
	public:
	virtual ~logger(){}
	virtual void log_error_event(const char *sz) = 0;
	virtual void log_warning_event(const char *sz) = 0;
	virtual void log_trace_event(const char *sz) = 0;
	};

// a stdout simple logger implementation
template <class T>
class stdout_logger : public logger {
  public:
	virtual void log_error_event(const char *buf) { log(buf);}
	virtual void log_warning_event(const char *buf) { log(buf);}
	virtual void log_trace_event(const char *buf) { log(buf);}
	
  private:
	void log(const char *sz) {
		m_cs.enter();
		std::cout << sz << std::endl;
		m_cs.leave();
	}
	T m_cs;
};

// an ofstrean simple logger implementation
template <class T>
class ofstream_logger : public logger {
	public:
	ofstream_logger(const char* szName, int nMode = std::ios::out) 
	:	m_ofs(szName, nMode) {}
	
	~ofstream_logger() { m_ofs.close();}

	virtual void log_error_event(const char *buf) { log(buf);}
	virtual void log_warning_event(const char *buf) { log(buf);}
	virtual void log_trace_event(const char *buf) { log(buf);}
	
	private:
	void log(const char *sz) {
		m_cs.enter();
		m_ofs << sz << std::endl;
		m_cs.leave();
	}
	T m_cs;
	std::ofstream m_ofs;
};

} // namespace lib

} // namespace ambulant

#endif // 
