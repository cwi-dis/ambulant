
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_ABSTRACT_MTSYNC_H
#define AMBULANT_LIB_ABSTRACT_MTSYNC_H

namespace ambulant {

namespace lib {

class abstract_critical_section {
  public:
	virtual ~abstract_critical_section() {}
	
	virtual void enter() = 0;
	virtual void leave() = 0;
};
} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_ABSTRACT_MTSYNC_H
