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

#ifndef AMBULANT_COMMON_LAYOUT_H
#define AMBULANT_COMMON_LAYOUT_H

#include <string>
#include "ambulant/lib/gtypes.h"
//#include "ambulant/lib/node.h"

namespace ambulant {

namespace lib {
class node;
class document;
class transition_info;
} // namespace lib

namespace common {

class region_info;
class animation_destination;
class renderer; // forward
class surface; // forward
class gui_events; // forward

/// Determine how to align an image in a region.
class alignment {
  public:
	virtual ~alignment() {};
	
	/// Return image point that should be painted at surface point get_surface_fixpoint().
	virtual lib::point get_image_fixpoint(lib::size image_size) const = 0;
	
	/// Return surface point at which image point get_image_fixpoint() should be painted.
	virtual lib::point get_surface_fixpoint(lib::size surface_size) const = 0;
};

/// Receive notification when something has been animated.
/// Animation_notification is where an animator will send a notification when it
/// is has changed parameters on an animation destination.
class animation_notification {
  public:
  
  	/// Called by the animator after some animation parameter has changed.
	virtual void animated() = 0;
};

/// The virtual baseclass for GUI-dependent window classes.
/// Together with gui_events it forms the interface between the AmbulantPlayer core
/// and the embedding application. gui_window must be implemented by the embedder and
/// is called from the core, gui_events is implemented by the core and is where the
/// embedder can send its redraw requests and such.
class gui_window {
  protected:
	gui_window(gui_events *handler)
	:   m_handler(handler) {};
  public:
	virtual ~gui_window() {}
	
	/// Signals that rectangle r may need to be redrawn.
	virtual void need_redraw(const lib::screen_rect<int> &r) = 0;
	
	/// Signals whether the core is interesting in mouse events and others.
	virtual void need_events(bool want) = 0;
  protected:
	gui_events *m_handler;	///< The other side of the interface
};

/// User event types that may be used with gui_events::user_event()
enum user_event_type {
	user_event_click,		///< User clicked the mouse
	user_event_mouse_over	///< User moved the mouse
};

/// API for receiving GUI events.
/// The pure virtual baseclass for both toplevel ambulant windows (as
/// seen from the GUI code) and renderers.
/// It is used to commmunicate redraw requests and mouse ckicks
/// and such from the GUI window all the way down to
/// the renderer.
class gui_events  {
  public:
  
  	/// Request to redraw a certain area.
	virtual void redraw(const lib::screen_rect<int> &dirty, gui_window *window) = 0;
	
	/// Signals a ouse click or move.
	virtual void user_event(const lib::point &where, int what = 0) = 0;
	
	/// Signals that a transition in the given area has started.
	virtual void transition_freeze_end(lib::screen_rect<int> area) = 0;
};

/// 
/// renderer is an pure virtual baseclass for renderers that
/// render to a region (as opposed to audio renderers, etc).
class renderer : public gui_events {
  public:
	virtual ~renderer() {};
	
	/// Render to a specific surface.
	virtual void set_surface(surface *destination) = 0;
	
	/// Use alignment align for image display.
	virtual void set_alignment(alignment *align) = 0;
	
	/// Apply an inTransition when starting playback.
	virtual void set_intransition(const lib::transition_info *info) = 0;
	
	/// Start an outTransition now.
	virtual void start_outtransition(const lib::transition_info *info) = 0;
	
	// XXXX This is a hack.
	virtual surface *get_surface() = 0;

};

/// bgrenderer is a pure virtual baseclass for background renderers.
/// It is really a subset of the renderer API with only the methods applicable
/// to backgrounds.
class bgrenderer : public gui_events {
  public:
	virtual ~bgrenderer() {};
	
	/// Render to a specific surface.
	virtual void set_surface(surface *destination) = 0;
};

/// Pure virtual baseclass for a region of screenspace.
/// It is the only interface that renderers use when talking to regions, and regions
/// use when talking to their parent regions.
class surface {
  public:
	virtual ~surface() {};
	
	/// The given renderer wants redraws and events from now on.
	virtual void show(gui_events *renderer) = 0;
	
	/// The given renderer no longer wants redraws and events.
	virtual void renderer_done(gui_events *renderer) = 0;

	/// The given rect r has changed and needs a redraw.
	virtual void need_redraw(const lib::screen_rect<int> &r) = 0;
	
	/// The whole region has changed and needs a redraw.
	virtual void need_redraw() = 0;
	
	/// Requests forwarding of mouse events.
	virtual void need_events(bool want) = 0;
	
	/// Signals that a transition has finished.
	virtual void transition_done() = 0;

	/// Returns the region rectangle, (0, 0) based.
	virtual const lib::screen_rect<int>& get_rect() const = 0;
	
	/// Returns the gui_window coordinates for (0, 0).
	virtual const lib::point &get_global_topleft() const = 0;
	
	/// Determine where to draw an image.
	/// For a given image size, return portion of source image to display, and where
	/// to display it. The renderer must do the scaling.
	virtual lib::screen_rect<int> get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect, alignment *align) const = 0;
	
	/// Get object holding SMIL region parameters for querying.
	virtual const region_info *get_info() const = 0;
	
#ifdef USE_SMIL21
	/// Get the outermost surface for this surface.
	virtual surface *get_top_surface() = 0;
#endif
	
	/// Get the OS window for this surface.
	virtual gui_window *get_gui_window() = 0;
};

/// API for creating windows.
/// window_factory is subclassed by the various GUI implementations.
/// It should create a GUI window, and set up for that GUI window to forward
/// its redraw requests to the given region.
class window_factory {
  public:
	virtual ~window_factory() {}
	
	/// Create a new window.
	virtual gui_window *new_window(const std::string &name, lib::size bounds, gui_events *handler) = 0;
	
	/// Create a new bgrenderer.
	virtual bgrenderer *new_background_renderer(const region_info *src) = 0;
	
	/// Close a window.
	virtual void window_done(const std::string &name) {} 
};

/// surface_template is an abstract baseclass used by the SMIL2 and MMS layout managers
/// to create subregions in the region hierarchy. During playback it is also the destination
/// for animation nofitications.
class surface_template : public animation_notification {
  public:
	virtual ~surface_template() {}
	
	/// Create a new subregion.
	virtual surface_template *new_subsurface(const region_info *info, bgrenderer *bgrend) = 0;
	
	/// Get the surface corresponding to this region.
	virtual surface *activate() = 0;
};


/// surface_template is an abstract baseclass used by the SMIL2 and MMS layout managers
/// to create toplevel regions for the region hierarchy.
class surface_factory {
  public:
	virtual ~surface_factory() {}
	
	/// Create a new toplevel region.
	virtual surface_template *new_topsurface(const region_info *info, bgrenderer *bgrend, window_factory *wf) = 0;
};

/// Determine where to render media items.
/// The layout_manager manages the mapping of media items to regions.
class layout_manager {
  public:
	virtual ~layout_manager() {};
	
	/// Return the surface on which the given node should be rendered.
	virtual surface *get_surface(const lib::node *node) = 0;
	
	/// Returns the imagge aligment parameters for the given node.
	virtual alignment *get_alignment(const lib::node *node) = 0;
	
	/// Return the object that will receive notifications when the given node is animated.
	virtual animation_notification *get_animation_notification(const lib::node *node) = 0;
	
	/// Return the object that the animator will control for the given node.
	virtual animation_destination *get_animation_destination(const lib::node *node) = 0;
};


// XXX These should be elsewhere
layout_manager *create_smil2_layout_manager(window_factory *wf,lib::document *doc);
//layout_manager *create_mms_layout_manager();
surface_factory *create_smil_surface_factory();
	
} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_LAYOUT_H
