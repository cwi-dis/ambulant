
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

// Ambulant standard base (ASB) compatibility interface

#ifndef AMBULANT_LIB_WIN32_ASB_H
#define AMBULANT_LIB_WIN32_ASB_H

#include <string>

namespace ambulant {

namespace lib {

namespace win32 {

void sleep(unsigned long secs);
void sleep_msec(unsigned long msecs);

std::string getcwd();
std::string resolve_path(const char *s);

} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_ASB_H