/* 
 * @$Id$ 
 */
#ifndef __QT_MAINLOOP_H__
#define __QT_MAINLOOP_H__

// Environment for testing design classes

#include <iostream>
#include <ambulant/version.h>
#include <ambulant/lib/logger.h>
#include <ambulant/common/player.h>
#include <ambulant/lib/event_processor.h>
#include <ambulant/lib/asb.h>
#include <qt_gui.h>
#include "ambulant/gui/none/none_gui.h"
#include <qt_renderer.h>

class qt_mainloop_callback_arg {
};
class qt_mainloop : public ambulant::lib::ref_counted {
//  	static bool m_done;
  public:
	qt_mainloop(qt_gui* parent)
	:	m_refcount(1),
	  m_parent(parent) {}
//	m_done(false),
	
	// The callback member function.
	void player_done_callback(qt_mainloop_callback_arg *p) {
//		m_done = true;
		m_parent->player_done();
	}
	
	static void* run(void* qt_gui);
	long add_ref() {return ++m_refcount;}

	long release() {
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}
	
//	static bool done() {return m_done;}
  private:
//  	bool m_done;
	qt_gui* m_parent;
	ambulant::lib::basic_atomic_count<ambulant::lib::critical_section> m_refcount;
};
#endif/*__QT_MAINLOOP_H__*/
