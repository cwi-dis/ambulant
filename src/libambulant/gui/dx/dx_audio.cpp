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

#include "ambulant/gui/dx/dx_audio.h"
#include "ambulant/gui/dx/dx_audio_player.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/common/region.h"
#include "ambulant/smil2/test_attrs.h"


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

double s_global_level = 1.0;

extern const char dx_audio_playable_tag[] = "audio";
extern const char dx_audio_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_audio_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererDirectXAudio");
extern const char dx_audio_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererAudio");

common::playable_factory *
gui::dx::create_dx_audio_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXAudio"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererAudio"), true);
	return new common::single_playable_factory<
		gui::dx::dx_audio_renderer,
		dx_audio_playable_tag,
		dx_audio_playable_renderer_uri,
		dx_audio_playable_renderer_uri2,
		dx_audio_playable_renderer_uri3 >(factory, mdp);
}

gui::dx::dx_audio_renderer::dx_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *dxplayer)
:	common::renderer_playable(context, cookie, node, evp, fp, dxplayer),
	m_player(0),
	m_update_event(0),
	m_level(1.0),
	m_balance(0),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
{

	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer(0x%x)", this);
	net::url url = m_node->get_url("src");
	_init_clip_begin_end();
	if(url.is_local_file() && lib::win32::file_exists(url.get_file()))
		m_player = new gui::dx::audio_player(url.get_file());
	else if(url.is_absolute())
		m_player = new gui::dx::audio_player(url.get_url());
	else {
		lib::logger::get_logger()->error("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
}

gui::dx::dx_audio_renderer::~dx_audio_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_audio_renderer()");
	if(m_player) stop();
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
}

void gui::dx::dx_audio_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer::start(0x%x)", this);

	if(!m_player) {
		// Not created or stopped (gone)

		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Does it have all the resources to play?
	if(!m_player->can_play()) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Already activated
	if(m_activated) {
		// repeat
		m_player->start(t);
		schedule_update();
		return;
	}

	// Activate this renderer.
	m_activated = true;

	if (m_intransition && !m_transition_engine) {
		m_transition_engine = new smil2::audio_transition_engine();
		m_transition_engine->init(m_event_processor, false, m_intransition);
	}

	// And set volume(s)
	update_levels();

	// Start the underlying player
	if (m_clip_end == -1)
		m_player->endseek(m_player->get_dur().second);
	else
		m_player->endseek(m_clip_end / 1000000.0);
	m_player->start(t + (m_clip_begin / 1000000.0));

	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);

	// Schedule a self-update
	schedule_update();
}

void gui::dx::dx_audio_renderer::update_levels() {
	if (!m_dest) return;
	const common::region_info *info = m_dest->get_info();
	double level = info ? info->get_soundlevel() : 1;
	level = level * s_global_level;

	if (m_intransition || m_outtransition) {
		level = m_transition_engine->get_volume(level);
	}
	if (level != m_level)
		m_player->set_volume((long)(level*100));
	m_level = level;

	common::sound_alignment align = info ? info->get_soundalign() : common::sa_default;
	int balance = 0;

	if (align == common::sa_left) {
		balance = -100;
	} else if (align == common::sa_right) {
		balance = 100;
	}
	if (balance != m_balance)
		m_player->set_balance(balance);
	m_balance = balance;
}

void
gui::dx::dx_audio_renderer::set_intransition(const lib::transition_info* info) {
	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gui::dx::dx_audio_renderer::start_outtransition(const lib::transition_info* info) {
	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}

void gui::dx::dx_audio_renderer::seek(double t) {
	if (m_player) m_player->seek(t + (m_clip_begin / 1000000.0));
}
std::pair<bool, double> gui::dx::dx_audio_renderer::get_dur() {
	if(m_player) {
		std::pair<bool, double> durp = m_player->get_dur();
		if (!durp.first) return durp;
		double dur = durp.second;
		if (m_clip_end > 0 && dur > m_clip_end / 1000000.0)
			dur = m_clip_end / 1000000.0;
		dur -= (m_clip_begin / 1000000.0);
		return std::pair<bool, double>(true, dur);
	}
	return std::pair<bool, double>(false, 0.0);
}

bool gui::dx::dx_audio_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.stop(0x%x)", this);
	if(!m_player) return true;
	audio_player *p = m_player;
	m_player = 0;
	m_update_event = 0;
	p->stop();
	delete p;
	m_activated = false;
	m_context->stopped(m_cookie);
	return false;
}

void gui::dx::dx_audio_renderer::pause(common::pause_display d) {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.pause(0x%x)", this);
	if(m_player) m_player->pause();
}

void gui::dx::dx_audio_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("dx_audio_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
}

void gui::dx::dx_audio_renderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	// we don't have any bits to blit for audio
}

void gui::dx::dx_audio_renderer::update_callback() {
	if(!m_update_event || !m_player) {
		return;
	}
	if(m_player->is_playing()) {
		update_levels();
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::dx::dx_audio_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dx_audio_renderer>(this,
		&dx_audio_renderer::update_callback);
	m_event_processor->add_event(m_update_event, 100, lib::ep_high);
}

void
gui::dx::set_global_level(double level)
{
	s_global_level = level;
	// XXXX Should also adapt currently existing volumes
}

double
gui::dx::change_global_level(double factor)
{
	s_global_level *= factor;
	// XXXX Should also adapt currently existing volumes
	return s_global_level;
}

void
gui::dx::set_global_rate(double rate)
{
#ifdef WITH_TPB_AUDIO_SPEEDUP
	gui::dx::audio_player::set_global_rate(rate);
#endif
}

double
gui::dx::change_global_rate(double factor)
{
#ifdef WITH_TPB_AUDIO_SPEEDUP
	return gui::dx::audio_player::change_global_rate(factor);
#else
	return 1.0;
#endif
}
