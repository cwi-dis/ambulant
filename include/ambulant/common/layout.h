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
class surface_source; // forward

//class mouse_region_factory; // forward

class gui_region {
  public:
	virtual ~gui_region() {};
	virtual gui_region *clone() const = 0;
	
	virtual void clear() = 0;
	virtual bool is_empty() const = 0;
	virtual bool contains(const lib::point &p) const = 0;
	virtual gui_region& operator =(const lib::screen_rect<int> &rect) = 0;    /* assignment */
	virtual gui_region& operator &=(const gui_region &r) = 0; /* intersection */
	virtual gui_region& operator &=(const lib::screen_rect<int> &rect) = 0;
	virtual gui_region& operator |=(const gui_region &r) = 0; /* union */
	virtual gui_region& operator -=(const gui_region &r) = 0; /* difference */
	virtual gui_region& operator +=(const lib::point &tr) = 0; /* translation */
        
	virtual gui_region* operator &(const gui_region &r) const {
            gui_region *rv = this->clone();
            *rv &= r;
            return rv;
        }
	virtual gui_region* operator |(const gui_region &r) const {
            gui_region *rv = this->clone();
            *rv |= r;
            return rv;
        }
	virtual gui_region* operator -(const gui_region &r) const {
            gui_region *rv = this->clone();
            *rv -= r;
            return rv;
        }
};

// abstract_window is a virtual baseclass for GUI-dependent window classes.
// It is the only interface that the ambulant core code uses to talk to the
// GUI layer.
class abstract_window {
  protected:
	abstract_window(surface_source *region)
	:   m_region(region) {};
  public:
	virtual ~abstract_window() {}
	virtual void need_redraw(const lib::screen_rect<int> &r) = 0;
	virtual void mouse_region_changed() = 0;
  protected:
	surface_source *m_region;
};

// User event types that may be used with renderer::user_event()
enum user_event_type {user_event_click, user_event_mouse_over};

// renderer is an pure virtual baseclass for renderers that
// render to a region (as opposed to audio renderers, etc).
class renderer {
  public:
	virtual ~renderer() {};
	
	virtual void set_surface(surface *destination) = 0;
	virtual void set_intransition(lib::transition_info *info) = 0;
	virtual void start_outtransition(lib::transition_info *info) = 0;
	virtual void redraw(const lib::screen_rect<int> &dirty, abstract_window *window) = 0;
	virtual void user_event(const lib::point &where, int what = 0) = 0;
	// XXXX This is a hack.
	virtual surface *get_surface() = 0;

};

// The pure virtual baseclassfor subregions
// themselves. It is used to commmunicate redraw requests and mouse ckicks
// and such from the GUI window all the way down to
// the renderer.
// XXX This class should go!
class surface_source : public renderer {
  public:
	virtual const gui_region& get_mouse_region() const = 0;
	void set_intransition(lib::transition_info *info) { /* Ignore, for now */ }
	void start_outtransition(lib::transition_info *info) { /* Ignore, for now */ }
};

// class alignment is a pure virtual baseclass used for aligning an
// image in a region
class alignment {
  public:
	virtual ~alignment() {};
	
	virtual lib::point get_image_fixpoint(lib::size image_size) const = 0;
	virtual lib::point get_surface_fixpoint(lib::size surface_size) const = 0;
};

// animation_notification is where an animator will send a notification when it
// is has changed parameters on an animation destination.
class animation_notification {
  public:
	virtual void animated() = 0;
};

// surface is a pure virtual baseclass for a region of screenspace.
// It is the only interface that renderers use when talking to regions, and regions
// use when talking to their parent regions.
class surface {
  public:
	virtual ~surface() {};
	
	virtual void show(renderer *renderer) = 0;
	virtual void renderer_done() = 0;

	virtual void need_redraw(const lib::screen_rect<int> &r) = 0;
	virtual void need_redraw() = 0;
	virtual void need_events(bool want) = 0;

	virtual const lib::screen_rect<int>& get_rect() const = 0;
	virtual const lib::point &get_global_topleft() const = 0;
	
	// For a given image size, return portion of source image to display, and where
	// to display it. The renderer must do the scaling.
	virtual void set_alignment(const alignment *align) = 0;
	virtual const alignment *get_alignment() const = 0;
	virtual lib::screen_rect<int> get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect) const = 0;
	
	// Get object holding SMIL region parameters for querying
	virtual const region_info *get_info() const = 0;
	
	// Get the OS window for this surface
	virtual abstract_window *get_abstract_window() = 0;
};

// window_factory is subclassed by the various GUI implementations.
// It should create a GUI window, and set up for that GUI window to forward
// its redraw requests to the given region.
class window_factory {
  public:
	virtual ~window_factory() {}
	virtual abstract_window *new_window(const std::string &name, lib::size bounds, surface_source *region) = 0;
	virtual gui_region *new_mouse_region() = 0;
	virtual renderer *new_background_renderer(const region_info *src) = 0;
	virtual void window_done(const std::string &name) {} 
};

// surface_factory is an abstract baseclass used by the SMIL2 and MMS layout managers
// to create hierarchical regions
class surface_template : public animation_notification {
  public:
	virtual ~surface_template() {}
	virtual surface_template *new_subsurface(const region_info *info, renderer *bgrend) = 0;
	virtual surface *activate() = 0;
};

class surface_factory {
  public:
	virtual ~surface_factory() {}
	virtual surface_template *new_topsurface(const region_info *info, renderer *bgrend, window_factory *wf) = 0;
};

class layout_manager {
  public:
	virtual ~layout_manager() {};
	virtual surface *get_surface(const lib::node *node) = 0;
	virtual animation_notification *get_animation_notification(const lib::node *node) = 0;
	virtual animation_destination *get_animation_destination(const lib::node *node) = 0;
};

// XXX These should be elsewhere
layout_manager *create_smil2_layout_manager(window_factory *wf,lib::document *doc);
//layout_manager *create_mms_layout_manager();
surface_factory *create_smil_surface_factory();
	
} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_LAYOUT_H
