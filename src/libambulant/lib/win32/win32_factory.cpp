
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/win32/win32_event_processor.h"
#include "ambulant/lib/win32/win32_timer.h"

using namespace ambulant;

// Machine-dependent factory function
lib::event_processor*
lib::event_processor_factory() {
	return new lib::win32::event_processor();
}

// Factory routine for the machine-independent
// timer class
lib::timer *
lib::timer_factory() {
	return new lib::win32::os_timer();
}

