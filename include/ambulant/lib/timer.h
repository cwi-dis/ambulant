
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

#include <set>

namespace ambulant {

namespace lib {

class abstract_timer_client {
  public:
	// this timer time type (assumed in msecs)
	typedef std::set<abstract_timer_client *> client_set;
	typedef client_set::iterator client_index;
	
	virtual ~abstract_timer_client() {}

	virtual void speed_changed();
	// A child clock can call add_dependent to have calls that change
	// timing (such as set_speed) forwarded to it
	client_index add_dependent(abstract_timer_client *child);
	void remove_dependent(client_index pos);
	void remove_dependent(abstract_timer_client *child);
	
  protected:
    client_set m_dependents;
};

class abstract_timer : public abstract_timer_client {
  public:
	// this timer time type (assumed in msecs)
	typedef unsigned long time_type;
	
	virtual ~abstract_timer() {}
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	virtual time_type elapsed() const = 0;
	virtual void set_speed(double speed) = 0;
	virtual double get_realtime_speed() const = 0;
};

// Note: timer objects are not refcounted, because it is assumed that
// a parent timer will always automatically live longer than a child
// timer. If this does not hold in future then we should add refcounting.
class timer : public abstract_timer {
  public:
	timer(abstract_timer *parent, double speed);
	timer(abstract_timer *parent);
	~timer();
//	void add_dependent(timer& child);
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	time_type elapsed() const;
	void set_speed(double speed);
	double lib::timer::get_realtime_speed() const;
  private:
	void re_epoch();
	
	abstract_timer *m_parent;
//	std::vector<timer&> m_children;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
};

// A (machine-dependent) routine to create a timer object
abstract_timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


