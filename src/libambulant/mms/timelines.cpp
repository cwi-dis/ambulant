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

#include "ambulant/mms/timelines.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {
namespace mms {

class dead_playable_class : public common::playable {
  public:
  	dead_playable_class()
  	:	common::playable() {}
  	
	void stop() {}
	void start(double t) {};
	void pause() {};
	void resume() {};
	void seek(double t) {};
	void wantclicks(bool want) {};
	void preroll(double t1, double t2, double t3) {};
	std::pair<bool, double> get_dur() { return std::pair<bool, double>(false, 0.0); }
	virtual const int& get_cookie() const { return 0; }
	long add_ref() {return 1;};
	long release() {return 1;};
	long get_ref_count() const { return 1;}
};

dead_playable_class dead_playable;

namespace detail {
/* ---------------- */
// The active_int_action subclass stores an internal timeline action.
// Firing such an action has no side effects, it only triggers a timeline
// dependency, possibly after a timeout.

class active_int_action : public active_action {
  public:
	active_int_action(int dependency)
	:	m_dependency(dependency),
		m_timeout(0) {};

	active_int_action(int dependency, const timeline_delay& delay)
	:	m_dependency(dependency),
		m_timeout(delay.timeout()) {};
	
	void fire(active_timeline * const parent) const;
	void delayed_fire(active_timeline * const parent) const;
#ifndef AMBULANT_NO_IOSTREAMS
	void to_stream(std::ostream& os) const;
#endif
  private:
    const int m_dependency;
    const int m_timeout;
};

void
active_int_action::delayed_fire(active_timeline * const parent) const
{
	lib::logger::get_logger()->error("timelines.active_int_action::delayed_fire called!");
#ifndef AMBULANT_NO_ABORT
	abort();
#endif
}

void
active_int_action::fire(active_timeline * const parent) const
{
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "fire_int " << m_dependency << " timeout=" << m_timeout << std::endl;
#endif
	
	typedef lib::scalar_arg_callback<active_timeline, detail::dependency_callback_arg> mycallback;
	lib::event *e = new mycallback(parent, &active_timeline::dependency_callback, m_dependency);
	parent->m_event_processor->add_event(e, m_timeout, lib::event_processor::high);
}

#ifndef AMBULANT_NO_IOSTREAMS
void
active_int_action::to_stream(std::ostream& os) const
{
	os << "active_int_action(" << m_dependency << ", delay=" << m_timeout << ")";
}
#endif

/* ---------------- */
// The active_ext_action class stores an external timeline action,
// an action that only causes a side-effect and has no direct
// influence on the timeline scheduler.
class active_ext_action : public detail::active_action {
  public:
	active_ext_action(int action_index, timeline_event_class cls, int node_index)
	:	m_action_index(action_index),
		m_cls(cls),
		m_node_index(node_index) {};
		
	void fire(active_timeline * const parent) const;
#ifndef AMBULANT_NO_IOSTREAMS
	void to_stream(std::ostream& os) const;
#endif
  protected:
  	const int m_action_index;
    const timeline_event_class m_cls;
    const int m_node_index;
    
};

void
active_ext_action::fire(active_timeline * const parent) const
{
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "fire_ext " << m_action_index << std::endl;
#endif
	
	typedef lib::scalar_arg_callback<active_timeline, detail::dependency_callback_arg> mycallback;
	lib::event *e = new mycallback(parent, &active_timeline::ext_dependency_callback, m_action_index);
	parent->m_event_processor->add_event(e, 0, lib::event_processor::low);
}

#ifndef AMBULANT_NO_IOSTREAMS
void
active_ext_action::to_stream(std::ostream& os) const
{
	os << "active_ext_action(" << m_cls << ", node_index=" << m_node_index << ")";
}
#endif

// Three classes that subclass active_ext_action to provide the
// correct side-effects on fire()
class active_preroll_action : public detail::active_ext_action {
  public:
    active_preroll_action(int action_index, timeline_event_class cls, int node_index)
	: active_ext_action(action_index, cls, node_index) {};
	
	void delayed_fire(active_timeline * const parent) const
	{
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "delayed_fire_ext_preroll " << m_cls << ", node index=" << m_node_index << std::endl;
#endif
		parent->ext_preroll(m_node_index);
	};
};	
	
class active_startplay_action : public detail::active_ext_action {
  public:
    active_startplay_action(int action_index, timeline_event_class cls, int node_index)
	:	active_ext_action(action_index, cls, node_index) {};
	
	void delayed_fire(active_timeline * const parent) const
	{
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "delayed_fire_ext_startplay " << m_cls << ", node index=" << m_node_index << std::endl;
#endif
		parent->ext_play(m_node_index);
	};
};	
	
class active_stopplay_action : public detail::active_ext_action {
  public:
    active_stopplay_action(int action_index, timeline_event_class cls, int node_index)
	: active_ext_action(action_index, cls, node_index) {};
	
	void delayed_fire(active_timeline * const parent) const
	{
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "delayed_fire_ext_stopplay " << m_cls << ", node index=" << m_node_index << std::endl;
#endif
		parent->ext_stop(m_node_index);
	};
};	
	
} // namespace detail
/* ---------------- */

void
detail::timeline_rhs_event::build_action(detail::active_action_vector& actions,
	detail::dependency_index_generator& indexer, int node_index)
{
		// First handle the special cases for external events
	if ( m_what == START_PREROLL_RENDERER ) {
		detail::active_ext_action *act = new active_preroll_action(actions.size(), m_what, node_index);
		actions.push_back(act);
	} else
	if ( m_what == START_PLAY_RENDERER ) {
		detail::active_ext_action *act = new active_startplay_action(actions.size(), m_what, node_index);
		actions.push_back(act);
	} else
	if ( m_what == STOP_PLAY_RENDERER ) {
		detail::active_ext_action *act = new active_stopplay_action(actions.size(), m_what, node_index);
		actions.push_back(act);
	} else 
	if ( m_what == DELAY) {
		int index = indexer.get_index(*this);
		detail::active_int_action *act = new active_int_action(index, *m_delay);
		actions.push_back(act);
	} else {
		int index = indexer.get_index(*this);
		detail::active_int_action *act = new active_int_action(index);
		actions.push_back(act);
	}
	
}


/* ---------------- */
int
detail::dependency_index_generator::set_index(detail::timeline_event_vector &evv)
{
	detail::timeline_event_vector::iterator i;
	
	for(i=evv.begin(); i<evv.end(); i++) {
		if (m_index.find(*i) != m_index.end()) {
#ifndef AMBULANT_NO_IOSTREAMS
			std::cout << "dependency_index_generator: duplicate: " << *i << std::endl; // XXXX
#endif
#ifndef AMBULANT_NO_ABORT
			abort();
#endif
		}
		m_index[*i] = m_cur_end_pos;
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "Inserted dependency " << *i << " at " << m_cur_end_pos << std::endl;
#endif
	}
	return m_cur_end_pos++;
}

int
detail::dependency_index_generator::get_index(detail::timeline_event &ev)
{
	std::map<timeline_event, int>::iterator i;
	i = m_index.find(ev);
	if (i == m_index.end()) {
		return -1;
	}
	return m_index[ev];
}

/* ---------------- */
void 
timeline_node_transition::add_lhs(timeline_event_class what)
{
	m_lhs.push_back(detail::timeline_event(what, (lib::node *)NULL));
}

void 
timeline_node_transition::add_lhs(timeline_event_class what, const lib::node *direct_object)
{
	m_lhs.push_back(detail::timeline_event(what, direct_object));
}

void 
timeline_node_transition::add_lhs(timeline_event_class what, const timeline_delay *direct_object)
{
	m_lhs.push_back(detail::timeline_event(what, direct_object));
}

void 
timeline_node_transition::add_rhs(timeline_event_class what, const lib::node *direct_object)
{
	m_rhs.push_back(detail::timeline_rhs_event(what, direct_object));
}

void 
timeline_node_transition::add_rhs(timeline_event_class what, const timeline_delay *direct_object)
{
	m_rhs.push_back(detail::timeline_rhs_event(what, direct_object));
}

void
timeline_node_transition::build_index(detail::dependency_index_generator& indexer)
{
	m_event_index = indexer.set_index(m_lhs);
}

void 
timeline_node_transition::build_actions(detail::active_action_vector& actions,
		detail::dependency_index_generator& indexer, int node_index)
{
	detail::timeline_rhs_event_vector::iterator i;
	
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "build actions for " << this << std::endl;
#endif
	m_action_begin = actions.size();
	for(i=m_rhs.begin(); i<m_rhs.end(); i++)
		i->build_action(actions, indexer, node_index);
	m_action_end = actions.size();
}

void 
timeline_node_transition::build_dependencies(detail::active_dependency_vector& dependencies)
{
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "build dependencies for " << this << std::endl;
#endif
	detail::active_dependency act = detail::active_dependency(m_lhs.size(), m_action_begin, m_action_end);
	dependencies.push_back(act);
}

#ifndef AMBULANT_NO_IOSTREAMS
void 
timeline_node_transition::dump(std::ostream& os)
{
	detail::timeline_event_vector::iterator i;
	detail::timeline_rhs_event_vector::iterator j;
	bool first = true;
	
	for (i=m_lhs.begin(); i<m_lhs.end(); i++) {
		if (!first)
			os << " && ";
		first = false;
		os << (*i);
	}
	os << ":" << std::endl;
	first = true;
	for (j=m_rhs.begin(); j<m_rhs.end(); j++) {
		if (!first)
			os << "," << std::endl;
		first = false;
		os << "\t\t" << (*j);
	}
	os << std::endl;
}
#endif
		
/* ---------------- */

timeline_node_transition *
timeline_node::add_transition()
{
	timeline_node_transition *rv = new timeline_node_transition();
	m_transitions.push_back(rv);
	return rv;
}

#ifndef AMBULANT_NO_IOSTREAMS
void 
timeline_node::dump(std::ostream& os, int &num)
{
	std::vector<timeline_node_transition*>::iterator i;
	
	for (i=m_transitions.begin(); i<m_transitions.end(); i++) {
		os << num++ << "\t";
		(**i).dump(os);
	}
	os << std::endl;
}
#endif


#ifndef AMBULANT_NO_IOSTREAMS
void 
timeline_node::dump(std::ostream& os)
{
	std::vector<timeline_node_transition*>::iterator i;
	
	for (i=m_transitions.begin(); i<m_transitions.end(); i++) {
		(**i).dump(os);
	}
	os << std::endl;
}
#endif

void
timeline_node::build_index(detail::dependency_index_generator& indexer)
{
	std::vector<timeline_node_transition*>::iterator trans;
	
	for(trans=m_transitions.begin(); trans<m_transitions.end(); trans++) {
		(*trans)->build_index(indexer);
	}
}

void 
timeline_node::build_actions(detail::active_action_vector& actions,
		detail::dependency_index_generator& indexer, int node_index)
{
	std::vector<timeline_node_transition*>::iterator trans;
	
	for(trans=m_transitions.begin(); trans<m_transitions.end(); trans++) {
		(*trans)->build_actions(actions, indexer, node_index);
	}
}

int
timeline_node::get_playdone_index(detail::dependency_index_generator& indexer, int node_index)
{
	detail::timeline_event done_event = detail::timeline_event(DONE_PLAY_RENDERER, 
			static_cast<detail::event_uid>(m_node));
	return indexer.get_index(done_event);
}

void 
timeline_node::build_dependencies(detail::active_dependency_vector& dependencies)
{
	std::vector<timeline_node_transition*>::iterator trans;
	
	for(trans=m_transitions.begin(); trans<m_transitions.end(); trans++) {
		(*trans)->build_dependencies(dependencies);
	}
}

/* ---------------- */

const int START_PREROLL_TIMELINE_INDEX = 0;
const int START_PLAY_TIMELINE_INDEX = 1;
const int DONE_PLAY_TIMELINE_INDEX = 2;

passive_timeline::passive_timeline(lib::node *rootnode)
:	m_rootnode(rootnode),
	m_is_built(0),
	m_playdone_indices(NULL)

{
	// Initial transitions. Note that there is interaction
	// between this code and the preroll(), start() and methods
	// of the active_timeline: those expect the events to
	// be at locations 0 and 1 of the dependecy vector.
	// This may need rethinking at some point
	
	timeline_node *outer = add_node(NULL);

	timeline_node_transition *tmp = outer->add_transition();
	tmp->add_lhs(START_PREROLL_TIMELINE);
	tmp->add_rhs(START_PREROLL, rootnode);

	tmp = outer->add_transition();
	tmp->add_lhs(START_PLAY_TIMELINE);
	tmp->add_rhs(START_PLAY, rootnode);
	
	// XXXX Or should we add the DONE_PLAY_TIMELINE dummy event?
	tmp = outer->add_transition();
	tmp->add_lhs(DONE_PLAY, rootnode);
}

passive_timeline::~passive_timeline()
{
	if (m_playdone_indices)
		delete m_playdone_indices;
	m_playdone_indices = NULL;
}

timeline_node *
passive_timeline::add_node(const lib::node *the_node)
{
	timeline_node *rv = new timeline_node(the_node);
	m_timeline_nodes.push_back(rv);
	return rv;
}

timeline_delay *
passive_timeline::add_delay(int timeout)
{
	timeline_delay *rv = new timeline_delay(timeout);
	m_delays.push_back(rv);
	return rv;
}

void 
passive_timeline::build()
{
	detail::dependency_index_generator indexer;
	detail::timeline_node_vector::iterator node;

	// Step one - Give each (event_class, id) tuple use on a LHS
	// a unique sequence number.
	for(node=m_timeline_nodes.begin(); node < m_timeline_nodes.end(); node++) {
		(*node)->build_index(indexer);
	}
//	indexer.dump();
	// Step two - Flatten all events on the RHS into a long
	// list. The transitions will remember begin- and end positions in
	// this list, and fill m_playdone_indices with a mapping from node numbers
	// to PLAYDONE_EVENT index numbers
	m_playdone_indices = new int[m_timeline_nodes.size()];//(-1);
	for(size_t i=0;i<m_timeline_nodes.size();i++) m_playdone_indices[i] = -1;
	for(node=m_timeline_nodes.begin(); node < m_timeline_nodes.end(); node++) {
		int node_index = node - m_timeline_nodes.begin();
		(*node)->build_actions(m_actions, indexer, node_index);
		int playdone_index = (*node)->get_playdone_index(indexer, node_index);
		m_playdone_indices[node_index] = playdone_index;
	}
	// Step three - Build the dependencies vector by collecting all
	// the needed information from the transitions
	for(node=m_timeline_nodes.begin(); node < m_timeline_nodes.end(); node++) {
		(*node)->build_dependencies(m_dependencies);
	}
	m_is_built = true;
}

active_timeline *
passive_timeline::activate(lib::event_processor *const evp, common::playable_factory *rf, common::layout_manager *lm)
{
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "activate(), #dep=" << m_dependencies.size() << " #act=" << m_actions.size() << std::endl;
#endif
	return new active_timeline(evp, this, m_dependencies, m_actions, m_timeline_nodes.size(), rf, lm);
}

#ifndef AMBULANT_NO_IOSTREAMS
void 
passive_timeline::dump(std::ostream& os)
{
	detail::timeline_node_vector::iterator i;
	detail::delay_vector::iterator j;
	int num = 0;
	
	os << "-------Passive timeline:"  << std::endl;
	for(j=m_delays.begin(); j<m_delays.end(); j++)
		os << (**j) << std::endl;
	for(i=m_timeline_nodes.begin(); i<m_timeline_nodes.end(); i++)
		(**i).dump(os, num);
	os << "-------End of assive timeline:"  << std::endl;
	
}
#endif

/* ---------------- */
active_timeline::active_timeline(lib::event_processor *const evp,
	passive_timeline *const source, 
	const detail::active_dependency_vector& dependencies,
	const detail::active_action_vector& actions,
	int nregion,
	common::playable_factory *rf,
	common::layout_manager *lm)
:	m_event_processor(evp),
	m_playable_factory(rf),
	m_source(source),
	m_layout_manager(lm),
	m_dependencies(dependencies),
	m_actions(actions),
	m_playables(std::vector<common::playable *>(nregion, (common::playable *)NULL)),
	m_playdone(NULL)
{
	m_source->add_ref();
}

void
active_timeline::preroll()
{
	detail::active_dependency& dep = m_dependencies[START_PREROLL_TIMELINE_INDEX];
	int i;
	
	dep.m_depcount--;
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "preroll: count=" << dep.m_depcount << std::endl;
#endif
	if (dep.m_depcount == 0) {
		for (i=dep.m_first; i<dep.m_last; i++) {
			m_actions[i]->fire(this);
		}
	}
}

void
active_timeline::start(lib::event *playdone)
{
	detail::active_dependency& dep = m_dependencies[START_PLAY_TIMELINE_INDEX];
	int i;
	
	m_playdone = playdone;
	dep.m_depcount--;
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "start: count=" << dep.m_depcount << std::endl;
#endif
	if (dep.m_depcount == 0) {
		for (i=dep.m_first; i<dep.m_last; i++) {
			m_actions[i]->fire(this);
		}
	}
}

void
active_timeline::stop()
{
	std::vector<common::playable *>::iterator it;
	for(it = m_playables.begin();it != m_playables.end();it++) {
		if(*it != 0 && *it != &dead_playable) {
			(*it)->stop();
			(*it)->release();
			(*it) = &dead_playable;
		}
	}
}

void
active_timeline::pause()
{
	std::vector<common::playable *>::iterator it;
	for(it = m_playables.begin();it != m_playables.end();it++) {
		if(*it != 0 && *it != &dead_playable) {
			(*it)->pause();
		}
	}
}

void
active_timeline::resume()
{
	std::vector<common::playable *>::iterator it;
	for(it = m_playables.begin();it != m_playables.end();it++) {
		if(*it != 0 && *it != &dead_playable) {
			(*it)->resume();
		}
	}
}

void
active_timeline::dependency_callback(detail::dependency_callback_arg arg)
{
	detail::active_dependency& dep = m_dependencies[arg];
	int i;
	
	dep.m_depcount--;
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "active_timeline.dependency_callback(" << arg << ") -> " << dep.m_depcount << std::endl;
#endif
	if (dep.m_depcount == 0) {
		for (i=dep.m_first; i<dep.m_last; i++) {
			m_actions[i]->fire(this);
		}
	}
	// And special-case the final event:
	if (arg == DONE_PLAY_TIMELINE_INDEX) {
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "DONE_PLAY_TIMELINE()" << std::endl;
#endif
		if (m_playdone) {
#ifndef AMBULANT_NO_IOSTREAMS
			AM_DBG std::cout << "firing playdone event to parent" << std::endl;
#endif
			m_event_processor->add_event(m_playdone, 0, lib::event_processor::low);
		}
		// Other cleanup
	}
}

void
active_timeline::ext_dependency_callback(detail::dependency_callback_arg arg)
{
	m_actions[arg]->delayed_fire(this);
}

void
active_timeline::ext_preroll(int node_index)
{
	if (m_playables[node_index] == &dead_playable)
		return;
	if (m_playables[node_index] != NULL) {
		return;
	}
	timeline_node *tln = m_source->m_timeline_nodes[node_index];
	const lib::node *the_node = tln->m_node;
	common::playable *pl = m_playable_factory->new_playable(
		this, node_index, the_node, m_event_processor);
	// Connect the playable/renderer to its output surface, if applicable
	if (pl) {
		common::renderer *rend = pl->get_renderer();
		if (rend) {
			common::surface *dest = m_layout_manager->get_surface(the_node);
			rend->set_surface(dest);
		}
	}
	m_playables[node_index] = pl;

}

void
active_timeline::ext_play(int node_index)
{
	if (m_playables[node_index] == NULL) {
		ext_preroll(node_index);
	}
	if (m_playables[node_index] == &dead_playable) {
		// It was STOPPLAY-ed before this PLAY came in.
		// Fire the playdone event
		lib::logger::get_logger()->error("active_timeline::ext_play(%d) after ext_stop()!", node_index);
	} else {
		m_playables[node_index]->start(0);
#if JACK_DEBUG_MOUSEREGIONS
                // Temporary code to enable mous clicking in every second node played
		static int counter;
		if (counter++ & 1) m_playables[node_index]->wantclicks(true);
#endif
	}
}

void
active_timeline::ext_stop(int node_index)
{
	if (m_playables[node_index] == &dead_playable)
		return;
	if (m_playables[node_index]) {
		m_playables[node_index]->stop();
		m_playables[node_index]->release();
	}
	m_playables[node_index] = &dead_playable;
}

#ifndef AMBULANT_NO_IOSTREAMS

void 
active_timeline::dump(std::ostream& os)
{
	
	os << "dumping active timeline, #dep=" << m_dependencies.size() << " #act=" << m_actions.size() << std::endl;
	detail::active_dependency_vector::iterator i;
	int c;
	for(c=0,i=m_dependencies.begin(); i<m_dependencies.end(); c++, i++)
		os << "dep " << c << " " << *i << std::endl;
	detail::active_action_vector::const_iterator j;
	for(c=0, j=m_actions.begin(); j<m_actions.end(); c++, j++)
		os << "act " << c << " " << **j << std::endl;
}

#endif

void 
active_timeline::started(int n, double t)
{
	AM_DBG lib::logger::get_logger()->trace("active_timeline::started(%d)", n);
}

void 
active_timeline::stopped(int n, double t)
{
	// XXXX This should call dependency_callback(playdone_index)
	int playdone_index = m_source->m_playdone_indices[n];
	AM_DBG lib::logger::get_logger()->trace("active_timeline::stopped(%d) playdone %d", n, playdone_index);
	if (playdone_index < 0)
		lib::logger::get_logger()->fatal("active_timeline::stopped(node %d): playdone_index=%d", n, playdone_index);
	typedef lib::scalar_arg_callback<active_timeline, detail::dependency_callback_arg> mycallback;
	lib::event *e = new mycallback(this, &active_timeline::dependency_callback, playdone_index);
	m_event_processor->add_event(e, 0, lib::event_processor::high);
}

void 
active_timeline::clicked(int n, double t)
{
	AM_DBG lib::logger::get_logger()->trace("active_timeline::clicked(%d)", n);
}

} // namespace mms
} // namespace ambulant
