/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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
#else// ! WITH_UIKIT
#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/AppKit.h>
#define VIEW_SUPERCLASS NSView
#endif// ! WITH_UIKIT
#endif//__OBJC__

#ifdef WITH_UIKIT
#define __WEBSERVICESCORE__ // HACK! HACK!
#include <CoreGraphics/CoreGraphics.h>
#else // ! WITH_UIKIT
#include <ApplicationServices/ApplicationServices.h>
#endif // ! WITH_UIKIT

namespace ambulant {

namespace gui {

namespace cg {

inline CGRect CGRectFromAmbulantRect(const lib::rect& arect) {
    return CGRectMake(arect.left(), arect.top(), arect.width(), arect.height());
}

inline ambulant::lib::rect ambulantRectFromCGRect(const CGRect& nsrect) {
	ambulant::lib::rect arect = ambulant::lib::rect(
		ambulant::lib::point(int(CGRectGetMinX(nsrect)), int(CGRectGetMinY(nsrect))),
		ambulant::lib::size(int(CGRectGetWidth(nsrect)), int(CGRectGetHeight(nsrect))));
	return arect;
}

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
#ifdef	WITH_AVFOUNDATION
common::playable_factory *create_cg_avfoundation_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
#endif//WITH_AVFOUNDATION
common::playable_factory *create_cg_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
common::playable_factory *create_cg_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
//common::playable_factory *create_cg_ink_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
common::playable_factory *create_cg_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
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
	ambulant::gui::cg::cg_window *ambulant_window;  // Our counterpart-object on the Ambulant side
    ambulant::lib::size original_bounds;            // The size specified in the SMIL document
	int transition_count;
	int fullscreen_count;
	BOOL fullscreen_outtrans;
	BOOL transition_pushed;
	BOOL fullscreen_ended;
	BOOL has_drawn; // Indicates whether something has been draw in the view
	CGLayerRef transition_surface;
#ifdef	WITH_UIKIT
	CGLayerRef fullscreen_oldimage;
//	CGLayerRef transition_tmpsurface;
	ambulant::smil2::transition_engine *fullscreen_engine;
	ambulant::lib::transition_info::time_type fullscreen_now;
#else // ! WITH_UIKIT
	NSImage *transition_tmpsurface;
	NSImage *fullscreen_previmage;
	NSImage *fullscreen_oldimage;
	ambulant::smil2::transition_engine *fullscreen_engine;
	ambulant::lib::transition_info::time_type fullscreen_now;
	NSGraphicsContext* old_context;
#endif// ! WITH_UIKIT
}

- (id)initWithFrame:(CGRect)frameRect;
- (id)initWithCoder:(NSCoder *)aDecoder;
- (void)dealloc;

- (void)setAmbulantWindow: (ambulant::gui::cg::cg_window *)window;
- (void)ambulantWindowClosed;
- (bool)isAmbulantWindowInUse;
- (bool)ignoreResize;

- (void)ambulantSetSize: (ambulant::lib::size) bounds;
- (void)ambulantNeedEvents: (bool) want;

- (CGContextRef) getCGContext;
- (CGAffineTransform) transformForRect: (const CGRect *)rect flipped: (BOOL)flipped translated: (BOOL)translated;

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect;
- (void) syncDisplayIfNeeded: (id) dummy;

#ifdef WITH_UIKIT
- (void) tappedWithPoint: (CGPoint)where;
- (BOOL) tappedAtPoint:(CGPoint) location;
#else // ! WITH_UIKIT
- (BOOL)isFlipped;
- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize;
- (void)mouseDown: (NSEvent *)theEvent;
- (void)mouseMoved: (NSEvent *)theEvent;
- (void)pseudoMouseMove: (id)dummy;
#endif //! WITH_UIKIT

- (BOOL)wantsDefaultClipping;

- (void) incrementTransitionCount;
- (void) decrementTransitionCount;


// pushes the context associated with transition_surface on the CGContext stack
// this has the effect that subsequents drawings will be done on transition_surface
- (void) pushTransitionSurface;

// pops the context associated with transition_surface from the CGContext stack
// this has the effect that subsequents drawings will be done on screen back buffer
- (void) popTransitionSurface;

- (void) startScreenTransition: (BOOL) isOuttrans;
- (void) endScreenTransition;
- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
		elapsed: (ambulant::lib::transition_info::time_type)now;

- (void) _screenTransitionPreRedraw;
- (void) _screenTransitionPostRedraw;

// while in a transition, getTransitionSurface returns the surface that the
// transitioning element should be drawn to.
- (CGLayerRef) getTransitionSurface;

// internal: release the transition surfaces when we're done with it.
- (void) releaseTransitionSurfaces;

// while in a transition, if we need an auxiliary surface (to draw a clipping
// path or something like that) getTransitionTmpSurface will return one.
- (CGLayerRef) getTransitionTmpSurface;

#ifndef WITH_UIKIT
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
- (void) screenTransitionStep: (ambulant::smil2::transition_engine*) engine
					  elapsed: (ambulant::lib::transition_info::time_type) now;

- (void) _screenTransitionPreRedraw;
- (void) _screenTransitionPostRedraw;

#endif // ! WITH_UIKIT
#ifdef WITH_UIKIT

// Graphics debugging routines

// Get an UIImage of the iPhone/iPad screen
+ (UIImage*) UIImageFromScreen; 

// Get the entire content of an UIView*  (without subviews) as an UIImage*
+ (UIImage*) UIImageFromUIView: (UIView*) view;

// Get an UIImage* from the contents of a CGLayerRef
+ (UIImage*) UIImageFromCGLayer: (CGLayerRef) layer;

// Create a new CGLayer containing a CGImage
+ (CGLayerRef) CGLayerCreateFromCGImage: (CGImageRef) image;

// write a CGImageRef to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpCGImage: (CGImageRef) img withId: (NSString*) id;

// write the contents of an UIView to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpUIView: (UIView*) view withId: (NSString*) id;

// write the contents of an CGLayer to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpCGLayer: (CGLayerRef) cglr withId: (NSString*) id;

// write the contents of an iPhone/iPad screen to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpScreenWithId: (NSString*) id;


#if 0
// while in a transition, getTransitionOldSource will return the old pixels,
// i.e. the pixels "behind" the transitioning element.
- (CGLayerRef) getTransitionOldSource; // Not used

// while in a transition, getTransitionNewSource will return the new pixels,
// i.e. the pixels the transitioning element drew into getTransitionSurface.
- (CGLayerRef) getTransitionNewSource; //TBD

// Return the current on-screen image, caters for AVFoundation movies 
- (CGLayerRef) _getOnScreenImage; //TBD

// Return part of the onscreen image, does not cater for AVFoundation
- (CGImageRef) getOnScreenImageForRect: (CGRect)bounds; //TBD
#endif // 0
#endif// WITH_UIKIT
@end

#endif // __OBJC__
#endif // AMBULANT_GUI_CG_CG_GUI_H
