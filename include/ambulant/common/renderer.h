
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_RENDERER_H
#define AMBULANT_LIB_RENDERER_H

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/region.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"

namespace ambulant {

namespace lib {

namespace detail {

class readdone_callback_arg {
};

}; // namespace detail

class active_renderer : public ref_counted {
  public:
  	active_renderer()
  	:	m_event_processor(NULL),
  		m_src(NULL),
  		m_node(NULL),
  		m_readdone(NULL),
  		m_playdone(NULL),
  		m_refcount(1), m_dest(0) {}
  	active_renderer(const ambulant::lib::active_renderer& src)
  	:	m_event_processor(src.m_event_processor),
  		m_src(src.m_src),
  		m_node(src.m_node),
  		m_readdone(src.m_readdone),
  		m_playdone(src.m_playdone),
  		m_refcount(1), m_dest(0) {}
	active_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node);
		
	~active_renderer() {}
	
	virtual void start(event *playdone);
	virtual void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft) = 0;
	virtual void stop();
	
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

  protected:
	virtual void readdone(detail::readdone_callback_arg *dummy);

  	event_processor *const m_event_processor;
  	net::active_datasource *m_src;
	active_region *const m_dest;
	const node *m_node;
	lib::event *m_readdone;
	lib::event *m_playdone;
	basic_atomic_count<critical_section> m_refcount;
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
	void readdone(detail::readdone_callback_arg *dummy);
	void *m_data;
	unsigned m_data_size;
};



// Foctory class for renderers.
class renderer_factory {
  public:
	virtual active_renderer *new_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node) = 0;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_RENDERER_H
