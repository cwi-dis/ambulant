/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/dx/dx_text.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/lib/region.h"
#include "ambulant/lib/node.h"

using namespace ambulant;

gui::dx::dx_text_renderer::dx_text_renderer(
	lib::event_processor *const evp,
	net::passive_datasource *src,
	lib::passive_region *const dest,
	const lib::node *node)
:   lib::active_renderer(evp, src, dest, node) {
}

gui::dx::dx_text_renderer::~dx_text_renderer() {
}

void gui::dx::dx_text_renderer::start(lib::event *playdone) {
	if(!m_node || !m_src) abort();
	m_playdone = playdone;
	if(!m_src->exists()) {
		lib::logger::get_logger()->error("The location specified for the data source does not exist.");
		m_event_processor->add_event(playdone, 0, lib::event_processor::low);
		return;
	}
	// Create a dx-region
	viewport *v = get_viewport();
	m_region = v->create_region(m_dest->get_rect_outer(), m_dest->get_parent()->get_rect_outer());
	
	// Prepare dx-region's pixel map
	m_region->set_background("teal");
	m_region->clear();
	
	m_dest->show(this);
	m_src->start(m_event_processor, m_readdone);
}

void gui::dx::dx_text_renderer::readdone() {
	lib::logger::get_logger()->trace("dx_text_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
	
	// Prepare dx-region's pixel map
	net::databuffer& db = m_src->get_databuffer();
	m_region->set_text((const char*) db.data(), int(db.size()));
	
	m_dest->need_redraw();
	if(m_playdone)
		m_event_processor->add_event(m_playdone, 0, lib::event_processor::low);
}

void gui::dx::dx_text_renderer::stop() {
	viewport *v = get_viewport();
	v->remove_region(m_region);
	m_region = 0;
	lib::active_renderer::stop();
}

void gui::dx::dx_text_renderer::redraw(const lib::screen_rect<int> &dirty, 
	lib::passive_window *window, const lib::point &window_topleft) {
	viewport *v = get_viewport(window);
	v->redraw();
}

gui::dx::viewport* gui::dx::dx_text_renderer::get_viewport() {
	const lib::passive_region *top = 0;
	const lib::passive_region *p = m_dest->get_parent();
	while(p != 0) {
		top = p;
		p = p->get_parent();
	}
	dx_window *dxwindow = (dx_window *) top;
	return dxwindow->get_viewport();
}

gui::dx::viewport* gui::dx::dx_text_renderer::get_viewport(lib::passive_window *window) {
	dx_window *dxwindow = (dx_window *) window;
	return dxwindow->get_viewport();
}
 

