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
 * communicate with Ambulant Player solely throlowugh the region and renderer
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

// #define AM_DBG if(1)

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
	m_animation_engine(0),
	m_root(0),
	m_dom2tn(0),
	m_layout_manager(0),
	m_timer(new timer(realtime_timer_factory(), 1.0, false)),
	m_event_processor(0),
	m_scheduler(0),
	m_state(common::ps_idle),
	m_cursorid(0), 
	m_pointed_node(0), 
	m_eom_flag(true) {
	m_logger = lib::logger::get_logger();
	AM_DBG m_logger->debug("smil_player::smil_player()");
	m_event_processor = event_processor_factory(m_timer);
	
	// build the layout (we need the top-level layout)
	build_layout();
	
	// Build the timegraph using the current filter
	build_timegraph();
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
		
	delete m_event_processor;
	delete m_timer;
	delete m_dom2tn;
	delete m_animation_engine;
	delete m_root;
	delete m_scheduler;
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
	m_layout_manager = new smil_layout_manager(m_factory->wf, m_doc);
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
	m_event_processor->add_event(ev, t, (event_processor::event_priority)ep);
}

// Command to start playback
void smil_player::start() {
	if(m_state == common::ps_pausing) {
		resume();
	} else if(m_state == common::ps_idle || m_state == common::ps_done) {
		if(!m_root) build_timegraph();
		if(m_root) {
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
}

// Done callback from the scheduler
void smil_player::done_playback() {
	m_state = common::ps_done;
	m_timer->pause();
	if(m_system) 
		m_system->done(this);
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
	AM_DBG lib::logger::get_logger()->debug("smil_player::pause_playable(0x%x)", (void*)n);
	common::playable *np = get_playable(n);
	if(np) np->pause();
}

// Request to resume the playable of the node.
void smil_player::resume_playable(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::resume_playable(0x%xf)", (void*)n);
	common::playable *np = get_playable(n);
	if(np) np->resume();
}

// Query the node's playable for its duration.
std::pair<bool, double> 
smil_player::get_dur(const lib::node *n) {
	const std::pair<bool, double> not_available(false, 0.0);
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(np) {
		std::pair<bool, double> idur = np->get_dur();
		if(idur.first) m_playables_dur[n] = idur.second;
		AM_DBG lib::logger::get_logger()->debug("smil_player::get_dur(0x%x): <%s, %f>", n, idur.first?"true":"false", idur.second);
		return idur;
	}
	std::map<const node*, double>::iterator it2 = m_playables_dur.find(n);
	std::pair<bool, double> rv = (it2 != m_playables_dur.end())?std::pair<bool, double>(true,(*it2).second):not_available;
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
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		dom_event_cb *cb = new dom_event_cb((*it).second, 
			&time_node::raise_activate_event, timestamp);
		schedule_event(cb, 0, ep_high);
		m_scheduler->exec();
	}
}

// Playable notification for a point (mouse over) event.
void smil_player::pointed(int n, double t) {
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
				q_smil_time timestamp(m_root, m_root->get_simple_time());
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_outofbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			if (m_pointed_node->wants_focusout_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)m_pointed_node);
				q_smil_time timestamp(m_root, m_root->get_simple_time());
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_focusout_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			m_pointed_node = NULL;
		}
		m_pointed_node = (*it).second;
		if((*it).second->wants_activate_event())
			m_cursorid = 1;
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
				q_smil_time timestamp(m_root, m_root->get_simple_time());
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_inbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
		}
		if (changed_focus && m_pointed_node->wants_focusin_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusInEvent", (void*)m_pointed_node);
				q_smil_time timestamp(m_root, m_root->get_simple_time());
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
				q_smil_time timestamp(m_root, m_root->get_simple_time());
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_outofbounds_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			if (m_pointed_node->wants_focusout_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)m_pointed_node);
				q_smil_time timestamp(m_root, m_root->get_simple_time());
				dom_event_cb *cb = new dom_event_cb((*it).second, 
					&time_node::raise_focusout_event, timestamp);
				schedule_event(cb, 0, ep_high);
			}
			m_pointed_node = NULL;
		}
	}
	AM_DBG m_logger->debug("smil_player::pointed: now m_pointed_node=0x%x", m_pointed_node);
}

// Playable notification for a start event.
void smil_player::started(int n, double t) {
	AM_DBG m_logger->debug("smil_player::started(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> bom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		bom_event_cb *cb = new bom_event_cb((*it).second, 
			&time_node::on_bom, timestamp);
		schedule_event(cb, 0, ep_high);
	}
}

// Playable notification for a stop event.
void smil_player::stopped(int n, double t) {
	AM_DBG m_logger->debug("smil_player::stopped(%d, %f)", n, t);
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> eom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
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
		q_smil_time timestamp(m_root, m_root->get_simple_time());
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
	typedef std::pair<q_smil_time, int> accesskey;
	typedef scalar_arg_callback_event<time_node, accesskey> accesskey_cb;
	q_smil_time timestamp(m_root, m_root->get_simple_time());
	AM_DBG m_logger->debug("smil_player::on_char(): '%c' [%d] at %ld", char(ch), ch, timestamp.second());
	accesskey ak(timestamp, ch);
	accesskey_cb *cb = new accesskey_cb(m_root, &time_node::raise_accesskey, ak);
	schedule_event(cb, 0, ep_high);
	m_scheduler->exec();
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
	common::playable_factory *pf = (playable_factory*) m_factory->rf;
	common::playable *np = pf->new_playable(this, nid, n, m_event_processor);
	// And connect it to the rendering surface
	if (np) {
		common::renderer *rend = np->get_renderer();
		
		if (rend) {
			AM_DBG m_logger->debug("smil_player::new_playable: surface  set,rend = 0x%x, np = 0x%x", (void*) rend, (void*) np);
			rend->set_surface(surf);
			alignment *align = m_layout_manager->get_alignment(n);
			rend->set_alignment(align);
		} else {
			AM_DBG m_logger->debug("smil_player::new_playable: surface not set because rend == NULL");
		}

		
	}
	return np;
}

// Destroys the playable of the node (checkpoint).
void smil_player::destroy_playable(common::playable *np, const lib::node *n) {
#if 1
	std::string tag = n->get_local_name();
	const char *pid = n->get_attribute("id");
	
	AM_DBG m_logger->debug("%s[%s].destroy_playable 0x%x", tag.c_str(), (pid?pid:"no-id"), np);
#endif
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
			std::map<int, time_node*>::iterator it = m_dom2tn->find(target->get_numid());
			if(it != m_dom2tn->end()) {
				m_scheduler->start((*it).second);
			}
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
			m_event_processor->add_event(update_event, dt, event_processor::high);
		}
	}
}
