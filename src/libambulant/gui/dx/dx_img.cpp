/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/dx/dx_img.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/lib/region.h"
#include "ambulant/lib/node.h"

#include "jpeglib.h"
#pragma comment (lib,"libjpeg.lib")

using namespace ambulant;

gui::dx::dx_img_renderer::dx_img_renderer(
	lib::event_processor *const evp,
	net::passive_datasource *src,
	lib::passive_region *const dest,
	const lib::node *node)
:   lib::active_renderer(evp, src, dest, node) {
}

gui::dx::dx_img_renderer::~dx_img_renderer() {
}

void gui::dx::dx_img_renderer::start(lib::event *playdone) {
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
	
	// Prepare dx-region's pixel map bgd
	m_region->set_background("silver");
	m_region->clear();
	
	m_dest->show(this);
	m_src->start(m_event_processor, m_readdone);
}

void gui::dx::dx_img_renderer::readdone() {
	lib::logger::get_logger()->trace("dx_img_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
	
	// Prepare dx-region's pixel map
	typedef jpg_decoder<net::active_datasource, lib::color_trible> decoder_class;
	
	HDC hdc = ::GetDC(NULL);
	decoder_class* decoder = new decoder_class(m_src, hdc);
	if(decoder->can_decode()) {
		dib_surface<lib::color_trible> *ds = decoder->decode();
		m_region->set_bmp(ds->m_hbmp);
	}
	delete decoder;
	::DeleteDC(hdc);
	
	m_dest->need_redraw();
	if(m_playdone)
		m_event_processor->add_event(m_playdone, 0, lib::event_processor::low);
}

void gui::dx::dx_img_renderer::stop() {
	viewport *v = get_viewport();
	v->remove_region(m_region);
	m_region = 0;
	lib::active_renderer::stop();
}

void gui::dx::dx_img_renderer::redraw(const lib::screen_rect<int> &dirty, 
	lib::passive_window *window, const lib::point &window_topleft) {
	viewport *v = get_viewport(window);
	v->redraw();
}

gui::dx::viewport* gui::dx::dx_img_renderer::get_viewport() {
	const lib::passive_region *top = 0;
	const lib::passive_region *p = m_dest->get_parent();
	while(p != 0) {
		top = p;
		p = p->get_parent();
	}
	dx_window *dxwindow = (dx_window *) top;
	return dxwindow->get_viewport();
}

gui::dx::viewport* gui::dx::dx_img_renderer::get_viewport(lib::passive_window *window) {
	dx_window *dxwindow = (dx_window *) window;
	return dxwindow->get_viewport();
}
 

