/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/dx/dx_audio.h"

#include "ambulant/lib/region.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"

using namespace ambulant;

gui::dx::dx_audio_renderer::dx_audio_renderer(
	lib::event_processor *const evp,
	net::passive_datasource *src,
	lib::passive_region *const dest,
	const lib::node *node)
:   lib::active_renderer(evp, src, dest, node), 
	m_player(0) {
}

gui::dx::dx_audio_renderer::~dx_audio_renderer() {
	delete m_player;
}

void gui::dx::dx_audio_renderer::start(lib::event *playdone) {
	// XXX: do not call the base lib::active_renderer::start(playdone) as we should.
	// This renderer will read/decode its data directly from the url.
	if(!m_node || !m_src) abort();
	m_playdone = playdone;
	if(!m_src->exists()) {
		lib::logger::get_logger()->error("The location specified for the data source does not exist.");
		m_event_processor->add_event(playdone, 0, lib::event_processor::low);
		return;
	}
	m_dest->show(this);
	m_player = new gui::dx::audio_player<net::active_datasource>(m_src);
	m_player->play();
}

void gui::dx::dx_audio_renderer::stop() {
	lib::logger::get_logger()->trace("dx_audio_renderer.stop(0x%x)", (void *)this);
	m_player->stop();
	lib::active_renderer::stop();
}

void gui::dx::dx_audio_renderer::redraw(const lib::screen_rect<int> &dirty, lib::passive_window *window, 
	const lib::point &window_topleft) {
	// we don't need to do anything for audio
}

