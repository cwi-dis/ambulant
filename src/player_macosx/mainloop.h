#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/player.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

class mainloop : public ambulant::lib::ref_counted_obj {
  public:
	mainloop()
	:   m_done(false) {}
	
	// The callback member function.
	void player_done_callback() {
		m_done = true;
	}
	
	void run(const char *filename, ambulant::lib::window_factory *wf);
	
  private:
  	bool m_done;
};
