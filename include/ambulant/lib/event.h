
/* 
 * @$Id$ 
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

class flag_event : public event {
  public:
	flag_event(bool& flag)
	:	m_flag(flag) {}
	
	void fire() {
		m_flag = !m_flag;
	}
  public:
	bool& m_flag;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_EVENT_H
