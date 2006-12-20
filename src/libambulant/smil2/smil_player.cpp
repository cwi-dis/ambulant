// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/system.h"
#include "ambulant/lib/transition_info.h"

#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/schema.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/smil2/smil_player.h"
#include "ambulant/smil2/timegraph.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/smil2/time_sched.h"
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
	common::factories* factory,
	common::embedder *sys)
{
	return new smil_player(doc, factory, sys);
}

smil_player::smil_player(lib::document *doc, common::factories *factory, common::embedder *sys)
:	m_doc(doc),
	m_factory(factory),
	m_system(sys),
	m_feedback_handler(0),
	m_animation_engine(0),
	m_root(0),
	m_dom2tn(0),
	m_layout_manager(0),
	m_timer(new timer_control_impl(realtime_timer_factory(), 1.0, false)),
	m_event_processor(0),
	m_scheduler(0),
	m_state(common::ps_idle),
	m_cursorid(0), 
	m_pointed_node(0), 
	m_eom_flag(true),
	m_focus(0),
	m_focussed_nodes(new std::set<int>()),
	m_new_focussed_nodes(0)
{
	
	m_logger = lib::logger::get_logger();
	AM_DBG m_logger->debug("smil_player::smil_player()");
}

void
smil_player::initialize()
{
	document_loaded(m_doc);
	
	m_event_processor = event_processor_factory(m_timer);
	// build the layout (we need the top-level layout)
	build_layout();
	// Build the timegraph using the current filter
	build_timegraph();
	m_layout_manager->load_bgimages(m_factory);
}

smil_player::~smil_player() {
	AM_DBG m_logger->debug("smil_player::~smil_player()");
	
	// sync destruction
	m_timer->pause();
	cancel_all_events();
	m_timer->pause();
	cancel_all_events();		
	m_scheduler->reset_document();
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++) {
		int rem = (*it).second->release();
		if (rem) m_logger->trace("smil_player::~smil_player: playable 0x%x still has refcount of %d", (*it).second, rem);
	}
	
	delete m_focussed_nodes;
	delete m_new_focussed_nodes;
	delete m_event_processor;
	delete m_timer;
	delete m_dom2tn;
	delete m_animation_engine;
	delete m_root;
	delete m_scheduler;
//	delete m_doc;
	delete m_layout_manager;
}

void smil_player::build_layout() {
	if(m_state != common::ps_idle && m_state != common::ps_done)
		return;
	if(m_layout_manager) {
		delete m_layout_manager;
		delete m_animation_engine;
	}
	m_layout_manager = new smil_layout_manager(m_factory, m_doc);
	m_animation_engine = new animation_engine(m_event_processor, m_layout_manager);
}

void smil_player::build_timegraph() {
	if(m_state != common::ps_idle && m_state != common::ps_done)
		return;
	if(m_root) {
		delete m_root;
		delete m_dom2tn;
		delete m_scheduler;
	}
	timegraph tg(this, m_doc, schema::get_instance());
	m_root = tg.detach_root();
	m_dom2tn = tg.detach_dom2tn();
	m_scheduler = new scheduler(m_root, m_timer);
}

void smil_player::schedule_event(lib::event *ev, lib::timer::time_type t, event_priority ep) {
	m_event_processor->add_event(ev, t, ep);
}

// Command to start playback
void smil_player::start() {
	if(m_state == common::ps_pausing) {
		resume();
	} else if(m_state == common::ps_idle || m_state == common::ps_done) {
		if(!m_root) build_timegraph();
		if(m_root) {
			if (m_system) m_system->starting(this);
			m_scheduler->start(m_root);
			update();
		}
	}
}

// Command to stop playback
void smil_player::stop() {
	if(m_state != common::ps_pausing && m_state != common::ps_playing)
		return;
	m_timer->pause();
	cancel_all_events();		
	m_scheduler->reset_document();
	done_playback();
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
	document_started();
}

// Done callback from the scheduler
void smil_player::done_playback() {
	m_state = common::ps_done;
	m_timer->pause();
	document_stopped();
	if(m_system) 
		m_system->done(this);
}

// Request to create a playable for the node.
common::playable *smil_player::create_playable(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np == NULL) {
		np = new_playable(n);
AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.enter", (void*)n);
		m_playables_cs.enter();
		m_playables[n] = np;
		m_playables_cs.leave();
AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.leave", (void*)n);
	}
	
	// We also need to remember any accesskey attribute (as opposed to accesskey
	// value for a timing attribute) because these are global.
	// XXX It may be better/cheaper if we simply iterate over the m_playables....
	const char *accesskey = n->get_attribute("accesskey");
	if (accesskey) {
		int nid = n->get_numid();
		// XXX Is this unicode-safe?
		m_accesskey_map[accesskey[0]] = nid;
	}

	return np;
}
// Request to start the playable of the node.
// When trans is not null the playable should transition in 
void smil_player::start_playable(const lib::node *n, double t, const lib::transition_info *trans) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::start_playable(0x%x, %f)", (void*)n, t);
	common::playable *np = create_playable(n);
	if (trans) {
		common::renderer *rend = np->get_renderer();
		if (!rend) {
			const char *pid = n->get_attribute("id");
			m_logger->trace("smil_player::start_playable: node %s has transition but is not visual", pid?pid:"no-id");
		} else {
			rend->set_intransition(trans);
		}
	}
	np->start(t);
}

// Request to seek the playable of the node.
void smil_player::seek_playable(const lib::node *n, double t) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::seek_playable(0x%x, %f)", (void*)n, t);
	common::playable *np = create_playable(n);
	np->seek(t);
}

// Request to start a transition of the playable of the node.
void smil_player::start_transition(const lib::node *n, const lib::transition_info *trans, bool in) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::start_transition(0x%x, -x%x, in=%d)", (void*)n, trans, in);
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(!np) {
		const char *pid = n->get_attribute("id");
		m_logger->debug("smil_player::start_transition: node %s has no playable", pid?pid:"no-id");
		return;
	}
	common::renderer *rend = np->get_renderer();
	if (!rend) {
		const char *pid = n->get_attribute("id");
		m_logger->trace("smil_player::start_transition: node %s has transition but is not visual", pid?pid:"no-id");
	} else {
		if (in) {
			// XXX Jack thinks there's no reason for this...
			AM_DBG m_logger->debug("smil_player::start_transition: called for in-transition");
			rend->set_intransition(trans);
		} else {
			rend->start_outtransition(trans);
		}
	}
}

// Request to stop the playable of the node.
void smil_player::stop_playable(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable(0x%x)", (void*)n);
	if (n == m_focus) {
		m_focus = NULL;
		highlight(n, false);
		node_focussed(NULL);
	}
AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable(0x%x)cs.enter", (void*)n);
	m_playables_cs.enter();
		
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	std::pair<const lib::node*, common::playable*> victim(NULL,NULL);
	if(it != m_playables.end()) {
#ifdef AMBULANT_PLATFORM_WIN32_WCE
		victim = std::pair<const lib::node*, common::playable*>((*it).first, (*it).second);
#else
		victim = *it;
#endif
		m_playables.erase(it);
	}
	m_playables_cs.leave();
	if (victim.first)
		destroy_playable(victim.second, victim.first);
AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable(0x%x)cs.leave", (void*)n);
}

// Request to pause the playable of the node.
void smil_player::pause_playable(const lib::node *n, pause_display d) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::pause_playable(0x%x)", (void*)n);
	common::playable *np = get_playable(n);
	if(np) np->pause(d);
}

// Request to resume the playable of the node.
void smil_player::resume_playable(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::resume_playable(0x%xf)", (void*)n);
	common::playable *np = get_playable(n);
	if(np) np->resume();
}

// Query the node's playable for its duration.
common::duration 
smil_player::get_dur(const lib::node *n) {
	const common::duration not_available(false, 0.0);
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np) {
		common::duration idur = np->get_dur();
		if(idur.first) m_playables_dur[n] = idur.second;
		AM_DBG lib::logger::get_logger()->debug("smil_player::get_dur(0x%x): <%s, %f>", n, idur.first?"true":"false", idur.second);
		return idur;
	}
	std::map<const node*, double>::iterator it2 = m_playables_dur.find(n);
	common::duration rv = (it2 != m_playables_dur.end())?common::duration(true,(*it2).second):not_available;
	AM_DBG lib::logger::get_logger()->debug("smil_player::get_dur(0x%x): <%s, %f>", n, rv.first?"true":"false", rv.second);
	return rv;
}

// Notify the playable that it should update this on user events (click, point).
void smil_player::wantclicks_playable(const lib::node *n, bool want) {
	common::playable *np = get_playable(n);
	if(np) np->wantclicks(want);
}

// Playable notification for a click event.
void smil_player::clicked(int n, double t) {
	AM_DBG m_logger->debug("smil_player::clicked(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> dom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && (*it).second->wants_activate_event()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		dom_event_cb *cb = new dom_event_cb((*it).second, 
			&time_node::raise_activate_event, timestamp);
		schedule_event(cb, 0, ep_high);
		m_scheduler->exec();
	}
}

void
smil_player::before_mousemove(int cursorid)
{
	m_cursorid = cursorid;
	delete m_new_focussed_nodes;
	m_new_focussed_nodes = new std::set<int>();
}

int
smil_player::after_mousemove()
{
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> dom_event_cb;
	std::set<int>::iterator i;

	m_pointed_node = 0;

	// First we send outOfBounds and focusOut events to all
	// the nodes that were in the focus but no longer are.
	for (i=m_focussed_nodes->begin(); i!=m_focussed_nodes->end(); i++) {
		int n = *i;
		
		// If the node is also in the new focus we're done.
		if (m_new_focussed_nodes->count(n) > 0) continue;
		
		// If the node can't be found we're done.
		std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
		if (it == m_dom2tn->end()) continue;
		
		// Otherwise we send it outofbounds and focusout events, if it is interested.
		time_node *tn = (*it).second;
		AM_DBG m_logger->debug("after_mousemove: focus lost by %d, 0x%x", n, tn);
		
		if (tn->wants_outofbounds_event()) {
			AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.outOfBoundsEvent", (void*)tn);
			time_node::value_type root_time = m_root->get_simple_time();
			m_scheduler->update_horizon(root_time);
			q_smil_time timestamp(m_root, root_time);
			dom_event_cb *cb = new dom_event_cb(tn, 
				&time_node::raise_outofbounds_event, timestamp);
			schedule_event(cb, 0, ep_high);
		}
		if (tn->wants_focusout_event()) {
			AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)tn);
			time_node::value_type root_time = m_root->get_simple_time();
			m_scheduler->update_horizon(root_time);
			q_smil_time timestamp(m_root, root_time);
			dom_event_cb *cb = new dom_event_cb(tn, 
				&time_node::raise_focusout_event, timestamp);
			schedule_event(cb, 0, ep_high);
		}
	}
	
	// Next we send inbound and focusin events to the nodes that
	// are now in the focus, and were not there before.
	for (i=m_new_focussed_nodes->begin(); i!=m_new_focussed_nodes->end(); i++) {
		int n = *i;
				
		// If the node can't be found we're done.
		std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
		if (it == m_dom2tn->end()) continue;

		// If the node is interested in activate events (clicks) we tell
		// it that the mouse is over it (it may want to show visual feedback)
		time_node *tn = (*it).second;

		if(tn->wants_activate_event()) {
			m_cursorid = 1;
			node_focussed(tn->dom_node());
			m_pointed_node = tn;
		}

		// If the node was also in the old focus we're done.
		if (m_focussed_nodes->count(n) > 0) continue;

		AM_DBG m_logger->debug("after_mousemove: focus acquired by %d, 0x%x", n, tn);

		// Send it the focusin and inbounds event, if it wants them.
		if (tn->wants_inbounds_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.inBoundsEvent", (void*)tn);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb(tn, 
					&time_node::raise_inbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
		}
		if (tn->wants_focusin_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusInEvent", (void*)tn);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb(tn, 
					&time_node::raise_focusin_event, timestamp);
				schedule_event(cb, 0, ep_high);
		}
	}
	
	// Finally juggle the old and new focussed nodes set
	delete m_focussed_nodes;
	m_focussed_nodes = m_new_focussed_nodes;
	m_new_focussed_nodes = NULL;
	// And if nothing has focus anymore signal that to the embedding app.
	if (m_cursorid == 0)
		node_focussed(NULL);
	return m_cursorid;
}

// Playable notification for a point (mouse over) event.
void smil_player::pointed(int n, double t) {
#if 1
	m_new_focussed_nodes->insert(n);
#else
	AM_DBG m_logger->debug("smil_player::pointed(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> dom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		bool changed_focus = m_pointed_node != (*it).second;
		if (m_pointed_node && changed_focus) {
			// XXX We treat outOfBounds and focusOut identical, which is
			// not 100% correct.
			if (m_pointed_node->wants_outofbounds_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.outOfBoundsEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_outofbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			if (m_pointed_node->wants_focusout_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_focusout_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			m_pointed_node = NULL;
		}
		m_pointed_node = (*it).second;
		if((*it).second->wants_activate_event()) {
			m_cursorid = 1;
			node_focussed(m_pointed_node->dom_node());
		}
		if (changed_focus) {
			AM_DBG m_logger->debug("smil_player::pointed: m_pointed_node is now 0x%x %s[%s]",
				m_pointed_node, 
				m_pointed_node->get_time_attrs()->get_tag().c_str(),
				m_pointed_node->get_time_attrs()->get_id().c_str());
		}
		// XXX We treat inBounds and focusIn identical, which is
		// not 100% correct.
		if (changed_focus && m_pointed_node->wants_inbounds_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.inBoundsEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_inbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
		}
		if (changed_focus && m_pointed_node->wants_focusin_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusInEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_focusin_event, timestamp);
				schedule_event(cb, 0, ep_high);
		}
	} else {
		if (m_pointed_node) {
			// XXX We treat outOfBounds and focusOut identical, which is
			// not 100% correct.
			if (m_pointed_node->wants_outofbounds_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.outOfBoundsEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_outofbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			if (m_pointed_node->wants_focusout_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)m_pointed_node);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_focusout_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			m_pointed_node = NULL;
			node_focussed(NULL);
		}
	}
	AM_DBG m_logger->debug("smil_player::pointed: now m_pointed_node=0x%x", m_pointed_node);
#endif
}

// Playable notification for a start event.
void smil_player::started(int n, double t) {
	AM_DBG m_logger->debug("smil_player::started(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> bom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		bom_event_cb *cb = new bom_event_cb((*it).second, 
			&time_node::on_bom, timestamp);
		schedule_event(cb, 0, ep_high);
	}
}

// Playable notification for a stop event.
void smil_player::stopped(int n, double t) {
	AM_DBG m_logger->debug("smil_player::stopped(%d, %f) roottime=%d", n, t, m_root->get_simple_time());
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> eom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		eom_event_cb *cb = new eom_event_cb((*it).second, 
			&time_node::on_eom, timestamp);
		schedule_event(cb, 0, ep_high);
	}
}

// Playable notification for a transition stop event.
void smil_player::transitioned(int n, double t) {
	// remove fill effect for nodes specifing fill="transition" 
	// and overlap with n
	AM_DBG m_logger->debug("smil_player::transitioned(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> transitioned_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		transitioned_event_cb *cb = new transitioned_event_cb((*it).second, 
			&time_node::on_transitioned, timestamp);
		schedule_event(cb, 0, ep_high);
	}
}

// Playable notification for a stall event.
void smil_player::stalled(int n, double t) {
	AM_DBG m_logger->debug("smil_player::stalled(%d, %f)", n, t);
}

// Playable notification for an unstall event.
void smil_player::unstalled(int n, double t) {
	AM_DBG m_logger->debug("smil_player::unstalled(%d, %f)", n, t);
}

// UI notification for a char event.
void smil_player::on_char(int ch) {
	// First check for an anchor node with accesskey attribute
	std::map<int, int>::iterator p = m_accesskey_map.find(ch);
	if (p != m_accesskey_map.end()) {
		// The character was registered, at some point in time.
		int nid = (*p).second;
		std::map<int, time_node*>::iterator it = m_dom2tn->find(nid);
		if(it != m_dom2tn->end() && (*it).second->wants_activate_event()) {
			// And the node is alive and still wants the event. Use clicked()
			// to do the hard work.
			AM_DBG m_logger->debug("smil_player::on_char() convert to clicked(%d): '%c' [%d]", nid, (char)ch, ch);
			clicked(nid, 0);
			return;
		}
	}

	typedef std::pair<q_smil_time, int> accesskey;
	typedef scalar_arg_callback_event<time_node, accesskey> accesskey_cb;
	time_node::value_type root_time = m_root->get_simple_time();
	m_scheduler->update_horizon(root_time);
	q_smil_time timestamp(m_root, root_time);
	AM_DBG m_logger->debug("smil_player::on_char(): '%c' [%d] at %ld", char(ch), ch, timestamp.second());
	accesskey ak(timestamp, ch);
	accesskey_cb *cb = new accesskey_cb(m_root, &time_node::raise_accesskey, ak);
	schedule_event(cb, 0, ep_high);
	m_scheduler->exec();
}

void smil_player::on_focus_advance() {
	AM_DBG m_logger->debug("smil_player::on_focus_advance");
AM_DBG lib::logger::get_logger()->debug("smil_player:::on_focus_advance(0x%x)cs.enter", (void*)this);
	m_playables_cs.enter();
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.begin();
	// First find the current focus
	if (m_focus != NULL) {
		while (it != m_playables.end() && (*it).first != m_focus) it++;
		if (it == m_playables.end()) {
			m_logger->debug("smil_player::on_focus_advance: current focus unknown");
			it = m_playables.begin();
		} else {
			it++;
		}
	}
	// Now find the next playable that is interested in focus
	while (it != m_playables.end()) {
		std::map<int, time_node*>::iterator tit = m_dom2tn->find((*it).first->get_numid());
		if (tit != m_dom2tn->end()) {
			time_node *tn = (*tit).second;
			if (tn && tn->wants_activate_event()) break;
		}
		it++;
	}
	if (m_focus) highlight(m_focus, false);
	m_focus = NULL;
	if (it != m_playables.end()) m_focus = (*it).first;
	if (m_focus) {
		highlight(m_focus);
	} else {
		m_logger->trace("on_focus_advance: Nothing to focus on");
	}
	m_playables_cs.leave();
AM_DBG lib::logger::get_logger()->debug("smil_player:::on_focus_advance(0x%x)cs.leave", (void*)this);
	node_focussed(m_focus);	
}

void smil_player::on_focus_activate() {
	AM_DBG m_logger->debug("smil_player::on_focus_activate");
	if (m_focus) {
		clicked(m_focus->get_numid(), 0);
	} else {
		m_logger->trace("on_focus_activate: No current focus");
	}
}

// Creates and returns a playable for the node.
common::playable *
smil_player::new_playable(const lib::node *n) {
	int nid = n->get_numid();
	std::string tag = n->get_local_name();
	const char *pid = n->get_attribute("id");
	
	surface *surf = m_layout_manager->get_surface(n);
	AM_DBG m_logger->debug("%s[%s].new_playable 0x%x cookie=%d  rect%s at %s", tag.c_str(), (pid?pid:"no-id"),
		(void*)n, nid,
		::repr(surf->get_rect()).c_str(),
		::repr(surf->get_global_topleft()).c_str());
	common::playable_factory *pf = m_factory->get_playable_factory();
	common::playable *np = pf->new_playable(this, nid, n, m_event_processor);
	// And connect it to the rendering surface
	if (np) {
		common::renderer *rend = np->get_renderer();
		
		if (rend) {
			AM_DBG m_logger->debug("smil_player::new_playable: surface  set,rend = 0x%x, np = 0x%x", (void*) rend, (void*) np);
			rend->set_surface(surf);
			const alignment *align = m_layout_manager->get_alignment(n);
			rend->set_alignment(align);
		} else {
			AM_DBG m_logger->debug("smil_player::new_playable: surface not set because rend == NULL");
		}
	}
	return np;
}

// Destroys the playable of the node (checkpoint).
void smil_player::destroy_playable(common::playable *np, const lib::node *n) {
	AM_DBG {
		std::string tag = n->get_local_name();
		const char *pid = n->get_attribute("id");
	
		m_logger->debug("%s[%s].destroy_playable 0x%x", tag.c_str(), (pid?pid:"no-id"), np);
	}
	np->stop();
	int rem = np->release();
	if (rem) m_logger->debug("smil_player::destroy_playable: playable 0x%x still has refcount of %d", np, rem);
}

void smil_player::show_link(const lib::node *n, const net::url& href, src_playstate srcstate, dst_playstate dststate) {
	AM_DBG lib::logger::get_logger()->debug("show_link(\"%s\"), srcplaystate=%d, dstplaystate=%d",
		href.get_url().c_str(), (int)srcstate, (int)dststate);
	net::url our_url(m_doc->get_src_url()); 
	if(srcstate == src_replace && href.same_document(our_url)) {
		// This is an internal hyperjump
		std::string anchor = href.get_ref();
		const lib::node *target = m_doc->get_node(anchor);
		if(target) {
			goto_node(target);
		} else {
			m_logger->error(gettext("Link destination not found: %s"), href.get_url().c_str());
		}
		return;
	}
	
	if(!m_system) {
		lib::logger::get_logger()->error(gettext("This implementation cannot open <%s> in new window"), href.get_url().c_str());
		return;
	}
	
	if (srcstate == src_pause) {
		AM_DBG lib::logger::get_logger()->debug("show_link: pausing source document");
		pause();
	}
	smil_player *to_replace = NULL;
	if (srcstate == src_replace) {
		AM_DBG lib::logger::get_logger()->debug("show_link: replacing source document");
		to_replace = this;
	}
	if ( dststate == dst_external ) {
		AM_DBG lib::logger::get_logger()->debug("show_link: open externally: \"%s\"", href.get_url().c_str());
		if (to_replace)
			m_system->close(to_replace);
		m_system->show_file(href);
	} else {
		m_system->open(href, dststate == dst_play, to_replace);
	}
}

bool smil_player::goto_node(const lib::node *target)
{
	std::map<int, time_node*>::iterator it = m_dom2tn->find(target->get_numid());
	
	if(it != m_dom2tn->end()) {
		bool already_running = m_root->is_active();
		m_scheduler->start((*it).second);
		if (!already_running) update();
		return true;
	}
	return false;
}

bool
smil_player::highlight(const lib::node *n, bool on)
{
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if (np == NULL) return false;
	common::renderer *rend = np->get_renderer();
	if (rend == NULL) return false;
	common::surface *surf = rend->get_surface();
	if (surf == NULL)  return false;
	surf->highlight(on);
	return true;
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

void smil_player::update() {
	if(m_scheduler && m_root && m_root->is_active()) {
		lib::timer::time_type dt = m_scheduler->exec();
		if(m_root->is_active()) {
			lib::event *update_event = new lib::no_arg_callback_event<smil_player>(this, 
				&smil_player::update);
			m_event_processor->add_event(update_event, dt, lib::ep_high);
		}
	}
}
