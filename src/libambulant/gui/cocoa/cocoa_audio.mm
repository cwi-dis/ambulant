
/* 
 * @$Id$ 
 */

#include "ambulant/gui/cocoa/cocoa_audio.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_active_audio_renderer::cocoa_active_audio_renderer(event_processor *const evp,
	net::passive_datasource *src,
	const node *node)
:	active_basic_renderer(evp, node),
	m_url(src->get_url()),
	m_sound(NULL)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (!m_node) abort();
	NSString *filename = [NSString stringWithCString: m_url.c_str()];
	m_sound = [[NSSound alloc] initWithContentsOfFile:filename byReference: YES];
	if (!m_sound)
		lib::logger::get_logger()->error("cocoa_active_audio_renderer: cannot open soundfile: %s", m_url.c_str());
	m_timer_index = m_event_processor->get_timer()->add_dependent(this);
	[pool release];
}

cocoa_active_audio_renderer::~cocoa_active_audio_renderer()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG logger::get_logger()->trace("~cocoa_active_audio_renderer(0x%x)", (void *)this);
	if (m_sound)
		[m_sound release];
	m_sound = NULL;
	m_event_processor->get_timer()->remove_dependent(this);
	m_lock.leave();
	[pool release];
}
	
void
cocoa_active_audio_renderer::start(event *playdone)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	std::ostringstream os;
	os << *m_node;
	AM_DBG lib::logger::get_logger()->trace("cocoa_active_audio_renderer.start(0x%x, %s, playdone=0x%x)", (void *)this, os.str().c_str(), (void *)playdone);
	if (m_sound)
		if (![m_sound play])
			lib::logger::get_logger()->error("cocoa_active_audio_renderer: cannot start audio");
	if (m_playdone)
		m_event_processor->add_event(m_playdone, 0, event_processor::low);
	m_lock.leave();
	[pool release];
}

void
cocoa_active_audio_renderer::stop()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_active_audio_renderer.stop(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound stop])
			lib::logger::get_logger()->error("cocoa_active_audio_renderer: cannot stop audio");
	}
	m_lock.leave();
	[pool release];
}

void
cocoa_active_audio_renderer::speed_changed()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("cocoa_active_audio_renderer.speed_changed(0x%x)", (void *)this);
	if (m_sound) {
		abstract_timer *our_timer = m_event_processor->get_timer();
		double rtspeed = our_timer->get_realtime_speed();
		
		if (rtspeed < 0.01) {
			if (![m_sound pause])
				lib::logger::get_logger()->error("cocoa_active_audio_renderer: cannot pause audio");
		} else {
			if (rtspeed < 0.99)
				lib::logger::get_logger()->trace("cocoa_active_audio_renderer: only speed 1.0 and 0.0 supported, not %f", rtspeed);
			if (![m_sound resume])
				lib::logger::get_logger()->error("cocoa_active_audio_renderer: cannot resume audio");
		}
	}
	m_lock.leave();
}



} // namespace cocoa

} // namespace gui

} //namespace ambulant
