/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_CG_CG_GUI_H
#define AMBULANT_GUI_CG_CG_GUI_H

#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/smil2/transition.h"
#ifdef __OBJC__
#ifdef WITH_UIKIT
#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>
#include <UIKit/UIKit.h>
#define VIEW_SUPERCLASS UIView
inline CGRect CGRectFromViewRect(CGRect rect) { return rect; }
inline CGRect ViewRectFromCGRect(CGRect rect) { return rect; }
inline CGPoint CGPointFromViewPoint(CGPoint point) { return point; }
inline CGSize CGSizeFromViewSize(CGSize size) { return size; }
#else
#include <AppKit/AppKit.h>
#define VIEW_SUPERCLASS NSView
inline CGRect CGRectFromViewRect(NSRect rect) { return *(CGRect*)&rect; }
inline NSRect ViewRectFromCGRect(CGRect rect) { return *(NSRect*)&rect; }
inline CGPoint CGPointFromViewPoint(NSPoint point) { return *(CGPoint*)&point; }
inline CGSize CGSizeFromViewSize(NSSize size) { return *(CGSize*)&size; }
#endif
#endif // __OBJC__

// The following define enables code that allows drawing things
// on top of quicktime movies, using a separate overlay window.
// Not defining this means that anything drawn on top of a quicktime
// movie is simply not seen.
#undef WITH_QUICKTIME_OVERLAY

namespace ambulant {

namespace gui {

namespace cg {

class cg_window : public common::gui_window {
  public:
	cg_window(const std::string &name, lib::size bounds, void *_view, common::gui_events *handler)
	:	common::gui_window(handler),
	m_view(_view) {};
	~cg_window();

	void need_redraw(const lib::rect &r);
	void redraw_now();
	void need_events(bool want);

	void redraw(const lib::rect &r);
	bool user_event(const lib::point &where, int what = 0);

	void *view() { return m_view; }

	void set_size(lib::size bounds);

  private:
	void *m_view;
};

class cg_window_factory : public common::window_factory {
  public:
	cg_window_factory(void *view)
	:	m_defaultwindow_view(view)
	{}

	lib::size get_default_size();
	common::gui_window *new_window(const std::string &name, lib::size bounds, common::gui_events *handler);
	common::bgrenderer *new_background_renderer(const common::region_info *src);
  private:
	void *m_defaultwindow_view;
};

class cg_gui_screen : public common::gui_screen {
  public:
	cg_gui_screen(void *view)
	:	m_view(view)
	{}
	void get_size(int *width, int *height);
	bool get_screenshot(const char *type, char **out_data, size_t *out_size);
  private:
	void *m_view;
};

AMBULANTAPI common::window_factory *create_cg_window_factory(void *view);
common::playable_factory *create_cg_dsvideo_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
common::playable_factory *create_cg_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
common::playable_factory *create_cg_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_ink_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
common::playable_factory *create_cg_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

} // namespace cg

} // namespace gui

} // namespace ambulant

#ifdef __OBJC__
@interface NSRectHolder : NSObject
{
	CGRect rect;
}

- (id) initWithRect: (CGRect)r;
- (CGRect)rect;
@end

@interface AmbulantView : VIEW_SUPERCLASS
{
	ambulant::gui::cg::cg_window *ambulant_window;
//	NSImage *transition_surface;
//	NSImage *transition_tmpsurface;
	int transition_count;
	int fullscreen_count;
//	NSImage *fullscreen_previmage;
//	NSImage *fullscreen_oldimage;
//	ambulant::smil2::transition_engine *fullscreen_engine;
	ambulant::lib::transition_info::time_type fullscreen_now;
#ifdef WITH_QUICKTIME_OVERLAY
	NSWindow *overlay_window;
	BOOL overlay_window_needs_unlock;
	BOOL overlay_window_needs_reparent;
	BOOL overlay_window_needs_flush;
	BOOL overlay_window_needs_clear;
//	int overlay_window_count;
#endif // WITH_QUICKTIME_OVERLAY
#ifdef	WITH_UIKIT
	CGPoint tapped_location;
	CGRect original_frame;
	ambulant::lib::size original_bounds;
#endif//WITH_UIKIT
}

#ifdef	WITH_UIKIT
@property(nonatomic) CGPoint tapped_location;
@property(nonatomic) CGRect	original_frame;
@property(nonatomic) ambulant::lib::size original_bounds;
- (void) adaptDisplayAfterRotation;
#endif//WITH_UIKIT

- (id)initWithFrame:(CGRect)frameRect;
- (void)dealloc;

- (void)setAmbulantWindow: (ambulant::gui::cg::cg_window *)window;
- (void)ambulantWindowClosed;
- (bool)isAmbulantWindowInUse;
- (bool)ignoreResize;
#ifndef WITH_UIKIT
- (BOOL)isFlipped;
#endif

- (void)ambulantSetSize: (ambulant::lib::size) bounds;
- (void)ambulantNeedEvents: (bool) want;

- (CGContextRef) getCGContext;
- (CGRect) CGRectForAmbulantRect: (const ambulant::lib::rect *)arect;
- (ambulant::lib::rect) ambulantRectForCGRect: (const CGRect *)nsrect;

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect;
- (void) syncDisplayIfNeeded: (id) dummy;

#ifdef WITH_UIKIT
- (void) tappedWithPoint: (CGPoint)where;
#else
- (void)mouseDown: (NSEvent *)theEvent;
- (void)mouseMoved: (NSEvent *)theEvent;
- (void)pseudoMouseMove: (id)dummy;
#endif

- (BOOL)wantsDefaultClipping;

- (void) incrementTransitionCount;
- (void) decrementTransitionCount;

#if NOT_YET_UIKIT
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

// Return the current on-screen image, caters for quicktime movies
- (NSImage *)_getOnScreenImage;

// Return part of the onscreen image, does not cater for quicktime
- (NSImage *)getOnScreenImageForRect: (CGRect)bounds;

- (void) startScreenTransition;
- (void) endScreenTransition;
- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
		elapsed: (ambulant::lib::transition_info::time_type)now;

- (void) _screenTransitionPreRedraw;
- (void) _screenTransitionPostRedraw;

// Called by a renderer if it requires an overlay window.
// The overlay window is refcounted.
- (void) requireOverlayWindow;

// Helper
- (void) _createOverlayWindow: (id)dummy;

// Called by a renderer redraw() if subsequent redraws in the current redraw sequence
// should go to the overlay window
- (void) useOverlayWindow;

// Called by a renderer if the overlay window is no longer required.
- (void) releaseOverlayWindow;

// Called when the view hierarchy has changed
- (void) viewDidMoveToSuperview;
#endif // NOT_YET_UIKIT
@end

#endif // __OBJC__
#endif // AMBULANT_GUI_CG_CG_GUI_H
