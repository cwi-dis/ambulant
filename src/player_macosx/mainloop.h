#include "ambulant.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/player.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"

class mainloop_callback_arg {
};

class mainloop : public ambulant::lib::ref_counted {
  public:
	mainloop()
	:	m_refcount(1),
		m_done(false) {}
	
	// The callback member function.
	void player_done_callback(mainloop_callback_arg *p) {
		m_done = true;
	}
	
	void run(char *filename);
	////////////////////////
	// lib::ref_counted interface implementation
	
	long add_ref() {return ++m_refcount;}

	long release() {
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}
	
  private:
  	bool m_done;
	ambulant::lib::basic_atomic_count<ambulant::lib::critical_section> m_refcount;
};
