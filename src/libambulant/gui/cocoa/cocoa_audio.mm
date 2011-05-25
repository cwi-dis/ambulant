// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/gui/cocoa/cocoa_audio.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {


extern const char cocoa_audio_playable_tag[] = "audio";
extern const char cocoa_audio_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoaAudio");
extern const char cocoa_audio_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererAudio");

common::playable_factory *
create_cocoa_audio_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoaAudio"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererAudio"), true);
	return new common::single_playable_factory<
		cocoa_audio_playable,
		cocoa_audio_playable_tag,
		cocoa_audio_playable_renderer_uri,
		cocoa_audio_playable_renderer_uri2,
		cocoa_audio_playable_renderer_uri2 >(factory, mdp);
}

cocoa_audio_playable::cocoa_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	playable_imp(context, cookie, node, evp, fp, mdp),
	m_url(node->get_url("src")),
	m_sound(NULL)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (!m_url.is_local_file()) {
		lib::logger::get_logger()->error(gettext("cocoa_audio: cannot play non-local sound %s"), m_url.get_url().c_str());
	} else {
		NSString *filename = [NSString stringWithCString: m_url.get_file().c_str() encoding: NSUTF8StringEncoding];
		m_sound = [[NSSound alloc] initWithContentsOfFile:filename byReference: YES];
		if (!m_sound)
			lib::logger::get_logger()->error(gettext("%s: cannot open soundfile"), m_url.get_url().c_str());
	}
//	m_event_processor->get_timer()->add_listener(this);
	[pool release];
}

cocoa_audio_playable::~cocoa_audio_playable()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cocoa_audio_playable(0x%x)", (void *)this);
	if (m_sound)
		[m_sound release];
	m_sound = NULL;
//	m_event_processor->get_timer()->remove_listener(this);
	m_lock.leave();
	[pool release];
}

void
cocoa_audio_playable::start(double where)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.start(0x%x, %s, %f)", (void *)this, m_url.get_url().c_str(), where);
	if (where != 0.0)
		lib::logger::get_logger()->trace("Audio item: ignoring start time %f, starting at 0", where);
	if (m_sound)
		if (![m_sound play]) {
			lib::logger::get_logger()->error(gettext("%s: Cannot start audio playback"), m_url.get_url().c_str());
			[m_sound release];
			m_sound = NULL;
		}
	m_context->started(m_cookie, 0);
	if (!m_sound)
		m_context->stopped(m_cookie, 0);
	m_lock.leave();
	[pool release];
	check_still_playing();
}

void
cocoa_audio_playable::check_still_playing()
{
	m_lock.enter();
	if (m_sound != NULL) {
		bool still_playing = [m_sound isPlaying];

		if (still_playing) {
			AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.check_still_playing(0x%x): busy", (void*)this);
			typedef lib::no_arg_callback<cocoa_audio_playable> check_still_playing_callback;
			lib::event *ev = new check_still_playing_callback(this, &cocoa_audio_playable::check_still_playing);
			m_event_processor->add_event(ev, 100, lib::ep_med);
		} else {
			AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.check_still_playing(0x%x): finished", (void*)this);
			[m_sound release];
			m_sound = NULL;
			m_context->stopped(m_cookie, 0);
		}
	}
	m_lock.leave();
}

void
cocoa_audio_playable::seek(double where)
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.seek(0x%x, %f): ignored", (void*)this, where);
}

bool
cocoa_audio_playable::stop()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.stop(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound stop])
			lib::logger::get_logger()->error(gettext("%s: Cannot stop audio playback"), m_url.get_url().c_str());
		[m_sound release];
		m_sound = NULL;
		m_context->stopped(m_cookie, 0);
	}
	m_lock.leave();
	[pool release];
	return true; // Don't re-use this renderer}
}

void
cocoa_audio_playable::pause(pause_display d)
{
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.pause(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound pause])
			lib::logger::get_logger()->error(gettext("%s: Cannot pause audio playback"), m_url.get_url().c_str());
	}
	m_lock.leave();
//	[pool release];
}

void
cocoa_audio_playable::resume()
{
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_audio_playable.resume(0x%x)", (void *)this);
	if (m_sound) {
		if (![m_sound resume])
			lib::logger::get_logger()->error(gettext("%s: Cannot resume audio playback"), m_url.get_url().c_str());
	}
	m_lock.leave();
//	[pool release];
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
