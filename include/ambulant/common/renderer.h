/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_COMMON_RENDERER_H
#define AMBULANT_COMMON_RENDERER_H

#include "ambulant/config/config.h"
#include "ambulant/common/factory.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/net/datasource.h"
//XXX The raw_video_datasource should not be here, should be fixed with the factories.
#include "ambulant/net/raw_video_datasource.h"
#include "ambulant/common/playable.h"

namespace ambulant {

namespace common {

	class global_playable_factory;
		
// A helper class implementing some of the common code for playables
class playable_imp : public playable {
  public:
	playable_imp(
		playable_notification *context,
		cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp) 
	:	m_context(context), 
		m_cookie(cookie),
		m_node(node),
		m_event_processor(evp) {
	}

	// common::playable interface
	void pause() {}
	void resume() {}
	void seek(double where) {}
	void wantclicks(bool want) { m_wantclicks = want;}
	void preroll(double when, double where, double how_much) {}
	std::pair<bool, double> get_dur() { return std::pair<bool, double>(true, 0);}
	const cookie_type& get_cookie() const { return m_cookie;}
  protected:
    playable_notification *m_context;
    cookie_type m_cookie;
	const lib::node	*m_node;
	lib::event_processor *m_event_processor;
	bool m_wantclicks;
};

// An alt inline direct base for renderer playables.
class renderer_playable : public playable_imp, public renderer {
  public:
	renderer_playable(
		playable_notification *context,
		cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp) 
	:	playable_imp(context, cookie, node, evp),
		m_dest(0),
		m_alignment(0),
		m_activated(false) {
	}
			
	// common::renderer interface
	void set_surface(common::surface *dest) { m_dest = dest;}
	void set_alignment(common::alignment *align) { m_alignment = align; }
	void set_intransition(lib::transition_info *info) {  }
	void start_outtransition(lib::transition_info *info) {  }
	surface *get_surface() { return m_dest;}
	void user_event(const lib::point &where, int what) {}
	renderer *get_renderer() { return this; }
	void transition_freeze_end(lib::screen_rect<int> r) { m_context->transitioned(m_cookie); }
	
  protected:
	surface *m_dest;
	alignment *m_alignment;
	bool m_activated;
};


class renderer_playable_ds : public renderer_playable {
  public:
	renderer_playable_ds(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		common::factories* factory);
		
	virtual ~renderer_playable_ds();
	
	virtual void start(double where);
//	virtual void freeze() {}
	virtual void stop();
//	virtual void pause() {}
//	virtual void resume() {}
//	virtual void wantclicks(bool want);

	virtual void redraw(const lib::screen_rect<int> &dirty, gui_window *window) = 0;
	void user_event(const lib::point &where, int what = 0) {
		if (what == user_event_click) m_context->clicked(m_cookie, 0);
		else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
		else assert(0);
	}
	
	virtual void readdone() = 0;
  protected:

  	net::datasource *m_src;
};

// renderer_playable_dsall is a handy subclass of renderer_playable_ds:
// it waits until all data is available, reads it, and then calls
// need_redraw on the region. If you subclass this you only
// need to add a redraw method.
class renderer_playable_dsall : public renderer_playable_ds {
  public:
	renderer_playable_dsall(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		common::factories *factory)
	:	renderer_playable_ds(context, cookie, node, evp, factory),
		m_data(NULL),
		m_data_size(0) {};
	virtual ~renderer_playable_dsall();
	
  protected:
	void readdone();
	void *m_data;
	unsigned m_data_size;
};

class global_playable_factory : public playable_factory {
  public:
    global_playable_factory();
    ~global_playable_factory();
    
    void add_factory(playable_factory *rf);
    
    playable *new_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
  private:
    std::vector<playable_factory *> m_factories;
    playable_factory *m_default_factory;
};

class empty_playable_notification : public playable_notification {
	public:
	// Playables nodifications 
	  void started(cookie_type n, double t = 0)  {};
	  void stopped(cookie_type n, double t = 0)  {};
	  void stalled(cookie_type n, double t = 0)  {} ;
	  void unstalled(cookie_type n, double t = 0) {};
	  void clicked(cookie_type n, double t = 0)  {};
	  void pointed(cookie_type n, double t = 0)  {}; // mouse over
	  void transitioned(cookie_type n, double t = 0) {};
};

class active_video_renderer : public common::renderer_playable {
  public:
	active_video_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories *factory);

  	virtual ~active_video_renderer() {};
	
      
  	bool is_paused() { return m_is_paused; };
  	bool is_stopped() { return !m_is_playing;};
  	bool is_playing() { return m_is_playing; };  
	
	virtual void show_frame(char* frame, int size) {};
    virtual void redraw(const lib::screen_rect<int> &dirty, common::gui_window *window);
	
	void start(double where);
    void stop();
    void pause();
    void resume();
    void data_avail();
//	void playdone() {};
	
		
  protected:
	lib::size m_size;
  	net::video_datasource* m_src; 
  	net::audio_datasource *m_audio_ds;
  	common::playable *m_audio_renderer;
  	empty_playable_notification m_playable_notification;
  private:
	  typedef lib::no_arg_callback <active_video_renderer > dataavail_callback;
	  double now();
 	  unsigned long int m_epoch;
	  bool m_is_playing;
	  bool m_is_paused;
	  unsigned long int m_paused_epoch;
	  lib::critical_section m_lock;
};



// background_renderer is a convenience class: it implements some of the
// methods for a renderer that are applicable to background renderers
class background_renderer : public bgrenderer {
  public:
	background_renderer(const region_info *src)
	:   m_src(src),
		m_dst(NULL) {}
	virtual ~background_renderer() {}
	void set_surface(surface *destination) { m_dst = destination; }
	void user_event(const lib::point &where, int what = 0) {};
	void transition_freeze_end(lib::screen_rect<int> area) {};
  protected:
	const region_info *m_src;
	surface *m_dst;
};


} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_RENDERER_H
