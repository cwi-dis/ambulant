
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_PLAYER_H
#define AMBULANT_LIB_PLAYER_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward
class active_player;

namespace detail {

class timeline_done_arg {
};

} // namespace detail

// I'm not clear on the split between active and passive player yet. The
// passive_player could do loading of the document and parsing it, and
// possibly even generating the timelines.
// Splitting the player in active/passive may be a case of over-generality,
// in which case we get rid of the passive_player later.
class passive_player {
  public:
	friend class active_player;
	
	passive_player() 
	:	m_url("") {}
	passive_player(char *url)
	:	m_url(url) {}
	~passive_player() {}
	
	active_player *activate();
  private:
  	char *m_url;
};

class active_player : public ref_counted {
  public:
	active_player(passive_player *const source, node *tree);
	~active_player();
	
	void start(event_processor *evp, event *playdone);
	void stop();
	
	void timeline_done_callback(detail::timeline_done_arg *p) {
		std::cout << "active_player.timeline_done_callback()" << std::endl;
		m_done = true;
	}
	
	////////////////////////
	// lib::ref_counted interface implementation
	
	long add_ref() {return ++m_refcount;}

	long release() {
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}

  private:
  	passive_timeline *build_timeline();
  	event_processor *const m_event_processor;
	passive_player *const m_source;
	node *m_tree;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;

	basic_atomic_count<unix::critical_section> m_refcount;
};

// IMPLEMENTATION
active_player *
passive_player::activate()
{
	lib::tree_builder builder;
	if(!builder.build_tree_from_file(m_url)) {
		std::cout << "Could not build tree for file: " << m_url << std::endl;
		return NULL;
	}
	// get (and become owner of) the root of the tree
	lib::node *node = builder.detach();
	return new active_player(this, node);
}

active_player::active_player(passive_player *const source, node *tree)
:	m_source(source),
	m_tree(tree),
	m_refcount(1),
	m_event_processor(new unix::event_processor())
{
}

active_player::~active_player()
{
}

void
active_player::start(event_processor *evp, event *playdone)
{
	m_done = false;
	passive_timeline *ptl = build_timeline();
	if (ptl) {
		active_timeline *atl = ptl->activate(m_event_processor);
		m_active_timelines.push_back(atl);
	
		typedef callback<active_player,detail::timeline_done_arg> callback;
		event *ev = new callback(this, 
			&active_player::timeline_done_callback, 
			new detail::timeline_done_arg());
	
		// run it
		atl->preroll();
		atl->start(ev);
		while (!m_done)
			sleep(1);
	}
	if (evp && playdone)
		evp->add_event(playdone, 0, event_processor::low);

}

void
active_player::stop()
{
}

passive_timeline *
active_player::build_timeline()
{
	return NULL;
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PLAYER_H
