// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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
//#define AM_DBG
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
:
	m_state_engine(0),
	m_doc(doc),
	m_factory(factory),
	m_system(sys),
    m_focus_handler(0),
	m_feedback_handler(0),
	m_animation_engine(0),
	m_root(0),
	m_dom2tn(0),
	m_layout_manager(0),
	m_timer(new timer_control_impl(realtime_timer_factory(), 1.0, false, true)),
	m_event_processor(0),
	m_scheduler(0),
	m_state(common::ps_idle),
	m_cursorid(0),
	m_pointed_node(0),
	m_wait_for_eom_flag(true),
	m_focus(0),
	m_focussed_nodes(new std::set<int>()),
	m_new_focussed_nodes(0),
    m_timer_sync(NULL)
{
	m_logger = lib::logger::get_logger();
	AM_DBG m_logger->debug("smil_player::smil_player(0x%x)", this);
}

void
smil_player::initialize()
{
	assert(m_doc);
	document_loaded(m_doc);

	m_event_processor = event_processor_factory(m_timer);
#ifdef WITH_REMOTE_SYNC
    timer_sync_factory *tsf = m_factory->get_timer_sync_factory();
    if (tsf) {
        m_timer_sync = tsf->new_timer_sync(m_doc);
    }
    if (m_timer_sync) {
        m_timer_sync->initialize(m_timer);
    }
#endif // WITH_REMOTE_SYNC
	create_state_engine();
	// build the layout (we need the top-level layout)
	build_layout();
	// Build the timegraph using the current filter
	build_timegraph();
	m_layout_manager->load_bgimages(m_factory);
}

void
smil_player::terminate()
{
	m_lock.enter();
	m_doc = NULL;
#ifdef WITH_REMOTE_SYNC
    delete m_timer_sync;
#endif
	m_timer->pause();
	std::map<const lib::node*, common::playable *>::iterator it;
	m_scheduler->reset_document();
	m_playables_cs.enter();
	for(it = m_playables.begin();it!=m_playables.end();it++) {
		(*it).second->post_stop();
		long rem = (*it).second->release();
		if (rem > 0)
			m_logger->trace("smil_player::terminate: playable(0x%x) %s still has refcount of %d", (*it).second, (*it).second->get_sig().c_str(), rem);
	}

	// clean up the playable cache as well
	std::map<const std::string, common::playable *>::iterator it_url_based;
	for(it_url_based = m_cached_playables.begin();it_url_based!=m_cached_playables.end();it_url_based++) {
		(*it_url_based).second->post_stop();
		long rem = (*it_url_based).second->release();
		if (rem > 0)
			m_logger->trace("smil_player::terminate: url_based_playable(0x%x) %s still has refcount of %d)", (*it_url_based).second, (*it_url_based).second->get_sig().c_str(), rem);
	}

	m_playables_cs.leave();
	cancel_all_events();
	// XXXJACK Note by Jack and Kees: it may be unsafe to destroy the event processor here,
	// because it could have been passed to renderers, datasources, etc. and these are not
	// cleaned up until later. If we get crashes during
	lib::event_processor *evp = m_event_processor;
	lib::timer_control *tmr = m_timer;
	m_event_processor = NULL;
	m_timer = NULL;
	m_lock.leave();
	delete evp;
	delete tmr;
}

smil_player::~smil_player() {
	m_lock.enter();
	AM_DBG m_logger->debug("smil_player::~smil_player(0x%x)", this);

	// Make sure terminate was called first
	assert(m_doc == NULL);
	assert(m_timer == NULL);
	assert(m_event_processor == NULL);
	delete m_focussed_nodes;
	delete m_new_focussed_nodes;
	delete m_scheduler;
	delete m_animation_engine;
	delete m_layout_manager;
	delete m_dom2tn;
	delete m_root;
//	delete m_doc;
	m_lock.leave();
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
	// If there were any stateChange() events the timegraph builder has collected them.
	// We pass them on to the state engine, so it can fire the required events.
	if (m_state_engine) {
		const std::set<std::string>& state_change_args = tg.get_state_change_args();
		std::set<std::string>::const_iterator i;
		for (i=state_change_args.begin(); i != state_change_args.end(); i++) {
			AM_DBG lib::logger::get_logger()->debug("registering interest in stateChange(%s) with state engine", (*i).c_str());
			m_state_engine->want_state_change((*i).c_str(), this);
		}
	}
	m_root = tg.detach_root();
	m_dom2tn = tg.detach_dom2tn();
	m_scheduler = new scheduler(m_root, m_timer);
}

void smil_player::create_state_engine() {
	assert(m_doc);
	if (m_state_engine) delete m_state_engine;
	lib::node *doc_root = m_doc->get_root();
	if (!doc_root) return;
	lib::node *head = doc_root->get_first_child("head");
	if (!head) return;
	lib::node *state = head->get_first_child("state");
	if (!state) return;
	const char *language = state->get_attribute("language");
	if (!language) language = "http://www.w3.org/TR/1999/REC-xpath-19991116";
	common::state_component_factory *scf = m_factory->get_state_component_factory();
	if (!scf) {
		lib::logger::get_logger()->error(gettext("Document uses state, but no state support configured"));
		return;
	}
	m_state_engine = scf->new_state_component(language);
	if (!m_state_engine) {
		lib::logger::get_logger()->error(gettext("No state support for language %s"), language);
		return;
	}
	m_state_engine->declare_state(state);
	m_state_engine->register_state_test_methods(smil2::test_attrs::get_state_test_methods());
	m_doc->set_state(m_state_engine);
}

void smil_player::schedule_event(lib::event *ev, lib::timer::time_type t, event_priority ep) {
	m_event_processor->add_event(ev, t, ep);
}

// Command to start playback
void smil_player::start() {
	m_lock.enter();
	if(m_state == common::ps_pausing) {
		_resume();
#ifdef XXNOTWITH_GCD_EVENT_PROCESSOR
		//m_event_processor->resume();
#endif
	} else if(m_state == common::ps_idle || m_state == common::ps_done) {
		if(!m_root) build_timegraph();
		if(m_root) {
			if (m_system) m_system->starting(this);
			m_scheduler->start(m_root);
			_update();
		}
	}
	m_lock.leave();
}
void smil_player::empty_playable_cache()
{
	m_playables_cs.enter();
	while (!m_cached_playables.empty()) {
		std::map<const std::string, common::playable *>::iterator it_url_based = m_cached_playables.begin();
		AM_DBG lib::logger::get_logger()->debug("stop: playable still in url-based cache: %s", (*it_url_based).second->get_sig().c_str());
		(*it_url_based).second->post_stop();
        playable_deleted((*it_url_based).second);
		(*it_url_based).second->release();
		m_cached_playables.erase(it_url_based);
	}
	assert(m_cached_playables.empty());
	m_playables_cs.leave();
}

// Command to stop playback
void smil_player::stop() {
	AM_DBG lib::logger::get_logger()->debug("smil_player::stop()");

    empty_playable_cache();
	m_lock.enter();
	if(m_state == common::ps_pausing || m_state == common::ps_playing) {
        m_state = common::ps_done;
		m_timer->pause();
		cancel_all_events();
		m_scheduler->reset_document();
		m_animation_engine->reset();
		done_playback();
	}
	m_lock.leave();
}

// Command to pause playback
void smil_player::pause() {
	m_lock.enter();
	if(m_state == common::ps_playing) {
		m_state = common::ps_pausing;
		m_timer->pause();
#ifdef XXNOTWITH_GCD_EVENT_PROCESSOR
		//m_event_processor->pause();
#endif
#ifdef WITH_REMOTE_SYNC
        if (!uses_external_sync())
#endif//WITH_REMOTE_SYNC
        {
            std::map<const lib::node*, common::playable *>::iterator it;
            m_playables_cs.enter();
            for(it = m_playables.begin();it!=m_playables.end();it++)
                (*it).second->pause();
            m_playables_cs.leave();
        }
	}

	m_lock.leave();
}

// Command to resume playback
void smil_player::resume() {
	m_lock.enter();
	_resume();
#ifdef XXNOTWITH_GCD_EVENT_PROCESSOR
	//m_event_processor->resume();
#endif
	m_lock.leave();
}
// internal implementation resume playback
void smil_player::_resume() {
	if(m_state == common::ps_pausing) {
		m_state = common::ps_playing;
#ifdef WITH_REMOTE_SYNC
        if (!uses_external_sync())
#endif//WITH_REMOTE_SYNC
        {
            std::map<const lib::node*, common::playable *>::iterator it;

            m_playables_cs.enter();
            for(it = m_playables.begin();it!=m_playables.end();it++)
                (*it).second->resume();
            m_playables_cs.leave();
        }
		m_timer->resume();
	}
}

// Started callback from the scheduler
void smil_player::started_playback() {
	// no m_lock.enter();,called in locked state
	m_state = common::ps_playing;
	// m_lock.leave();
	document_started();
}

// Done callback from the scheduler
void smil_player::done_playback() {
	// no m_lock.enter();,called in locked state
	m_state = common::ps_done;
	// m_lock.leave();
	m_timer->pause();
    empty_playable_cache();
	document_stopped();
	if(m_system)
		m_system->done(this);
}

// Request to create a playable for the node.
common::playable *smil_player::create_playable(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)", (void*)n);
	assert(n);

	common::playable *np = NULL;
	bool is_prefetch = n->get_local_name() == "prefetch";
	bool from_cache = false;
	if (n->get_attribute("src")) {
		// It may be in the URL-based playable cache. Let us look.
		std::map<const std::string, common::playable *>::iterator it_url_based =
			m_cached_playables.find(n->get_url("src").get_url());
		if (it_url_based != m_cached_playables.end()) {
			from_cache = true;
			np = (*it_url_based).second;
			m_playables_cs.enter();
			m_cached_playables.erase(it_url_based);
			// In case of prefetch, there may be more than one playable (for example, one for audio,
			// the second one for prefetch) stored in the map, so this assert is not true any more.
			//assert(m_cached_playables.empty());
			m_playables_cs.leave();
			
            // xxxx copied code from _new_playable to call set_surface()
            assert(np);
			surface *surf = m_layout_manager->get_surface(n);
			assert(surf);	// XXXJACK: at least, I think it cannot be NULL....
			common::renderer *rend = np->get_renderer();

			// XXXJACK: Dirty hack, for now: we don't want prefetch to render to a surface so we zap it. Need to fix.
			if (is_prefetch) { 
				surf = NULL;
			}
			AM_DBG lib::logger::get_logger()->debug("smil_plager::create_playable(0x%x)%s: cached playable 0x%x, renderer 0x%x, surface 0x%x", n, n->get_sig().c_str(), np, rend, surf);

			if (rend && surf) {
				// XXXJACK: if rend->get_surface() == surf, couldn't we skip re-setting it?
				if (rend->get_surface() != NULL)
					rend->get_surface()->renderer_done(rend);
				rend->set_surface(surf);
                // XXXJACK Because true animation (and setting through state) doesn't always
                // work for regpoint/regalign we re-set it here. Can go once we implement
                // animating regpoint/align.
                const alignment *align = m_layout_manager->get_alignment(n);
                rend->set_alignment(align);			}
		} else {
			AM_DBG lib::logger::get_logger()->debug("smil_plager::create_playable(0x%x)%s: no cached playable", n, n->get_sig().c_str());
		}
	}
	if( np == NULL ) {
		np = _new_playable(n);
		AM_DBG lib::logger::get_logger()->debug("smil_plager::create_playable(0x%x) _new_playable 0x%x", (void*)n, (void*)np);
		AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.enter", (void*)n);
		m_playables_cs.enter();
		m_playables[n] = np;
		m_playables_cs.leave();
		AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.leave", (void*)n);
	} else {
		AM_DBG lib::logger::get_logger()->debug("smil_plager::create_playable(0x%x), prior playable is found 0x%x", (void*)n, (void*)np);
		AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.enter", (void*)n);
		m_playables_cs.enter();
		m_playables[n] = np;
		m_playables_cs.leave();
		AM_DBG lib::logger::get_logger()->debug("smil_player::create_playable(0x%x)cs.leave", (void*)n);
	}
	if (np) {
		playable_started(np, n, from_cache ? "cached" : "");
		// Update the context info of np, for example, clipbegin, clipend, and cookie according to the node
		np->init_with_node(n);
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
	if (np == NULL) {
		stopped(n->get_numid(), t);
		return;
	}
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
	common::playable *np = _get_playable(n);
	if (np) np->seek(t);
}

// Request to start a transition of the playable of the node.
void smil_player::start_transition(const lib::node *n, const lib::transition_info *trans, bool in) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::start_transition(%s, -x%x, in=%d)", n->get_sig().c_str(), trans, in);
	std::map<const lib::node*, common::playable *>::iterator it =
		m_playables.find(n);
	common::playable *np = (it != m_playables.end())?(*it).second:0;
	if(!np) {
		AM_DBG m_logger->debug("smil_player::start_transition: node %s has no playable", n->get_sig().c_str());
		return;
	}
	common::renderer *rend = np->get_renderer();
	if (!rend) {
		m_logger->trace("smil_player::start_transition: node %s has transition but is not visual", n->get_sig().c_str());
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
	AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable(%s)", n->get_sig().c_str());
	if (n == m_focus) {
		m_focus = NULL;
		highlight(n, false);
		node_focussed(NULL);
	}
	AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable(0x%x)cs.enter", (void*)n);
	m_playables_cs.enter();
	std::map<const lib::node*, common::playable *>::iterator it =
		m_playables.find(n);
	std::pair<const lib::node*, common::playable*> victim((const lib::node*)NULL,(common::playable*)NULL);
	if(it != m_playables.end()) {
		victim = *it;
		m_playables.erase(it);
	}
	m_playables_cs.leave();
    // XXXKEES found that AmbulantPlayer_gtk NYC-StateTest.smil always asserts here
    // assert(victim.second); // Jack thinks we should always have a playable when we get here. Remove assert if untrue:-)

	if (victim.second == NULL) return;

	// There are now three possibilities:
	// 1. Not cachable, or no URL. Destroy. 
	// 2. Cachable, fill=continue. Store in cache, don't stop playback.
	// 3. Cachable, no fill=continue. Store in cache, but stop playback.
    // Also, we don't cache if we are stopping anyway.
	bool can_cache = (m_state != common::ps_done);
	bool must_post_stop;

	must_post_stop = !victim.second->stop();
	if (n->get_attribute("src") == NULL) {
		can_cache = false;
	} else {
		// See if there is one in the cache already
		std::map<const std::string, common::playable *>::iterator it_url_based =
		m_cached_playables.find((victim.first->get_url("src")).get_url());
		common::playable *np = (it_url_based != m_cached_playables.end())?(*it_url_based).second:0;
		if( np != NULL ) {
			lib::logger::get_logger()->debug("smil_player::stop_playable: destroying, cache entry occupied for %s", victim.first->get_url("src").get_url().c_str());
			// xxxbo: in case of the on playing node has the same src with the prefetch one, 
			// we should cache the prefetch one in any case and remove the one which are playing
			//can_cache = false; 
			m_playables_cs.enter();
			m_cached_playables.erase(it_url_based);
			m_playables_cs.leave();
			np->release();
		}
	}
	if (can_cache && must_post_stop) {
		// If we are playing a fill=ambulant:continue node we want to continue playback for a while,
		// so we don't call post_stop. The scheduler will arrange for it being called in a short while, unless
		// this renderer is reused in the mean time.
		const char * fb = n->get_attribute("fill");
		if (fb != NULL && strcmp(fb, "ambulant:continue") == 0) {
			must_post_stop = false;
		}
	}
	if (must_post_stop) {
		victim.second->post_stop();
	}
	if (can_cache) {
		AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable: cache %s renderer", victim.first->get_sig().c_str());
		playable_cached(victim.second);
		m_playables_cs.enter();
		m_cached_playables[(victim.first->get_url("src")).get_url()] = victim.second;
		m_playables_cs.leave();
		//xxxbo: if this playable is created for prefetch, we don't destroy it.
		//xxxbo: we use node id as the index to find the corresponding time_node in time graph for each node in dom tree.
		std::map<int, time_node*>::iterator it2 = m_dom2tn->find(victim.first->get_numid());
		if(it2 != m_dom2tn->end() && !(*it2).second->is_prefetch())	 {
			// Add a event to destroy this playable on next 20000 microseconds, however, Jack thinks there is another option...
			typedef std::pair<const lib::node*, common::playable*> destroy_event_arg;
			lib::event *destroy_event = new lib::scalar_arg_callback<smil_player, destroy_event_arg>(this, &smil_player::destroy_playable_in_cache, victim);
			// XXXJACK: The following code is possibly incorrect. The assumption behind the design for deleting the
			// playable with a timeout is that any event scheduled with delta_t=0 will be scheduled before the destruction.
			// However, this may not be true, because the delta_t can be adjusted by the event processor.
			// For now, we put the event in the low-priority queue. Kees thinks that this is good enough.
			// Jack is not sure, but anything that forestalls me diving into this code is a welcome excuse:-)
			AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable: schedule destructor in 20ms for %s", victim.first->get_sig().c_str());
			m_event_processor->add_event(destroy_event, 20, lib::ep_low);
		} else {
			AM_DBG lib::logger::get_logger()->debug("smil_player::stop_playable: cache %s renderer without destructor callback", victim.first->get_sig().c_str());
		}
		return;
	}
	_destroy_playable(victim.second, victim.first);
}

// Request to pause the playable of the node.
void smil_player::pause_playable(const lib::node *n, pause_display d) {
	common::playable *np = _get_playable(n);
	AM_DBG lib::logger::get_logger()->debug("smil_player::pause_playable(0x%x) for %s", np, n->get_sig().c_str());
	if(np) np->pause(d);
}

// Request to resume the playable of the node.
void smil_player::resume_playable(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_player::resume_playable(%s)", n->get_sig().c_str());
	common::playable *np = _get_playable(n);
	if(np) np->resume();
}

// Query the node's playable for its duration.
common::duration
smil_player::get_dur(const lib::node *n) {
	const common::duration not_available(false, 0.0);
	std::map<const lib::node*, common::playable *>::iterator it = m_playables.find(n);
	common::playable *np = (it != m_playables.end()) ? (*it).second : 0;
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
	common::playable *np = _get_playable(n);
	if(np) np->wantclicks(want);
}

// Playable notification for a click event.
void
smil_player::clicked(int n, double t) {
	AM_DBG m_logger->debug("smil_player::clicked(%d, %f)", n, t);
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if (it == m_dom2tn->end()) return;
	time_node *tn = (*it).second;
	if(tn->wants_activate_event()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		async_arg aa(tn, timestamp);
		async_cb *cb = new async_cb(this, &smil_player::clicked_async, aa);
		schedule_event(cb, 0, ep_high);
	}
	// If there was an <a> around the node, this will be represented in the time tree as
	// the last child of this node. We pass the event along, if needed.
	tn = tn->last_child();
	if (tn == NULL || !tn->is_a()) return;
	time_node::value_type root_time = m_root->get_simple_time();
	m_scheduler->update_horizon(root_time);
	q_smil_time timestamp(m_root, root_time);
	async_arg aa(tn, timestamp);
	async_cb *cb = new async_cb(this, &smil_player::clicked_async, aa);
	schedule_event(cb, 0, ep_high);
}

void
smil_player::clicked_async(async_arg aa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
#ifdef WITH_REMOTE_SYNC
	if (m_timer_sync) {
		const lib::node *n = aa.first->dom_node();
		lib::timer::time_type t = 0; // aa.second.blabla
		m_timer_sync->clicked(n, t);
    }
#endif
	aa.first->raise_activate_event(aa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

void
smil_player::clicked_external(lib::node *n, lib::timer::time_type t)
{
	clicked(n->get_numid(), (double)t/1000000.0);
}

void
smil_player::before_mousemove(int cursorid)
{
	m_cursorid = cursorid;
	delete m_new_focussed_nodes;
	m_new_focussed_nodes = new std::set<int>();
	AM_DBG m_logger->debug("smil_player(0x%x)::before_mousemove(%d) m_new_focussed_nodes=0x%x", this, cursorid, m_new_focussed_nodes);
}

int
smil_player::after_mousemove()
{
	std::set<int>::iterator i;

	m_pointed_node = 0;

	if (m_new_focussed_nodes == NULL) {
		// This "cannot happen", but it turns out it can:-)
		// The scenario is that if a window shows up or disappear during
		// a mouse move, depending on the GUI toolkit it can happen.
		m_logger->debug("after_mousemove: called without corresponding before_mousemove\n");
		return m_cursorid;
	}
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
			async_arg aa((*it).second, timestamp);
			async_cb *cb = new async_cb(this, &smil_player::mouse_outofbounds_async, aa);
			schedule_event(cb, 0, ep_high);
		}
		if (tn->wants_focusout_event()) {
			AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusOutEvent", (void*)tn);
			time_node::value_type root_time = m_root->get_simple_time();
			m_scheduler->update_horizon(root_time);
			q_smil_time timestamp(m_root, root_time);
			async_arg aa((*it).second, timestamp);
			async_cb *cb = new async_cb(this, &smil_player::focus_outofbounds_async, aa);
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
				async_arg aa((*it).second, timestamp);
				async_cb *cb = new async_cb(this, &smil_player::mouse_inbounds_async, aa);
				schedule_event(cb, 0, ep_high);
		}
		if (tn->wants_focusin_event()) {
				AM_DBG m_logger->debug("smil_player::pointed: schedule 0x%x.focusInEvent", (void*)tn);
				time_node::value_type root_time = m_root->get_simple_time();
				m_scheduler->update_horizon(root_time);
				q_smil_time timestamp(m_root, root_time);
				async_arg aa((*it).second, timestamp);
				async_cb *cb = new async_cb(this, &smil_player::focus_inbounds_async, aa);
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

void
smil_player::mouse_outofbounds_async(async_arg aa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	aa.first->raise_outofbounds_event(aa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

void
smil_player::focus_outofbounds_async(async_arg aa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	aa.first->raise_focusout_event(aa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

void
smil_player::mouse_inbounds_async(async_arg aa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	aa.first->raise_inbounds_event(aa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

void
smil_player::focus_inbounds_async(async_arg aa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	aa.first->raise_focusin_event(aa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

// Playable notification for a point (mouse over) event.
void
smil_player::pointed(int n, double t) {
	AM_DBG m_logger->debug("smil_player(0x%x)::pointed(%d, %f) m_new_focussed_nodes=0x%x", this, n, t, m_new_focussed_nodes);
	if (m_new_focussed_nodes == NULL) {
		// This "cannot happen", but it turns out it can:-)
		// The scenario is that if a window shows up or disappear during
		// a mouse move, depending on the GUI toolkit it can happen.
		AM_DBG m_logger->debug("smil_player::pointed: m_new_focussed_nodes==NULL, ignoring");
		return;
	}
	m_new_focussed_nodes->insert(n);
}

// Playable notification for a start event.
void
smil_player::started(int n, double t) {
	//AM_DBG m_logger->debug("smil_player::started(%d, %f)", n, t);
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		AM_DBG m_logger->debug("smil_player::started: node %s is started", (*it).second->get_sig().c_str());
		async_arg aa((*it).second, timestamp);
		async_cb *cb = new async_cb(this, &smil_player::started_async, aa);
		schedule_event(cb, 0, ep_high);
	}
}

void
smil_player::started_async(async_arg aa) {
	m_scheduler->lock();
	AM_DBG m_logger->debug("smil_player::started_async: node %s is started_async", aa.first->get_sig().c_str());
	aa.first->on_bom(aa.second);
	m_scheduler->unlock();
}

// Playable notification for a stop event.
void
smil_player::stopped(int n, double t) {
	//AM_DBG m_logger->debug("smil_player::stopped(%d, %f) roottime=%d", n, t, m_root->get_simple_time());
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end() && !(*it).second->is_discrete()) {
		time_node::value_type root_time = m_root->get_simple_time();
		// XXXJACK m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		time_node* tn = (*it).second;
		AM_DBG m_logger->debug("smil_player::stopped(%d), want_on_eom()=%d", n, tn->want_on_eom());
		AM_DBG m_logger->debug("smil_player::stopped: node %s is stopped", tn->get_sig().c_str());
		if (tn->want_on_eom()) {
			async_arg aa((*it).second, timestamp);
			async_cb *cb = new async_cb(this, &smil_player::stopped_async, aa);
//XXXJACK			
			schedule_event(cb, 0, ep_high);
			// Temporary workaround: we would like the stopped-async call to happen after the started_async.
			// The timeout here is a gross hack to try and make that happen more often.
			//schedule_event(cb, 10000, ep_high);
		}
	}
}

void
smil_player::stopped_async(async_arg aa) {
	m_scheduler->lock();
	AM_DBG m_logger->debug("smil_player::stopped_async: node %s is stopped_async", aa.first->get_sig().c_str());
	aa.first->on_eom(aa.second);
	m_scheduler->unlock();
}
// Playable notification for a transition stop event.
void
smil_player::transitioned(int n, double t) {
	// remove fill effect for nodes specifing fill="transition"
	// and overlap with n
	AM_DBG m_logger->debug("smil_player::transitioned(%d, %f)", n, t);
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		async_arg aa((*it).second, timestamp);
		async_cb *cb = new async_cb(this, &smil_player::transitioned_async, aa);
		schedule_event(cb, 0, ep_high);
	}
}

void
smil_player::transitioned_async(async_arg aa) {
	m_scheduler->lock();
	aa.first->on_transitioned(aa.second);
	m_scheduler->unlock();
}

// Playable notification for a transition stop event.
void
smil_player::marker_seen(int n, const char *name, double t) {
	AM_DBG m_logger->debug("smil_player::marker_seen(%d, \"%s\", %f)", n, name, t);
	typedef std::pair<q_smil_time, std::string> marker_seen_arg;
	typedef lib::scalar_arg_callback_event<time_node, marker_seen_arg> marker_seen_event_cb;
	std::map<int, time_node*>::iterator it = m_dom2tn->find(n);
	if(it != m_dom2tn->end()) {
		time_node::value_type root_time = m_root->get_simple_time();
		m_scheduler->update_horizon(root_time);
		q_smil_time timestamp(m_root, root_time);
		async_string_arg asa((*it).second, std::make_pair(timestamp,name));
		async_string_cb *cb = new async_string_cb(this, &smil_player::marker_seen_async, asa);
		schedule_event(cb, 0, ep_high);
	}
}

void
smil_player::marker_seen_async(async_string_arg asa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	asa.first->raise_marker_event(asa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

#if 0
// Playable notification for a stall event.
void smil_player::playable_stalled(const playable *p, const char *reason) {
	if (m_feedback_handler) {
		m_feedback_handler->playable_stalled(p, reason);
	}
}

// Playable notification for an unstall event.
void smil_player::playable_unstalled(const playable *p, double t) {
	if (m_feedback_handler) {
		m_feedback_handler->playable_unstalled(p);
	}
}

void smil_player::playable_started(const playable *p, const lib::node *n, const char *comment) {
	if (m_feedback_handler) {
		m_feedback_handler->playable_unstalled(p, n, comment);
	}
}

void smil_player::playable_resource(const playable *p, const char *resource, long amount) {
	if (m_feedback_handler) {
		m_feedback_handler->playable_resource(p, resource, amount);
	}
}
#endif

// UI notification for a char event.
void smil_player::on_char(int ch) {
	// First check for an anchor node with accesskey attribute
	AM_DBG m_logger->debug("smil_player::on_char(): '%c' [%d]", char(ch), ch);
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
	time_node::value_type root_time = m_root->get_simple_time();
	m_scheduler->update_horizon(root_time);
	q_smil_time timestamp(m_root, root_time);
	AM_DBG m_logger->debug("smil_player::on_char(): '%c' [%d] at %ld", char(ch), ch, timestamp.second());
	async_int_arg aia(m_root, std::make_pair(timestamp, ch));
	async_int_cb *cb = new async_int_cb(this, &smil_player::on_char_async, aia);
	schedule_event(cb, 0, ep_high);
}

void
smil_player::on_char_async(async_int_arg aia) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	int ch = aia.second.second;
	AM_DBG m_logger->debug("smil_player::on_char_async(): '%c' [%d]", char(ch), ch);
	aia.first->raise_accesskey(aia.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
}

// UI notification for a char event.
void smil_player::on_state_change(const char *ref) {
	time_node::value_type root_time = m_root->get_simple_time();
	m_scheduler->update_horizon(root_time);
	q_smil_time timestamp(m_root, root_time);
	AM_DBG m_logger->debug("smil_player::state_change('%s'): at %ld", ref, timestamp.second());
	async_string_arg asa(m_root, std::make_pair(timestamp, ref));
	async_string_cb *cb = new async_string_cb(this, &smil_player::on_state_change_async, asa);
	schedule_event(cb, 0, ep_high);
}

void
smil_player::on_state_change_async(async_string_arg asa) {
//XXXJACK thinks this isn't needed	m_scheduler->lock();
	asa.first->raise_state_change(asa.second);
//XXXJACK thinks this isn't needed	m_scheduler->unlock();
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
smil_player::_new_playable(const lib::node *n) {
	assert(n);
	int nid = n->get_numid();
	std::string tag = n->get_local_name();
	const char *pid = n->get_attribute("id");

	surface *surf = m_layout_manager->get_surface(n);
	AM_DBG m_logger->debug("%s[%s]._new_playable 0x%x cookie=%d	 rect%s at %s", tag.c_str(), (pid?pid:"no-id"),
		(void*)n, nid,
		::repr(surf->get_rect()).c_str(),
		::repr(surf->get_global_topleft()).c_str());
	common::playable_factory *pf = m_factory->get_playable_factory();
	assert(pf);
	common::playable *np = pf->new_playable(this, nid, n, m_event_processor);
	assert(np);
	// And connect it to the rendering surface
	if (np) {
		common::renderer *rend = np->get_renderer();

		if (rend) {
			AM_DBG m_logger->debug("smil_player::_new_playable: surface	 set,rend = 0x%x, np = 0x%x", (void*) rend, (void*) np);
			rend->set_surface(surf);
			const alignment *align = m_layout_manager->get_alignment(n);
			rend->set_alignment(align);
		} else {
			AM_DBG m_logger->debug("smil_player::_new_playable: surface not set because rend == NULL");
		}
	} else {
		lib::logger::get_logger()->trace("%s: could not create playable", n->get_sig().c_str());
	}
	return np;
}

// Destroys the playable of the node (checkpoint).
void smil_player::_destroy_playable(common::playable *np, const lib::node *n) {
	assert(np);
	assert(n);
	AM_DBG {
		std::string tag = n->get_local_name();
		const char *pid = n->get_attribute("id");

		m_logger->debug("%s[%s]._destroy_playable 0x%x", tag.c_str(), (pid?pid:"no-id"), np);
	}
	playable_deleted(np);
	long rem = np->release();
	if (rem > 0)
		m_logger->debug("smil_player::_destroy_playable: playable(0x%x) %s still has refcount of %ld", np, np->get_sig().c_str(), rem);
}

void smil_player::destroy_playable_in_cache(std::pair<const lib::node*, common::playable*> victim) {
	AM_DBG m_logger->debug("smil_player::destroy_playable_in_cache: playable(0x%x) %s", victim.second, victim.second->get_sig().c_str());
	m_lock.enter();
	// If we are already terminating do nothing: the document may be gone
	if (m_doc == NULL) {
		m_lock.leave();
		return;
	}
	assert(victim.first);
	assert(victim.second);

	std::string url = victim.first->get_url("src").get_url();
	m_playables_cs.enter();
	std::map<const std::string, common::playable *>::iterator it_url_based = m_cached_playables.find(url);
	if (it_url_based != m_cached_playables.end()) {
		if ((*it_url_based).second != victim.second) {
			AM_DBG lib::logger::get_logger()->debug("smil_player::destroy_playable_in_cache: cache has different playable for %s, assuming %s is reused", url.c_str(), victim.first->get_sig().c_str());
			m_playables_cs.leave();
			m_lock.leave();
			return;
		}
		m_cached_playables.erase(it_url_based);
		m_playables_cs.leave();
		AM_DBG lib::logger::get_logger()->debug("smil_player::destroy_playable_in_cache: stop the playable in the cache for %s", victim.first->get_sig().c_str());
		victim.second->post_stop();
		playable_deleted(victim.second);
		long rem = victim.second->release();
		if (rem > 0) m_logger->debug("smil_player::destroy_playable_in_cache: playable(0x%x) %s still has refcount of %ld", victim.second, victim.second->get_sig().c_str(), rem);
	} else {
		m_playables_cs.leave();
		// Note that this is not an error, on the contrary: it could be that the playable has been reused.
		AM_DBG m_logger->debug("smil_player::destroy_playable_in_cache: playable for %s no longer in cache", url.c_str());
	}
	m_lock.leave();
}

void smil_player::show_link(const lib::node *n, const net::url& href,
	src_playstate srcstate, dst_playstate dststate, const char *target)
{
	AM_DBG lib::logger::get_logger()->debug("show_link(\"%s\"), srcplaystate=%d, dstplaystate=%d", href.get_url().c_str(), (int)srcstate, (int)dststate);
	net::url our_url(m_doc->get_src_url());
	if(srcstate == src_replace && href.same_document(our_url)) {
		// This is an internal hyperjump
		std::string anchor = href.get_ref();
		const lib::node *tnode = m_doc->get_node(anchor);
		if(tnode) {
			goto_node(tnode);
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
#ifdef WITH_OVERLAY_WINDOW
		if ( target && strcmp(target, "overlay") == 0) {
			m_system->aux_open(href);
		} else
#endif
		{
			if (target)
				lib::logger::get_logger()->trace("show_link: ignoring unknown target \"%s\"", target);
			m_system->open(href, dststate == dst_play, to_replace);
		}
	}
}

bool smil_player::goto_node(const lib::node *target)
{
	AM_DBG lib::logger::get_logger()->debug("goto_node(%s)", target->get_sig().c_str());

	std::map<int, time_node*>::iterator it = m_dom2tn->find(target->get_numid());

	if(it != m_dom2tn->end()) {
		bool already_running = m_root->is_active();
		if (!already_running) {
			if (m_system) m_system->starting(this);
		}
		m_scheduler->start((*it).second);
		if (!already_running) {
			_update();
		}
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
	m_lock.enter();
	_update();
	m_lock.leave();
}

void smil_player::_update() {
	if(m_scheduler && m_root && m_root->is_active()) {
		lib::timer::time_type dt = m_scheduler->exec();
		if(m_root->is_active()) {
			lib::event *update_event = new lib::no_arg_callback<smil_player>(this,
				&smil_player::update);
			m_event_processor->add_event(update_event, dt, lib::ep_high);
		} else {
			m_scheduler->reset_document();
			m_animation_engine->reset();
		}
	}
}
