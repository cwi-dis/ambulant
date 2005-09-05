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

#ifndef AMBULANT_GUI_COCOA_COCOA_GUI_H
#define AMBULANT_GUI_COCOA_COCOA_GUI_H

#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#ifdef USE_SMIL21
#include "ambulant/smil2/transition.h"
#endif
#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

class cocoa_window : public common::gui_window {
  public:
  	cocoa_window(const std::string &name, lib::size bounds, void *_view, common::gui_events *handler)
  	:	common::gui_window(handler),
  		m_view(_view) {};
	~cocoa_window();
  		
	void need_redraw(const lib::rect &r);
	void redraw_now();
	void need_events(bool want);
	
	void redraw(const lib::rect &r);
	void user_event(const lib::point &where, int what = 0);

	void *view() { return m_view; }
	
  private:
    void *m_view;
};

;
class cocoa_window_factory : public common::window_factory {
  public:
  	cocoa_window_factory(void *view)
  	:	m_defaultwindow_view(view) {}
  	
	common::gui_window *new_window(const std::string &name, lib::size bounds, common::gui_events *handler);
	common::bgrenderer *new_background_renderer(const common::region_info *src);
  private:
    void *m_defaultwindow_view;
};

class cocoa_renderer_factory : public common::playable_factory {
  public:
  	cocoa_renderer_factory(common::factories *factory)
	:   m_factory(factory) {}
  	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
		
	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
  private:
    common::factories *m_factory;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#ifdef __OBJC__
@interface NSRectHolder : NSObject
{
	NSRect rect;
}

- (id) initWithRect: (NSRect)r;
- (NSRect)rect;
@end

@interface AmbulantView : NSView
{
    ambulant::gui::cocoa::cocoa_window *ambulant_window;
	NSImage *transition_surface;
	NSImage *transition_tmpsurface;
	int transition_count;
#ifdef USE_SMIL21
	int fullscreen_count;
	NSImage *fullscreen_previmage;
	NSImage *fullscreen_oldimage;
	ambulant::smil2::transition_engine *fullscreen_engine;
	ambulant::lib::transition_info::time_type fullscreen_now;
#endif
}

- (id)initWithFrame:(NSRect)frameRect;
- (void)dealloc;

- (void)setAmbulantWindow: (ambulant::gui::cocoa::cocoa_window *)window;
- (void)ambulantWindowClosed;
- (bool)isAmbulantWindowInUse;
- (BOOL)isFlipped;

- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::rect *)arect;
- (ambulant::lib::rect) ambulantRectForNSRect: (const NSRect *)nsrect;

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect;
- (void) syncDisplayIfNeeded: (id) dummy;

- (void)mouseDown: (NSEvent *)theEvent;
- (void)mouseMoved: (NSEvent *)theEvent;

- (void)dumpToImageID: (char *)ident;
- (void)dump: (id)image toImageID: (char *)ident;
- (BOOL)wantsDefaultClipping;

- (void) incrementTransitionCount;
- (void) decrementTransitionCount;

// while in a transition, getTransitionSurface returns the surface that the
// transitioning element should be drawn to.
- (NSImage *)getTransitionSurface;

// internal: release the transition surface when we're done with it.
- (void)_releaseTransitionSurface;

// while in a transition, if we need an auxiliary surface (to draw a clipping
// path or something like that) getTransitionTmpSurface will return one.
- (NSImage *)getTransitionTmpSurface;

// while in a transition, getTransitionOldSource will return the old pixels,
// i.e. the pixels "behind" the transitioning element.
- (NSImage *)getTransitionOldSource;

// while in a transition, getTransitionNewSource will return the new pixels,
// i.e. the pixels the transitioning element drew into getTransitionSurface.
- (NSImage *)getTransitionNewSource;

// Return the current on-screen image
- (NSImage *)_getOnScreenImage;

// Return part of the onscreen image
- (NSImage *)getOnScreenImageForRect: (NSRect)bounds;

#ifdef USE_SMIL21
- (void) startScreenTransition;
- (void) endScreenTransition;
- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
		elapsed: (ambulant::lib::transition_info::time_type)now;
		
- (void) _screenTransitionPreRedraw;
- (void) _screenTransitionPostRedraw;
#endif
@end

#endif // __OBJC__
#endif // AMBULANT_GUI_COCOA_COCOA_GUI_H
