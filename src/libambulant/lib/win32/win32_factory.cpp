
/* 
 * @$Id$ 
 */

#include "ambulant/lib/win32/win32_event_processor.h"

using namespace ambulant;

// Machine-dependent factory function
lib::event_processor *
lib::event_processor_factory(abstract_timer *t) {
	return new lib::win32::event_processor(t);
}


