
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

// Ambulant standard base (ASB) compatibility implementations

#include "ambulant/lib/win32/win32_asb.h"

#include <windows.h>

using namespace ambulant;

void lib::win32::sleep(unsigned long secs) {
	Sleep(1000*secs);
}