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
#include "ambulant/common/layout.h"
#include "ambulant/common/schema.h"
#include "ambulant/common/renderer.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/smil2/smil_player.h"
#include "ambulant/smil2/timegraph.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

smil_player::smil_player(lib::document *doc, common::window_factory *wf, common::playable_factory *rf)
:	m_doc(doc),
	m_wf(wf),
	m_rf(rf),
	m_root(0),
	m_dom2tn(0),
	m_layout_manager(0),
	m_timer(new timer(realtime_timer_factory())),
	m_event_processor(0)
{
	m_logger = lib::logger::get_logger();
	AM_DBG m_logger->trace("smil_player::smil_player()");
	m_event_processor = event_processor_factory(m_timer);
	
	// build DOM lebel objects
	test_attrs::read_custom_attributes(m_doc);
	
	// though we need only the top level windows at this moment
	m_layout_manager = new smil_layout_manager(m_wf, m_doc);
	
}

smil_player::~smil_player() {
	AM_DBG m_logger->trace("smil_player::~smil_player()");
	stop();
	delete m_event_processor;
	delete m_timer;
	delete m_dom2tn;
	delete m_root;
	delete m_doc;
	delete m_layout_manager;
}

void smil_player::build_layout() {
	if(m_layout_manager) delete m_layout_manager;
	m_layout_manager = new smil_layout_manager(m_wf, m_doc);
}

void smil_player::build_timegraph() {
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

void smil_player::start() {
	if(!m_root) build_timegraph();
	if(m_root) m_root->start();
}

void smil_player::stop() {
	m_timer->stop();
	m_event_processor->stop_processor_thread();
	
	// The following operation deletes all the waiting events
	// Deleting an event that uses ref-counted objects calls release()
	m_event_processor->cancel_all_events();
	
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		destroy_playable((*it).second, (*it).first);
	m_playables.clear();
}

void smil_player::pause() {
	m_timer->pause();
	// we don't propagate pause/resume yet
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->pause();
}

void smil_player::resume() {
	m_timer->resume();
	// we don't propagate pause/resume yet
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->resume();
}

bool smil_player::is_done() const {
	return !m_root->is_active();
}

// Done playback callback
void smil_player::done_playback() {
}

void smil_player::start_playable(const lib::node *n, double t) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *nr = (it != m_playables.end())?(*it).second:0;
	if(nr) {
		nr->start(t);
		return;
	}
	nr = create_playable(n);
	m_playables[n] = nr;
	nr->start(t);	
}

void smil_player::stop_playable(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	if(it != m_playables.end()) {
		destroy_playable((*it).second, (*it).first);
		m_playables.erase(it);
	}
}

void smil_player::pause_playable(const lib::node *n, pause_display d) {
	common::playable *nr = get_playable(n);
	if(nr) nr->pause();
}

void smil_player::resume_playable(const lib::node *n) {
	common::playable *nr = get_playable(n);
	if(nr) nr->resume();
}

common::playable *
smil_player::get_playable(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	return (it != m_playables.end())?(*it).second:0;
}

std::pair<bool, double> 
smil_player::get_dur(const lib::node *n) {
	std::map<const lib::node*, common::playable *>::iterator it = 
		m_playables.find(n);
	common::playable *nr = (it != m_playables.end())?(*it).second:0;
	if(nr) {
		return nr->get_dur();
	}
	nr = create_playable(n);
	m_playables[n] = nr;
	return nr->get_dur();
}

void smil_player::wantclicks_playable(const lib::node *n, bool want) {
	common::playable *nr = get_playable(n);
	if(nr) nr->wantclicks(want);
}

void smil_player::clicked(int n, double t) {
	typedef scalar_arg_callback_event<time_node, q_smil_time> activate_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		activate_event_cb *cb = new activate_event_cb((*it).second, 
			&time_node::raise_activate_event, timestamp);
		schedule_event(cb, 0);
	}
}

void smil_player::stopped(int n, double t) {
	typedef lib::scalar_arg_callback_event<time_node, q_smil_time> eom_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		q_smil_time timestamp(m_root, m_root->get_simple_time());
		eom_event_cb *cb = new eom_event_cb((*it).second, 
			&time_node::eom_update, timestamp);
		schedule_event(cb, 0);
	}
}

void smil_player::started(int n, double t) {
}

void smil_player::on_click(int x, int y) {
	typedef scalar_arg_callback_event<time_node, q_smil_time> activate_event_cb;
	
	// the event instance to propagate
	q_smil_time timestamp(m_root, m_root->get_simple_time());
	
	AM_DBG m_logger->trace("smil_player::on_click(%d, %d) at %ld", x, y, timestamp.second());
	
	// WARNING: The following is test code
	// Does not use mouse regions, z-index etc
	std::map<const lib::node*, common::playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++) {
		playable *pl = ((*it).second);
		int nid = pl->get_cookie();
		renderer *rend = pl->get_renderer();
		if (rend == NULL) continue;
		surface *surf = rend->get_surface();
		if (surf == NULL) continue;
		
		lib::screen_rect<int> rc = surf->get_rect();
		lib::point pt = surf->get_global_topleft();
		rc.translate(pt);
		std::map<int, time_node*>::iterator it2 = m_dom2tn->find(nid);
		if(it2 != m_dom2tn->end() && 
			(*it2).second->want_activate_event() &&
			rc.contains(point(x, y))) {
			// check for area elements
			if((*it).first->get_local_name() == "area") {
				AM_DBG m_logger->trace("***** AREA CLICK");
			}
			activate_event_cb *cb = new activate_event_cb((*it2).second, 
				&time_node::raise_activate_event, timestamp);
			schedule_event(cb, 0);
		}
	}
}

void smil_player::on_char(int ch) {
	typedef std::pair<q_smil_time, int> accesskey;
	typedef scalar_arg_callback_event<time_node, accesskey> accesskey_cb;
	q_smil_time timestamp(m_root, m_root->get_simple_time());
	AM_DBG m_logger->trace("smil_player::on_char(): '%c' [%d] at %ld", char(ch), ch, timestamp.second());
	accesskey ak(timestamp, ch);
	accesskey_cb *cb = new accesskey_cb(m_root, &time_node::raise_accesskey, ak);
	schedule_event(cb, 0);
}

common::playable *
smil_player::create_playable(const lib::node *n) {
	int nid = n->get_numid();
	surface *surf = m_layout_manager->get_surface(n);
	if(true) {
		const char *pid = n->get_attribute("id");
		std::string tag = n->get_local_name();
		AM_DBG m_logger->trace("%s[%s].new_playable  rect%s at %s", tag.c_str(), (pid?pid:"no-id"),
			::repr(surf->get_rect()).c_str(),
			::repr(surf->get_global_topleft()).c_str()
			);
	}
	common::playable *rv = m_rf->new_playable(this, nid, n, m_event_processor);
	// And connect it to the rendering surface
	if (rv) {
		common::renderer *rend = rv->get_renderer();
		if (rend) {
			rend->set_surface(surf);
		}
	}
	return rv;
}

void smil_player::destroy_playable(common::playable *r, const lib::node *n) {
	r->stop();
	long rc = r->get_ref_count();
	r->release();
	if(rc > 1) {
		const char *pid = n->get_attribute("id");
		std::string tag = n->get_local_name();
		//lib::show_message("%s[%s].destroy(): [ref_count=%ld]", tag.c_str(), (pid?pid:"no-id"), (rc-1));
	}
}

