
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_MTSYNC_H
#define AMBULANT_LIB_MTSYNC_H

namespace ambulant {

namespace lib {

class critical_section {
  public:
	virtual ~critical_section() {}
	
	virtual void enter() = 0;
	virtual void leave() = 0;
};

// Machine-dependent factory function
critical_section *critical_section_factory();

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_MTSYNC_H
