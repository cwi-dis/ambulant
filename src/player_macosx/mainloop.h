#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

class mainloop : public ambulant::lib::ref_counted_obj {
  public:
	mainloop()
	:   m_running(false),
		m_speed(1.0),
		m_active_player(NULL) {}
	
	// The callback member function.
	void player_done_callback() {
		m_running = false;
	}
	
	void run(const char *filename, ambulant::lib::window_factory *wf);
	void set_speed(double speed);
	double get_speed() const { return m_speed; };
	bool is_running() const { return m_running; };
	
  private:
  	bool m_running;
	double m_speed;
	ambulant::lib::active_player *m_active_player;
};
