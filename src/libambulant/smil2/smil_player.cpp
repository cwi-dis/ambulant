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
 
#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/system.h"

#include "ambulant/common/layout.h"
#include "ambulant/common/schema.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/smil2/smil_player.h"
#include "ambulant/smil2/timegraph.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/smil2/animate_e.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

common::player *
common::create_smil2_player(
	lib::document *doc,
	common::window_factory *wf,
	common::playable_factory *pf,
	lib::system *sys)
{
	return new smil_player(doc, wf, pf, sys);
}

smil_player::smil_player(lib::document *doc, common::window_factory *wf, common::playable_factory *pf, lib::system *sys)
:	m_doc(doc),
	m_wf(wf),
	m_pf(pf),
	m_system(sys),
	m_animation_engine(0),
	m_root(0),
	m_dom2tn(0),
	m_layout_manager(0),
	m_timer(new timer(realtime_timer_factory(), 1.0, false)),
	m_event_processor(0),
	m_state(common::ps_idle),
	m_cursorid(0), 
	m_pointed_node(0) {
	m_logger = lib::logger::get_logger();
	AM_DBG m_logger->trace("smil_player::smil_player()");
	m_event_processor = event_processor_factory(m_timer);
	
	// build the layout (we need the top-level layout)
	build_layout();
	
	// Build the timegraph using the current filter
	build_timegraph();
}

smil_player::~smil_player() {
	AM_DBG m_logger->trace("smil_player::~smil_player()");
	
	// sync destruction
	m_timer->pause();
	cancel_all_events();
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->release();
		
	delete m_event_processor;
	delete m_timer;
	delete m_dom2tn;
	delete m_animation_engine;
	delete m_root;
	delete m_doc;
	delete m_layout_manager;
}

void smil_player::build_layout() {
	if(m_state != common::ps_idle && m_state != common::ps_done)
		return;
	if(m_layout_manager) {
		delete m_layout_manager;
		delete m_animation_engine;
	}
	m_layout_manager = new smil_layout_manager(m_wf, m_doc);
	m_animation_engine = new animation_engine(m_event_processor, m_layout_manager);
}

void smil_player::build_timegraph() {
	if(m_state != common::ps_idle && m_state != common::ps_done)
		return;
	if(m_root) {
		delete m_root;
		delete m_dom2tn;
	}
	timegraph tg(this, m_doc, schema::get_instance());
	m_root = tg.detach_root();
	m_dom2tn = tg.detach_dom2tn();
}

void smil_player::schedule_event(lib::event *ev, time_type t, event_priority ep) {
	m_event_processor->add_event(ev, t, (event_processor::event_priority)ep);
}

// Command to start playback
void smil_player::start() {
	if(m_state == common::ps_pausing) {
		resume();
	} else if(m_state == common::ps_idle || m_state == common::ps_done) {
		if(!m_root) build_timegraph();
		if(m_root) m_root->start();
		m_timer->resume();
	}
}

// Command to stop playback
void smil_player::stop() {
	if(m_state != common::ps_pausing && m_state != common::ps_playing)
		return;
	if(m_state == common::ps_pausing) 
		m_timer->resume();
	m_root->stop();
}

// Command to pause playback
void smil_player::pause() {
	if(m_state != common::ps_playing)
		return;	
	m_state = common::ps_pausing;
	m_timer->pause();
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->pause();
}

// Command to resume playback
void smil_player::resume() {
	if(m_state != common::ps_pausing)
		return;	
	m_state = common::ps_playing;
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->resume();
	m_timer->resume();
}

// Started callback from the scheduler
void smil_player::started_playback() {
	m_state = common::ps_playing;
}

// Done callback from the scheduler
void smil_player::done_playback() {
	m_state = common::ps_done;
	m_timer->pause();
}

// Request to create a playable for the node.
common::playable *smil_player::create_playable(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np) return np;
	np = new_playable(n);
	m_playables_cs.enter();
	m_playables[n] = np;
	m_playables_cs.leave();
	return np;
}
// Request to start the playable of the node.
// When trans is not null the playable should transition in 
void smil_player::start_playable(const lib::node *n, double t, const lib::node *trans) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np) {
		np->start(t);
		return;
	}
	np = new_playable(n);
	m_playables_cs.enter();
	m_playables[n] = np;
	m_playables_cs.leave();
	np->start(t);	
}

// Request to start a transition of the playable of the node.
void smil_player::start_transition(const lib::node *n, const lib::node *trans, bool in) {
}

// Request to stop the playable of the node.
void smil_player::stop_playable(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	if(it != m_playables.end()) {
		destroy_playable((*it).second, (*it).first);
		m_playables_cs.enter();
		m_playables.erase(it);
		m_playables_cs.leave();
	}
}

// Request to pause the playable of the node.
void smil_player::pause_playable(const lib::node *n, pause_display d) {
	common::playable *np = get_playable(n);
	if(np) np->pause();
}

// Request to resume the playable of the node.
void smil_player::resume_playable(const lib::node *n) {
	common::playable *np = get_playable(n);
	if(np) np->resume();
}

// Query the node's playable for its duration.
std::pair<bool, double> 
smil_player::get_dur(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np) {
		return np->get_dur();
	}
	np = new_playable(n);
	m_playables[n] = np;
	return np->get_dur();
}

// Notify the playable that it should update this on user events (click, point).
void smil_player::wantclicks_playable(const lib::node *n, bool want) {
	common::playable *np = get_playable(n);
	if(np) np->wantclicks(want);
}

// Playable notification for a click event.
void smil_player::clicked(int n, double t) {
	typedef scalar_arg_callback_event<time_node, q_smil_time> activate_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && (*it).second->wants_activate_event()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		activate_event_cb *cb = new activate_event_cb((*it).second, 
			&time_node::raise_activate_event, timestamp);
		schedule_event(cb, 0);
	}
}

// Playable notification for a point (mouse over) event.
void smil_player::pointed(int n, double t) {
	typedef scalar_arg_callback_event<time_node, q_smil_time> activate_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		m_pointed_node = (*it).second;
		if((*it).second->wants_activate_event())
			m_cursorid = 1;
	}
}

// Playable notification for a start event.
void smil_player::started(int n, double t) {
	typedef scalar_arg_callback_event<time_node, q_smil_time> bom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		bom_event_cb *cb = new bom_event_cb((*it).second, 
			&time_node::on_bom, timestamp);
		schedule_event(cb, 0);
	}
}

// Playable notification for a stop event.
void smil_player::stopped(int n, double t) {
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> eom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		eom_event_cb *cb = new eom_event_cb((*it).second, 
			&time_node::on_eom, timestamp);
		schedule_event(cb, 0);
	}
}

// Playable notification for a transition stop event.
void smil_player::transitioned(int n, double t) {
	// remove fill effect for nodes specifing fill="transition" 
	// and overlap with n
}

// Playable notification for a stall event.
void smil_player::stalled(int n, double t) {
}

// Playable notification for an unstall event.
void smil_player::unstalled(int n, double t) {
}

// UI notification for a char event.
void smil_player::on_char(int ch) {
	typedef std::pair<q_smil_time, int> accesskey;
	typedef scalar_arg_callback_event<time_node, accesskey> accesskey_cb;
	q_smil_time timestamp(m_root, m_root->get_simple_time());
	AM_DBG m_logger->trace("smil_player::on_char(): '%c' [%d] at %ld", char(ch), ch, timestamp.second());
	accesskey ak(timestamp, ch);
	accesskey_cb *cb = new accesskey_cb(m_root, &time_node::raise_accesskey, ak);
	schedule_event(cb, 0);
}

// Creates and returns a playable for the node.
common::playable *
smil_player::new_playable(const lib::node *n) {
	int nid = n->get_numid();
	std::string tag = n->get_local_name();
	const char *pid = n->get_attribute("id");
	
	surface *surf = m_layout_manager->get_surface(n);
	AM_DBG m_logger->trace("%s[%s].new_playable  rect%s at %s", tag.c_str(), (pid?pid:"no-id"),
		::repr(surf->get_rect()).c_str(),
		::repr(surf->get_global_topleft()).c_str());
		
	common::playable *np = m_pf->new_playable(this, nid, n, m_event_processor);
	// And connect it to the rendering surface
	if (np) {
		common::renderer *rend = np->get_renderer();
		
		if (rend) {
			AM_DBG m_logger->trace("smil_player::new_playable: surface  set,rend = 0x%x, np = 0x%x", (void*) rend, (void*) np);
			rend->set_surface(surf);
		} else {
			AM_DBG m_logger->trace("smil_player::new_playable: surface not set becasue rend == NULL");
		}

		
	}
	return np;
}

// Destroys the playable of the node (checkpoint).
void smil_player::destroy_playable(common::playable *np, const lib::node *n) {
	np->stop();
	np->release();
}

void smil_player::show_link(const lib::node *n, const std::string& href) {
	if(m_system) {
		m_system->show_file(href);
	} else {
		lib::logger::get_logger()->error("This implementation cannot open <%s> in a browser", href.c_str());
	}
}

std::string smil_player::get_pointed_node_str() const {
	if(m_pointed_node == 0) return "";
	const time_node *tn = m_pointed_node;
	const lib::node *n = tn->dom_node();
	const char *pid = n->get_attribute("id");
	const char *reg = 0;
	if(n->get_local_name() == "area" && n->up()) {
		reg = n->up()->get_attribute("region");
	} else {
		reg = n->get_attribute("region");
	}
	const char *href = n->get_attribute("href");
	char buf[256];
	if(href) 
		sprintf(buf, "%.32s - %.32s : %.32s", (pid?pid:"no-id"), (reg?reg:"no-reg"), href);
	else
		sprintf(buf, "%.32s - %.32s", (pid?pid:"no-id"), (reg?reg:"no-reg"));
	return buf;
}
