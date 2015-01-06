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

#ifndef AMBULANT_COMMON_LAYOUT_H
#define AMBULANT_COMMON_LAYOUT_H

#include <string>
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/common/preferences.h"

namespace ambulant {

namespace lib {
class document;
class transition_info;
} // namespace lib

namespace common {

class region_info;
class animation_destination;
class renderer; // forward
class surface; // forward
class gui_events; // forward
class factories;

typedef std::pair<lib::rect, lib::rect > tile_position; ///< Source and destination rect for a tile.
typedef std::vector<tile_position> tile_positions; ///< List of tile_position

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
	virtual ~animation_notification(){}

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
    /// Constructor, only for use by subclasses.
	gui_window(gui_events *handler)
	:   m_handler(handler) {};
  public:
	virtual ~gui_window() {}

	/// Signals that rectangle r may need to be redrawn.
	virtual void need_redraw(const lib::rect &r) = 0;

	/// Do any pending redraws right now
	virtual void redraw_now() = 0;

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
/// It is used to commmunicate redraw requests and mouse clicks
/// and such from the GUI window all the way down to
/// the renderer.
class AMBULANTAPI gui_events  {
  public:
	virtual ~gui_events(){}

	/// Request to redraw a certain area.
	virtual void redraw(const lib::rect &dirty, gui_window *window) = 0;

	/// Signals a mouse click or mouse move. Returns true if handled.
	virtual bool user_event(const lib::point &where, int what = 0) = 0;

	/// Signals that a transition in the given area has started.
	/// This event goes through the gui_events interface because SMIL
	/// semantics dictate that fill=transition ends on underlying
	/// areas when a new transition starts.
	virtual void transition_freeze_end(lib::rect area) = 0;
};

/// Interface for playables that actually render something.
/// Implemented by playable implementations that render to a region
/// (as opposed to SMIL animation playables, etc).
class AMBULANTAPI renderer : public gui_events {
  public:
	virtual ~renderer() {};

	/// Render to a specific surface.
	/// Called (by the scheduler) after the playable is created, to
	/// tell it where to render to.
	virtual void set_surface(surface *destination) = 0;

	/// Use alignment align for image display.
	virtual void set_alignment(const alignment *align) = 0;

	/// Apply an inTransition when starting playback.
	virtual void set_intransition(const lib::transition_info *info) = 0;

	/// Start an outTransition now.
	virtual void start_outtransition(const lib::transition_info *info) = 0;

	/// XXXX This is a hack.
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

	/// Keep current onscreen bits as background
	virtual void keep_as_background() = 0;

	/// Highlight a region
	virtual void highlight(gui_window *window) = 0;
};

// These two types enable a renderer to store private data on the surface
// it renders to, and this data can be retrieved by the next instance of this
// renderer. This can be used by renderers that use an external entity (such
// as an HTML widget) to do the actual rendering.
typedef lib::ref_counted renderer_private_data;
typedef void* renderer_private_id;

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
	virtual void need_redraw(const lib::rect &r) = 0;

	/// The whole region has changed and needs a redraw.
	virtual void need_redraw() = 0;

	/// Requests forwarding of mouse events.
	virtual void need_events(bool want) = 0;

	/// Signals that a transition has finished.
	virtual void transition_done() = 0;

	/// Signals that the current on-screen bits should be kept as background
	virtual void keep_as_background() = 0;

	/// Returns the region rectangle, (0, 0) based.
	virtual const lib::rect& get_rect() const = 0;

	/// Returns the region rectangle, topwindow-coordinate based and clipped.
	virtual const lib::rect& get_clipped_screen_rect() const = 0;

	/// Returns the gui_window coordinates for (0, 0).
	virtual const lib::point &get_global_topleft() const = 0;

	/// Determine where to draw an image.
	/// For a given image size, return portion of source image to display, and where
	/// to display it. The renderer must do the scaling.
	virtual lib::rect get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect, const alignment *align) const = 0;

	/// Determine where to draw an image.
	/// For a given image size, return portion of source image to display, and where
	/// to display it. The renderer must do the scaling.
	virtual lib::rect get_fit_rect(const lib::rect& src_crop_rect, const lib::size& src_size, lib::rect* out_src_rect, const alignment *align) const = 0;

    /// Determine source rectangle to draw.
    /// Applies panzoom attributes to the source image and returns resulting rect.
	virtual lib::rect get_crop_rect(const lib::size& src_size) const = 0;

	/// Get object holding SMIL region parameters for querying.
	virtual const region_info *get_info() const = 0;

	/// Get the outermost surface for this surface.
	virtual surface *get_top_surface() = 0;

	/// Return true if the image needs to be tiled
	virtual bool is_tiled() const = 0;

	/// Given image size and region rectangle return a list of (srcrect, dstrect).
	virtual tile_positions get_tiles(lib::size image_size, lib::rect surface_rect) const = 0;

	/// Get the OS window for this surface.
	virtual gui_window *get_gui_window() = 0;

	/// Save a per-renderer private data pointer on the surface.
	/// Renderer implementations that want to cache things between instantiations
	/// on the same surface can use this to do so. As an example, various
	/// HTML renderers use this to forestall having to destroy and immediately
	/// re-create HTML widgets (which is not only time-consuming but also
	/// causes a lot of flashing).
	virtual void set_renderer_private_data(renderer_private_id idd, renderer_private_data* data) = 0;

	/// Retrieve a per-renderer private data pointer previously stored with set_renderer_data.
	virtual renderer_private_data* get_renderer_private_data(renderer_private_id idd) = 0;

	/// Turn highlighting of the region on or off.
	virtual void highlight(bool on) = 0;

};

/// API for creating windows.
/// window_factory is subclassed by the various GUI implementations.
/// It should create a GUI window, and set up for that GUI window to forward
/// its redraw requests to the given region.
class AMBULANTAPI window_factory {
  public:
	virtual ~window_factory() {}

	/// Get the default size for a new window
	virtual lib::size get_default_size() { return lib::size(default_layout_width, default_layout_height); }
	/// Create a new window.
	virtual gui_window *new_window(const std::string &name, lib::size bounds, gui_events *handler) = 0;

	/// Create a new bgrenderer.
	virtual bgrenderer *new_background_renderer(const region_info *src) = 0;

	/// Close a window.
	virtual void window_done(const std::string &name) {}
};

/// Interface for storing SMIL layout information.
/// surface_template is an abstract baseclass used by the SMIL2 and MMS layout managers
/// to create subregions in the region hierarchy. During playback it is also the destination
/// for animation notifications.
class surface_template : public animation_notification {
  public:
	virtual ~surface_template() {}

	/// Create a new subregion.
	virtual surface_template *new_subsurface(const region_info *info, bgrenderer *bgrend) = 0;

	/// Get the surface corresponding to this region.
	virtual surface *activate() = 0;
};


/// Interface for storing SMIL layout information.
/// Used by the SMIL2 layout_manager implementation to create
/// surface_template objects for the toplevel windows.
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


/// Factory function for a SMIL2 layout_manager.
AMBULANTAPI layout_manager *create_smil2_layout_manager(common::factories *factory,lib::document *doc);

/// Factory function for a surface_factory implementation.
AMBULANTAPI surface_factory *create_smil_surface_factory();

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_LAYOUT_H
