
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

template <typename time_type = unsigned long>
class timeout_event : public event {
  public:
	typedef time_type self_time_type;
	
	// Returns the relative time this event is scheduled to fire.
	virtual self_time_type get_time() = 0;
	
	// Incr/decr relative schedule time.
	virtual void incr(self_time_type dt) = 0;
	virtual void decr(self_time_type dt) = 0;
	
	// Sets the relative time this event is scheduled to fire.
	virtual void set_timeout(self_time_type t) = 0;
};
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_EVENT_H
