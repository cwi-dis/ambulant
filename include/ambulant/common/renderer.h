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

#ifndef AMBULANT_LIB_RENDERER_H
#define AMBULANT_LIB_RENDERER_H

#include "ambulant/config/config.h"

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/layout.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/playable.h"

namespace ambulant {

namespace lib {

class active_basic_renderer : public active_playable {
  public:
  	active_basic_renderer()
  	:	active_playable((active_playable_events *)NULL, 0),
		m_node(NULL),
		m_event_processor(NULL) {};
//  	active_basic_renderer(const ambulant::lib::active_basic_renderer& src)
//  	:	m_event_processor(src.m_event_processor),
//  		m_node(src.m_node),
//  		m_playdone(src.m_playdone) {}
	active_basic_renderer(
		active_playable_events *context,
		active_playable_events::cookie_type cookie,
		const node *node,
		event_processor *const evp)
	:   active_playable(context, cookie),
		m_node(node),
		m_event_processor(evp) {};
		
	~active_basic_renderer() {};
	
  protected:
	const node *m_node;
  	event_processor *const m_event_processor;
};

;

class active_renderer : public active_basic_renderer, public abstract_rendering_source, public ref_counted_obj {
  public:
  	active_renderer()
  	:	active_basic_renderer(NULL, 0, NULL, NULL),
  		m_src(NULL),
  		m_dest(0),
  		m_readdone(NULL) {}
  	active_renderer(const ambulant::lib::active_renderer& src)
  	:	active_basic_renderer(src.m_context, src.m_cookie, src.m_node, src.m_event_processor),
  		m_dest(0),
  		m_readdone(src.m_readdone) {}
	active_renderer(
		active_playable_events *context,
		active_playable_events::cookie_type cookie,
		const node *node,
		event_processor *const evp,
		net::passive_datasource *src,
		abstract_rendering_surface *const dest);
		
	~active_renderer() {}
	
	virtual void start(double where);
	virtual void freeze() {}
	virtual void stop();
	virtual void pause() {}
	virtual void resume() {}

	virtual void redraw(const screen_rect<int> &dirty, abstract_window *window) = 0;
	
  protected:
	virtual void readdone();

  	net::active_datasource *m_src;
	abstract_rendering_surface *const m_dest;
	lib::event *m_readdone;
};

// active_final_renderer is a handy subclass of active_renderer:
// it waits until all data is available, reads it, and then calls
// need_redraw on the region. If you subclass this you only
// need to add a redraw method.
class active_final_renderer : public active_renderer {
  public:
	active_final_renderer(
		active_playable_events *context,
		active_playable_events::cookie_type cookie,
		const node *node,
		event_processor *const evp,
		net::passive_datasource *src,
		abstract_rendering_surface *const dest)
	:	active_renderer(context, cookie, node, evp, src, dest),
		m_data(NULL),
		m_data_size(0) {};
	virtual ~active_final_renderer();
	
  protected:
	void readdone();
	void *m_data;
	unsigned m_data_size;
};



// Foctory class for renderers.
class renderer_factory {
  public:
	virtual ~renderer_factory() {}
	virtual active_renderer *new_renderer(
		active_playable_events *context,
		active_playable_events::cookie_type cookie,
		const node *node,
		event_processor *const evp,
		net::passive_datasource *src,
		abstract_rendering_surface *const dest) = 0;
};

class global_renderer_factory : public renderer_factory {
  public:
    global_renderer_factory();
    ~global_renderer_factory();
    
    void add_factory(renderer_factory *rf);
    
    active_renderer *new_renderer(
		active_playable_events *context,
		active_playable_events::cookie_type cookie,
		const node *node,
		event_processor *const evp,
		net::passive_datasource *src,
		abstract_rendering_surface *const dest);
  private:
    std::vector<renderer_factory *> m_factories;
    renderer_factory *m_default_factory;
};


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_RENDERER_H
