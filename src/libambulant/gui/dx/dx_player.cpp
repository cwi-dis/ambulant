
/* 
 * @$Id$ 
 */

 
#include "ambulant/gui/dx/dx_player_impl.h"

#include "ambulant/common/player.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

using namespace ambulant;

gui::dx::dx_player_impl::dx_player_impl(const std::string& url, VCF f) 
:	m_url(url),
	m_create_viewport_fn(f),
	m_viewport(0),
	m_wf(0), 
	m_rf(0),
	m_pplayer(0), 
	m_aplayer(0),
	m_processor(0),
	m_logger(lib::logger::get_logger()) {
		
}

gui::dx::dx_player_impl::~dx_player_impl() {
	delete m_processor;
	delete m_pplayer;
	m_aplayer = lib::release(m_aplayer);
	// verify:
	if(m_aplayer != 0)
		m_logger->warn("active_player ref_count: " + m_aplayer->get_ref_count());
	delete m_rf;
	delete m_wf;
	delete m_viewport;
}

gui::dx::viewport* gui::dx::dx_player_impl::create_viewport(int w, int h) {
	if(m_create_viewport_fn)
		return (*m_create_viewport_fn)(w, h);
	if(!m_viewport)
		m_viewport = new viewport(w, h, 0);
	return m_viewport;
}

bool gui::dx::dx_player_impl::is_done() const { 
	return m_aplayer && m_aplayer->is_done();
}

bool gui::dx::dx_player_impl::start() {
	m_logger->trace("Attempting to play: %s", m_url.c_str());

	// Create passive_player from filename
	m_pplayer = new lib::passive_player(m_url.c_str());
	if (!m_pplayer) {
		m_logger->error("Failed to construct passive_player from file %s", m_url.c_str());
		return false;
	}
	
	// Create GUI window_factory and renderer_factory
	m_wf = new gui::dx::dx_window_factory(this);
	m_rf = new gui::dx::dx_renderer_factory(this);
	
	// Request an active_player for the provided factories 
	m_aplayer = m_pplayer->activate(m_wf, m_rf);
	if (!m_aplayer) {
		m_logger->error("passive_player::activate() failed to create active_player");
		return false;
	}
	
	// Pass a flag_event to be set when done.
	m_aplayer->start();
	m_logger->trace("Started playing");
	return  true;
}

void gui::dx::dx_player_impl::stop() {
	m_logger->trace("Attempting to stop: %s", m_url.c_str());
	if(m_aplayer) m_aplayer->stop();
}
void gui::dx::dx_player_impl::pause() {
	m_logger->trace("Attempting to pause: %s", m_url.c_str());
	if(m_aplayer) {
		if(m_aplayer->get_speed() == 0)
			m_aplayer->set_speed(1.0);
		else
			m_aplayer->set_speed(0);
	}
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url) {
	return new dx_player_impl(url, 0);
}

// static 
gui::dx::dx_player* gui::dx::dx_player::create_player(const std::string& url, VCF f) {
	return new dx_player_impl(url, f);
}

 

