
/* 
 * @$Id$ 
 */

// Ambulant standard base (ASB) compatibility implementations

#include "ambulant/lib/win32/win32_asb.h"

#include <string>

#include <windows.h>

using namespace ambulant;

void lib::win32::sleep(unsigned long secs) {
	Sleep(1000*secs);
}

void lib::win32::sleep_msec(unsigned long msecs) {
	Sleep(msecs);
}

std::string lib::win32::getcwd() {
	char buf[MAX_PATH];
	char *pFilePart = 0;	
	GetFullPathName(".", MAX_PATH, buf, &pFilePart);
	return buf;
}

std::string lib::win32::resolve_path(const char *s) {
	char buf[MAX_PATH];
	char *pFilePart = 0;	
	GetFullPathName(s, MAX_PATH, buf, &pFilePart);
	return buf;
}

