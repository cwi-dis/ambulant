/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_WIN32_ERROR_H
#define AMBULANT_LIB_WIN32_ERROR_H

namespace ambulant {

namespace lib {

namespace win32 {

void win_report_error(const char *func, unsigned long  err); 

void win_report_last_error(const char *func);


} // namespace win32

} // namespace lib

} // namespace ambulant 

#endif // AMBULANT_LIB_WIN32_ERROR_H
