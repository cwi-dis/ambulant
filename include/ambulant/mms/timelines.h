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

#ifndef AMBULANT_MMS_TIMELINES_H
#define AMBULANT_MMS_TIMELINES_H

#include "ambulant/config/config.h"

#include <vector>
#include <map>
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/net/datasource.h"

namespace ambulant {

namespace mms {

// Forward delcarations

class active_timeline;
class timeline_node;
class timeline_delay;
namespace detail {
class dependency_index_generator;
class active_ext_action;
class active_int_action;
class active_preroll_action;
class active_startplay_action;
class active_stopplay_action;
typedef std::vector<timeline_node*> timeline_node_vector;
typedef std::vector<timeline_delay*> delay_vector;
};

// timeline_event_class defines the types of events that can
// happen in the timeline scheduler.

enum timeline_event_class {
	// TIMELINE events are things happening to the timeline as a whole
	START_PREROLL_TIMELINE,
	START_PLAY_TIMELINE,
	DONE_PLAY_TIMELINE,
	
	// internal events: these are side-effect-free events that keep the
	// timeline scheduler running
	START_PREROLL,
	START_PLAY,
	STOP_PLAY,
	DONE_PLAY,

	DELAY,
	
	// external events: these are side-effect-only events that cause external
	// things to happen, or the externally-caused event returned from these
	START_PREROLL_RENDERER,
	START_PLAY_RENDERER,
	STOP_PLAY_RENDERER,
	DONE_PLAY_RENDERER,
	
};

#ifndef AMBULANT_NO_IOSTREAMS
inline std::ostream& operator<<(std::ostream& os, const timeline_event_class n) {
	static char *timeline_event_class_names[] = {
		"START_PREROLL_TIMELINE",
		"START_PLAY_TIMELINE",
		"DONE_PLAY_TIMELINE",
		
		"START_PREROLL",
		"START_PLAY",
		"STOP_PLAY",
		"DONE_PLAY",
		
		"DELAY",
		
		"START_PREROLL_RENDERER",
		"START_PLAY_RENDERER",
		"STOP_PLAY_RENDERER",
		"DONE_PLAY_RENDERER",
	};
	if ((unsigned)n < sizeof(timeline_event_class_names)/sizeof(timeline_event_class_names[0]))
		os << timeline_event_class_names[(int)n];
	else
		os << "UNKNOWN_EVENT[" << (int)n << "]";
	return os;
}
#endif

// timeline_delay objects are used by the timeline scheduler to
// implement a delay. The object stores the value of the delay (in
// milliseconds), but the object identity is also important:
// The combination (DELAY, delayobject) is what triggers the
// dependent actions

class timeline_delay {
  public:
  	timeline_delay(int time)
	: m_time(time) {};
	
	inline int timeout() const { return m_time; }

#ifndef AMBULANT_NO_IOSTREAMS
	friend inline std::ostream& operator<<(std::ostream& os, const timeline_delay& n) {
		os << "delay("  << static_cast<const void *>(&n) << ") = " << n.m_time << "ms" << std::endl;
		return os;
	}
#endif

  private:
  	int m_time;
};

namespace detail {

// active_action objects are used to store the RHS of a timeline_event
// for execution by an active_timeline object. The active_action objects
// themselves are immutable and owned by the passive_timeline.

class active_action {
  public:
  	// active_timeline calls this to execute the action
  	virtual void fire(active_timeline * const parent) const = 0;
  	virtual void delayed_fire(active_timeline * const parent) const = 0;
#ifndef AMBULANT_NO_IOSTREAMS
  	virtual void to_stream(std::ostream& os) const = 0;
#endif
};

typedef std::vector<detail::active_action*> active_action_vector;

#ifndef AMBULANT_NO_IOSTREAMS
inline std::ostream& operator<<(std::ostream& os, const detail::active_action& n) {
	n.to_stream(os);
	return os;
}
#endif

// event_uid is a type that is used in timeline events. It is a unique
// identifier of the object to which the timeline event refers.

typedef const void *event_uid;

// a timeline_event stores a unique event in the schedule of the
// timeline. It is similar to an arc in a petrinet.

class timeline_event {
  public:
    timeline_event(timeline_event_class what, detail::event_uid direct_object)
    :	m_what(what), m_direct_object(direct_object) {};
    timeline_event(timeline_event_class what, const lib::node *direct_object)
    :	m_what(what), m_direct_object(static_cast<detail::event_uid>(direct_object)) {};
    timeline_event(timeline_event_class what, const timeline_delay *direct_object)
    :	m_what(what), m_direct_object(static_cast<detail::event_uid>(direct_object)) {};
    
  	inline bool operator<(const timeline_event& right) const {
  		if (m_what < right.m_what) 
  			return true;
  		if (m_what > right.m_what)
  			return false;
  		if (m_direct_object < right.m_direct_object)
  			return true;
  		return false;
  	}

#ifndef AMBULANT_NO_IOSTREAMS
	friend inline std::ostream& operator<<(std::ostream& os, const detail::timeline_event& n) {
		os << n.m_what << "(" << static_cast<const void *>(n.m_direct_object) << ")";
		return os;
	}
#endif
	
  protected:
	timeline_event_class m_what;
	detail::event_uid m_direct_object;
};

// timeline_rhs_event stores one entry in the RHS of a node transition
// list. In addition to storing the information in a timeline_event
// it can generate the active_action objects for itself

class timeline_rhs_event : public timeline_event {
  public:
    timeline_rhs_event(timeline_event_class what, const lib::node *direct_object)
    :	timeline_event(what, direct_object),
    	m_node(direct_object),
    	m_delay(NULL) {};
    timeline_rhs_event(timeline_event_class what, const timeline_delay *direct_object)
    :	timeline_event(what, direct_object),
    	m_node(NULL),
    	m_delay(direct_object) {};

	void build_action(detail::active_action_vector& actions,
		detail::dependency_index_generator& indexer,
		int node_index);

  private:
  	const lib::node *m_node;
  	const timeline_delay *m_delay;
};

typedef std::vector<detail::timeline_event> timeline_event_vector;
typedef std::vector<detail::timeline_rhs_event> timeline_rhs_event_vector;

// The dependency index generator is an object used while flattening
// the information used during building a passive_timeline to the form
// executable by an active_timeline. It converts timeline_event
// objects to a unique sequence number.

class dependency_index_generator {
  public:
  	dependency_index_generator()
  	:	m_cur_end_pos(0) {}
  	
  	// Get the next available sequence number, and map all
  	// events in the given event vector to this sequence number.
  	int set_index(timeline_event_vector &ev);
  	
  	// Get the sequence number for a given event.
  	int get_index(timeline_event &ev);
  private:
  	std::map<timeline_event, int> m_index;
  	int m_cur_end_pos;
};

// Class to store the argument to a timeline dependency callback
typedef int dependency_callback_arg;

// active_dependency is the representation of the LHS of a single
// timeline node transition in an active_timeline object. A vector
// of these (indexed by the dependency index) is stored by the active_timeline.
// When a dependency callback comes in the dependency count is decremented
// and when it reaches zero the corresponding actions are fired.
// This is a mutable object, and each active_timeline object has a private
// copy.
class active_dependency {
  public:
  	active_dependency(int count, int first, int last)
  	:	m_depcount(count),
  		m_first(first),
  		m_last(last) {}

#ifndef AMBULANT_NO_IOSTREAMS
	friend inline std::ostream& operator<<(std::ostream& os, const detail::active_dependency& n) {
		os << "active_dependency(count=" << n.m_depcount << 
			", first=" << n.m_first <<
			", last=" << n.m_last << ")";
		return os;
	}
#endif
  		
	int	m_depcount;
	int m_first;
	int m_last;
};

typedef std::vector<detail::active_dependency> active_dependency_vector;

} //namespace detail

// timeline_node_transition helps building a state transition for
// node playback. Create it, then add preconditions and events fired

class timeline_node_transition {
  public:
  	// XXXX Note: I think node and region need to be refcounted.
	timeline_node_transition() {};
	
	// Methods for building the transitions
	void add_lhs(timeline_event_class what);
	void add_lhs(timeline_event_class what, const lib::node *direct_object);
	void add_lhs(timeline_event_class what, const timeline_delay *direct_object);
	
	void add_rhs(timeline_event_class what);
	void add_rhs(timeline_event_class what, const lib::node *direct_object);
	void add_rhs(timeline_event_class what, const timeline_delay *direct_object);

	void build_index(detail::dependency_index_generator& indexer);
	void build_actions(detail::active_action_vector& actions,
			detail::dependency_index_generator& indexer,
			int node_index);
	void build_dependencies(detail::active_dependency_vector& dependencies);

#ifndef AMBULANT_NO_IOSTREAMS	
	void dump(std::ostream& os);
#endif

  private:
	detail::timeline_event_vector m_lhs;
  	detail::timeline_rhs_event_vector m_rhs;
  	int m_action_begin, m_action_end;
  	int m_event_index;
};

#ifndef AMBULANT_NO_IOSTREAMS	
inline std::ostream& operator<<(std::ostream& os, const timeline_node_transition& n) {
	os << "timeline_node_transition(" << (const void *)&n << ")";
	return os;
}
#endif

// timeline_node stores all information about a node that the passive_timeline
// is interested in: the node point, the region it plays to and the list
// of timeline transitions for it.

class timeline_node {
  public:
  	friend class active_timeline;

  	// XXXX Note: I think node, datasource and region need to be refcounted.
	timeline_node(const lib::node *the_node)
	:	m_node(the_node) {};

	timeline_node_transition *add_transition();
	
	void build_index(detail::dependency_index_generator& indexer);
	void build_actions(detail::active_action_vector& actions,
			detail::dependency_index_generator& indexer,
			int node_index);
	void build_dependencies(detail::active_dependency_vector& dependencies);
	int get_playdone_index(detail::dependency_index_generator& indexer, int node_index);
	
#ifndef AMBULANT_NO_IOSTREAMS	
	void dump(std::ostream& os, int &num);
	void dump(std::ostream& os);
#endif
  private:
  	const lib::node *m_node;
  	std::vector<timeline_node_transition*> m_transitions;
};

#ifndef AMBULANT_NO_IOSTREAMS	
inline std::ostream& operator<<(std::ostream& os, const timeline_node& n) {
	os << "timeline_node(" << (const void *)&n << ")";
	return os;
}
#endif

// Passive_timeline is the object used to build and store a schedule
// for a timeline. Create the passive_timeline, add nodes, add delays,
// and add timeline transitions to the nodes. Then call build() to
// convert all datastructures from the building form to the executable
// form. At this point the object becomes immutable, and you can activate()
// it to create a new runnable active_timeline.
 
class passive_timeline : public lib::ref_counted_obj {
  public:
  	friend class active_timeline;
  	
  	// Methods for initialization and teardown
	passive_timeline(lib::node *rootnode);
	~passive_timeline();

	// Methods used while building the passive timeline
	timeline_node *add_node(const lib::node *the_node);
	timeline_delay *add_delay(int timeout);
	
	void build();
	inline bool is_built() { return m_is_built; }
	
	active_timeline *activate(lib::event_processor *const evp, common::playable_factory *rf, common::layout_manager *lm);

#ifndef AMBULANT_NO_IOSTREAMS	
	void dump(std::ostream& os);
#endif
	
  private:
  	lib::node *m_rootnode;
    bool m_is_built;
    detail::timeline_node_vector m_timeline_nodes;
    detail::delay_vector m_delays;

	detail::active_dependency_vector m_dependencies;
	detail::active_action_vector m_actions;
	int *m_playdone_indices;
};

#ifndef AMBULANT_NO_IOSTREAMS	
inline std::ostream& operator<<(std::ostream& os, const passive_timeline& n) {
	os << "passive_timeline(" << (const void *)&n << ")";
	return os;
}
#endif

// Active_timeline is a runnable timeline scheduler. It shares
// all of its immutable data with the corresponding passive_timeline,
// so creating an active_timeline should be relatively cheap.

class active_timeline : public common::playable_notification, public lib::ref_counted_obj {
  public:
	friend class detail::active_ext_action;
	friend class detail::active_int_action;
	friend class detail::active_preroll_action;
	friend class detail::active_startplay_action;
	friend class detail::active_stopplay_action;
  	
	active_timeline(lib::event_processor *const evp,
		passive_timeline *const source, 
		const detail::active_dependency_vector& dependencies,
		const detail::active_action_vector& actions,
		int nregion,
		common::playable_factory *rf,
		common::layout_manager *lm);
	
	~active_timeline() {
		m_source->release();
	}
	
	void preroll();
	void start(lib::event *playdone);
	void stop();
	void pause();
	void resume();
	
#ifndef AMBULANT_NO_IOSTREAMS	
	void dump(std::ostream& os);
#endif

	// playable_notification interface:
	void started(int n, double t);
	void stopped(int n, double t);
	void clicked(int n, double t);
	void pointed(int n, double t) {}
  protected:
  	// These are protected because they are only meant for use
  	// by the various active_action objects
  	void dependency_callback(detail::dependency_callback_arg arg);
  	void ext_dependency_callback(detail::dependency_callback_arg arg);
  	void ext_preroll(int node_index);
  	void ext_play(int node_index);
  	void ext_stop(int node_index);
  	
  	lib::event_processor * const m_event_processor;
  	common::playable_factory *m_playable_factory;
    passive_timeline * const m_source;
	common::layout_manager *m_layout_manager;
	detail::active_dependency_vector m_dependencies;
	const detail::active_action_vector& m_actions;
	std::vector<common::playable *> m_playables;
	lib::event *m_playdone;
};

#ifndef AMBULANT_NO_IOSTREAMS
inline std::ostream& operator<<(std::ostream& os, const active_timeline& n) {
	os << "active_timeline(" << (const void *)&n << ")";
	return os;
}
#endif

} // namespace mms
 
} // namespace ambulant

#endif // AMBULANT_MMS_TIMELINES_H
