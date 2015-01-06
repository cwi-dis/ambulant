/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_COMMON_RENDERER_IMPL_H
#define AMBULANT_COMMON_RENDERER_IMPL_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/renderer_select.h"
#ifdef __GNUC__
#include <cxxabi.h>
#endif
namespace ambulant {

namespace common {

class global_playable_factory;


inline std::string _get_playable_sig(const playable *p, const lib::node *n) {
	std::stringstream ss;
	std::string typenm = typeid(*p).name();
#ifdef __GNUC__
	int status;
	char *realname = abi::__cxa_demangle(typenm.c_str(), 0, 0, &status);
	if (status == 0) {
		typenm = realname;
		free(realname);
	}
#endif // __GNUC__
	ss << typenm << "(" << std::hex << (void*)p << ", " << n->get_sig() << ")";
	return ss.str();
};

/// A convenience class implementing some of the common code for playables.
/// Most of the methods in this class store parameters that are common
/// to most playables in protected instance variables.
/// Other methods do nothing, whenever that is
/// acceptable behavior. For example, pausing an image does not do
/// anything spectacular.
/// Use this class as a baseclass for your renderer/playable.
class AMBULANTAPI playable_imp : public playable {
  public:
	/// Constructor.
	playable_imp(
		playable_notification *context,
		cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
	:	m_context(context),
		m_cookie(cookie),
		m_node(node),
		m_event_processor(evp) {
	}

	// common::playable methods
	void pause(pause_display d=display_show) {}
	void resume() {}
//	void seek(double where) {}
	void wantclicks(bool want) { m_wantclicks = want;}
	void preroll(double when, double where, double how_much) {}
	void post_stop() {};
	void init_with_node(const lib::node *n) {};
	/// Return true if we are rendering for a node with fill="ambulant:continue"
	bool is_fill_continue_node() const {
		const char * fb = m_node->get_attribute("fill");
		return fb != NULL && strcmp(fb, "ambulant:continue") == 0;
	}
	duration get_dur() { return duration(true, 0);}
	cookie_type get_cookie() const { return m_cookie;}
	std::string get_sig() const { return _get_playable_sig(this, m_node); }

  protected:
	playable_notification *m_context;	///< Status feedback object.
	cookie_type m_cookie;				///< Parameter for status feedback object.
	const lib::node	*m_node;			///< The DOM node this playable corresponds to.
	lib::event_processor *m_event_processor;	///< The event_processor we can use.
	bool m_wantclicks;					///< True if we should send gui events to m_context.
};

/// A convenience class for playables that are also renderers.
/// In addition to the functionality provided by playable_imp this class
/// also provides default implementations of the renderer interface.
class AMBULANTAPI renderer_playable : public playable_imp, public renderer {
  public:
	/// Constructor.
	renderer_playable(
		playable_notification *context,
		cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	virtual ~renderer_playable();

	// common::renderer interface
	void set_surface(common::surface *dest) { m_dest = dest; };
	void set_alignment(const common::alignment *align) { m_alignment = align; }
	surface *get_surface() { return m_dest;}
	virtual bool user_event(const lib::point &where, int what = 0);
	/// Return True if this renderer should handle events at this point (given transparency issues and such).
	virtual bool user_event_sensitive(const lib::point &where);
	renderer *get_renderer() { return this; }
	void transition_freeze_end(lib::rect r) { m_context->transitioned(m_cookie); }
	virtual void start(double t);
	virtual bool stop();
	virtual void init_with_node(const lib::node *n);
  protected:
	virtual void _init_clip_begin_end();	///< Fill m_clip_begin and m_clip_end
	surface *m_dest;		///< The surface we should render to.
	const alignment *m_alignment;	///< The image alignment to use when rendering.
	bool m_activated;		///< True when playing
	bool m_erase_never;		///< True if erase="never" is specified on the node
	net::timestamp_t m_clip_begin;	///< Where continuous media start playing (microseconds)
	net::timestamp_t m_clip_end;	///< Where continuous media stop playing (microseconds)
};


/// A convenience class for playables that receive data from a datasource.
/// In addition to the functionality provided by renderer_playable this class
/// creates a raw datasource for the url specified in the node "src"
/// attribute. On start() the datasource is started too. A subclass must
/// define a readdone method which is called whenever data becomes available.
/// User event handling is also taken care of.
class AMBULANTAPI renderer_playable_ds : public renderer_playable {
  public:
	/// Constructor.
	renderer_playable_ds(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		common::factories* factory,
		common::playable_factory_machdep *mdp);

	virtual ~renderer_playable_ds();

	virtual void start(double where);
	virtual void seek(double t);
//	virtual void freeze() {}
//	virtual void stop();
	virtual bool stop();
//	virtual void pause() {}
//	virtual void resume() {}
//	virtual void wantclicks(bool want);

	virtual void redraw(const lib::rect &dirty, gui_window *window) = 0;
	/// Called whenever data is available.
	virtual void readdone() = 0;
  protected:
	net::datasource *m_src;	///< The datasource.
};

/// A convenience class for playables that cannot  start rendering until all data is available.
/// In addition to the functionality provided by renderer_playable_ds this class
/// creates a raw datasource for the url specified in the node "src"
/// attribute. On start() the datasource is started too, and renderer_playable_dsall
/// provides a readdone() method that simply collects all data. When all data
/// has been received it schedules a redraw.
///
/// Hence, when you subclass this class you only need to provide a redraw() method.
class AMBULANTAPI renderer_playable_dsall : public renderer_playable_ds {
  public:
	/// Constructor.
	renderer_playable_dsall(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	:	renderer_playable_ds(context, cookie, node, evp, factory, mdp),
		m_data(NULL),
		m_data_size(0),
		m_partial_data(NULL),
		m_partial_data_size(0) {};
	virtual ~renderer_playable_dsall();

	virtual void seek(double t) {}  // Assume dsall playables are images and such
  protected:
	void readdone();
	void *m_data;			///< The data to be rendered.
	size_t m_data_size;	///< The size of m_data.
  private:
	void *m_partial_data;	///< The data while it is being read.
	size_t m_partial_data_size;	///< Size of the above.
};

/// Implementation of playable_factory.
/// Playable implementations register themselves with this object
/// through add_factory(). Then, when the client code wants to allocate
/// a new playable all factories are tried in order until one is
/// able to create a playable.
class global_playable_factory_impl : public global_playable_factory {
  public:
	global_playable_factory_impl();
	~global_playable_factory_impl();

	/// Add a factory.
	void add_factory(playable_factory *rf);

	/// Signal preference for a certain renderer (or category of renderers)
	void preferred_renderer(const char* name);

	/// The global factory supports everything (it says:-)
	bool supports(renderer_select *rs)
	{
		return true;
	}

	/// Create a new playable.
	playable *new_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	playable* new_aux_audio_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);

  private:
	std::list<playable_factory *> m_factories;
	playable_factory *m_default_factory;
	std::map<int, renderer_select*> m_renderer_select;
    lib::critical_section m_lock;
    
};

/// Convience class: a playable_notification that does nothing.
class empty_playable_notification : public playable_notification {
  public:
	// Playables nodifications
	void started(cookie_type n, double t = 0)  {};
	void stopped(cookie_type n, double t = 0)  {};
	void clicked(cookie_type n, double t = 0)  {};
	void pointed(cookie_type n, double t = 0)  {}; // mouse over
	void transitioned(cookie_type n, double t = 0) {};
	void marker_seen(cookie_type n, const char *name, double t = 0) {};
	void playable_stalled(const playable *p, const char *reason)  {} ;
	void playable_unstalled(const playable *p) {};
	void playable_started(const playable *p, const lib::node *n, const char *comment) {};
	void playable_resource(const playable *p, const char *resource, long amount) {};
};

/// Convenience class for background renderers.
/// It implements some of the methods for a renderer that are applicable
/// to background renderers. Subclasses only need to implement
/// the redraw method.
/// Additionally, they should override the keep_as_background method.
class background_renderer : public bgrenderer {
  public:
	/// Constructor.
	background_renderer(const region_info *src)
	:   m_src(src),
		m_dst(NULL) {}
	virtual ~background_renderer() {}
	void set_surface(surface *destination) { m_dst = destination; }
	bool user_event(const lib::point &where, int what = 0) { return false;};
	void transition_freeze_end(lib::rect area) {};
  protected:
	const region_info *m_src;	///< Where we get our parameters (such as color) from.
	surface *m_dst;				///< Where we should render to.
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_RENDERER_IMPL_H
