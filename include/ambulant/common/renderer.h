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

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/region.h"
#include "ambulant/net/datasource.h"

namespace ambulant {

namespace lib {

class active_basic_renderer : public ref_counted_obj {
  public:
  	active_basic_renderer()
  	:	m_event_processor(NULL),
  		m_node(NULL),
  		m_playdone(NULL) {}
//  	active_basic_renderer(const ambulant::lib::active_basic_renderer& src)
//  	:	m_event_processor(src.m_event_processor),
//  		m_node(src.m_node),
//  		m_playdone(src.m_playdone) {}
	active_basic_renderer(event_processor *const evp,
		const node *node)
	:	m_event_processor(evp),
		m_node(node),
		m_playdone(NULL) {};
		
	~active_basic_renderer() {}
	
	virtual void start(event *playdone) = 0;
	virtual void stop() = 0;
		
  protected:
  	event_processor *const m_event_processor;
	const node *m_node;
	lib::event *m_playdone;
};

class active_renderer : public active_basic_renderer {
  public:
  	active_renderer()
  	:	active_basic_renderer(NULL, NULL),
  		m_src(NULL),
  		m_dest(0),
  		m_readdone(NULL),
  		m_playdone(NULL) {}
  	active_renderer(const ambulant::lib::active_renderer& src)
  	:	active_basic_renderer(src.m_event_processor, src.m_node),
  		m_dest(0),
  		m_readdone(src.m_readdone),
  		m_playdone(src.m_playdone) {}
	active_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node);
		
	~active_renderer() {}
	
	virtual void start(event *playdone);
	virtual void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft) = 0;
	virtual void stop();
	virtual void pause() {}
	virtual void resume() {}
	
  protected:
	virtual void readdone();

  	net::active_datasource *m_src;
	active_region *const m_dest;
	lib::event *m_readdone;
	lib::event *m_playdone;
};

// active_final_renderer is a handy subclass of active_renderer:
// it waits until all data is available, reads it, and then calls
// need_redraw on the region. If you subclass this you only
// need to add a redraw method.
class active_final_renderer : public active_renderer {
  public:
	active_final_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:	active_renderer(evp, src, dest, node),
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
	virtual active_renderer *new_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node) = 0;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_RENDERER_H
