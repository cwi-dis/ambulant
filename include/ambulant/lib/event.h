
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_EVENT_H
#define AMBULANT_LIB_EVENT_H

namespace ambulant {

namespace lib {

class event {
  public:
	virtual ~event() {}
	virtual void fire() = 0;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_EVENT_H
