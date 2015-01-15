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

#ifndef AMBULANT_GUI_CG_CG_GUI_H
#define AMBULANT_GUI_CG_CG_GUI_H

#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/smil2/transition.h"

// select proper frameworks for Iphone/MacOS target operating system
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#ifdef __OBJC__
#include <UIKit/UIKit.h>
#define VIEW_SUPERCLASS UIView
#endif//__OBJC__
#elif TARGET_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#ifdef __OBJC__
#include <AppKit/AppKit.h>
#define VIEW_SUPERCLASS NSView
#endif//__OBJC__
#else
#error "No valid TARGET_OS defined"
#endif//TARGET_OS

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
		m_view(_view)
		{}
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

#ifdef WITH_UIKIT
typedef CGRect NSRect;
#endif

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
	CGLayerRef fullscreen_oldimage;
	CGLayerRef transition_tmpsurface;
	ambulant::smil2::transition_engine *fullscreen_engine;
	ambulant::lib::transition_info::time_type fullscreen_now;
#ifndef	WITH_UIKIT
	NSGraphicsContext* old_context;
	// section for use by npambulant only.
	// npambulant creates a "fake" AmbulantView (not added to a superView) outside of the "normal" drawing loop.
	// It calls the drawRect() method directly when the browser wants a NPCocoaEventDrawRect to be handled with 
	// the CGContext given in the event data. When the player calls need_redraw(lib::rect), the npapi function
	// NPN_InvalidateRect(NSRect*) is called though a callback mechanism, causing the browser to re-generate the draw event.
	// NPAPI as in Firefox/Google Chrome (not Safari) require the function to be called from main plugin thread 
	// (see: https://developer.mozilla.org/en/Gecko_Plugin_API_Reference/Browser_Side_Plug-in_API)
	CGContextRef myCGContext;//X
	CGRect myBounds;		 //X
	CGRect myFrame;			 //X
	CGSize mySize;			 //X
	void* plugin_callback;	 // callback function pointer to call NPN_InvalidateRect on need_redraw()
	void* plugin_data;		 // browser data for use by NPN_InvalidateRect
	ambulant::common::gui_player* plugin_mainloop; // needed for cursor shape determination
#endif// ! WITH_UIKIT
}

- (id)initWithFrame:(NSRect)frameRect;
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
- (NSImage *)_getOnScreenImage;

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

// clear the transition surface before drawing on it
- (void) clearTransitionSurface;

// internal: release the transition surfaces when we're done with it.
- (void) releaseTransitionSurfaces;

// while in a transition, getTransitionOldSource will return the old pixels,
// i.e. the pixels "behind" the transitioning element.
- (CGLayerRef)getTransitionOldSource;

// return the current Graphics Context (AppKit/UIKit)
+ (CGContextRef) currentCGContext;

// Create a new CGLayer containing a CGImage
+ (CGLayerRef) CGLayerCreateFromCGImage: (CGImageRef) image flipped: (BOOL) flip;

#ifdef WITH_UIKIT

// Graphics debugging routines

// Get an UIImage of the iPhone/iPad screen
+ (UIImage*) UIImageFromScreen; 

// Get the entire content of an UIView*  (without subviews) as an UIImage*
+ (UIImage*) UIImageFromUIView: (UIView*) view;

// Get an UIImage* from the contents of a CGLayerRef
+ (UIImage*) UIImageFromCGLayer: (CGLayerRef) layer;

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
#else // ! WITH_UIKIT
- (ambulant::gui::cg::cg_window *) getAmbulant_window;
@property (nonatomic,assign) void* plugin_callback; 
@property (nonatomic,assign) void* plugin_data;
@property (nonatomic,assign) ambulant::common::gui_player* plugin_mainloop;
#endif// ! WITH_UIKIT
@end

#endif // __OBJC__

#ifndef WITH_UIKIT
// Helper functions for npambulant
void* new_AmbulantView(CGContextRef cg_ctxp, CGRect rectp, void* plugin_callback, void* plugin_ptr);
CGSize get_bounds_AmbulantView(void* view);
// call AmbulantView.drawRect directly with the given CGContext 
void* draw_rect_AmbulantView(void* obj, CGContextRef cg_ctxp, CGRect* rectp);
// mouse event handler for npambulant
typedef struct _event_data {double x; double y;} event_data; //XXX
void handle_event_AmbulantView(void* obj, CGContextRef ctx, void* type, void* data,  void* mainloop);
// destructor for npambulant
void delete_AmbulantView(void* obj);
const char* to_char_AmbulantView(void* obj, void* nstr);
#endif// ! WITH_UIKIT

#endif // AMBULANT_GUI_CG_CG_GUI_H
