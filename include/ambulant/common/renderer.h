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

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/playable.h"

namespace ambulant {

namespace common {


class active_playable : public playable {
  public:
    active_playable(playable_notification *context, const cookie_type cookie)
    :   m_context(context),
        m_cookie(cookie) {};
    virtual ~active_playable() {};

	virtual void seek(double t) {};
	virtual void preroll(double when, double where, double how_much) {};
	virtual std::pair<bool, double> get_dur() { return std::pair<bool, double>(false, 0); };
	const cookie_type& get_cookie() const { return m_cookie;}
	
  protected:
    void started_callback() const { m_context->started(m_cookie, 0); };
    void stopped_callback() const { m_context->stopped(m_cookie, 0); };
    void clicked_callback() const { m_context->clicked(m_cookie, 0); };
    
    playable_notification *const m_context;
    cookie_type m_cookie;
};

class active_basic_renderer : public active_playable, public renderer {
  public:
#if 0
  	active_basic_renderer()
  	:	active_playable((playable_notification *)NULL, 0),
		m_node(NULL),
		m_event_processor(NULL) {};
#endif
	active_basic_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp)
	:   active_playable(context, cookie),
		m_node(node),
		m_event_processor(evp) {};
		
	~active_basic_renderer() {};
	
  protected:
	const lib::node *m_node;
	lib::event_processor *const m_event_processor;
};

class active_renderer : public active_basic_renderer {
  public:
	active_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
		
	~active_renderer() {}
	
	virtual void start(double where);
	virtual void freeze() {}
	virtual void stop();
	virtual void pause() {}
	virtual void resume() {}
	virtual void wantclicks(bool want);

	virtual void redraw(const lib::screen_rect<int> &dirty, abstract_window *window) = 0;
	virtual void user_event(const lib::point &where) { clicked_callback(); }
	virtual void set_surface(surface *dest) { m_dest = dest; }
	virtual surface *get_surface() { return m_dest;}
	virtual renderer *get_renderer() { return this; }
	virtual void readdone() {};
	
  protected:

  	net::datasource *m_src;
	surface *m_dest;
//	lib::event *m_readdone;
};

// active_final_renderer is a handy subclass of active_renderer:
// it waits until all data is available, reads it, and then calls
// need_redraw on the region. If you subclass this you only
// need to add a redraw method.
class active_final_renderer : public active_renderer {
  public:
	active_final_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
	:	active_renderer(context, cookie, node, evp),
		m_data(NULL),
		m_data_size(0) {};
	virtual ~active_final_renderer();
	
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

// background_renderer is a convenience class: it implements some of the
// methods for a renderer that are applicable to background renderers
class background_renderer : public renderer {
  public:
	background_renderer(const region_info *src)
	:   m_src(src),
		m_dst(NULL) {}
	virtual ~background_renderer() {}
	virtual void set_surface(surface *destination) { m_dst = destination; }
	virtual void user_event(const lib::point &where) { /* Ignore, for now */ }
	virtual surface *get_surface() { return m_dst; }
  protected:
	const region_info *m_src;
	surface *m_dst;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_RENDERER_H
