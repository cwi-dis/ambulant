// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_text.h"
#include "ambulant/gui/cg/cg_image.h"
#include "ambulant/gui/cg/cg_fill.h"
#include "ambulant/gui/cg/cg_dsvideo.h"
#include "ambulant/lib/mtsync.h"
//#include "ambulant/gui/cg/cg_html.h"
//#include "ambulant/gui/cg/cg_ink.h"
//#include "ambulant/gui/cg/cg_video.h"
//#include "ambulant/gui/cg/cg_smiltext.h"

#ifdef WITH_UIKIT
// These functions do something for AppKit, but for UIKit we the corresponding types
// are actually identical. The code becomes more beautiful if we call the functions always.
#define NSSizeToCGSize(x) (x)
#define NSRectToCGRect(x) (x)
#define NSSizeFromCGSize(x) (x)
#define NSRectFromCGRect(x) (x)
#endif

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif//AM_DBG

#if 0
AM_DBG
void dumpResponderChain(NSResponder *r) {
    std::string x = "";
    while (r) {
        NSLog(@"%snext responder %@", x.c_str(), r);
        x += " ";
        r = [r nextResponder];
    }
}
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

common::window_factory *
create_cg_window_factory(void *view)
{
	return new cg_window_factory(view);
}

cg_window::~cg_window()
{
	AM_DBG logger::get_logger()->debug("cg_window::~cg_window(0x%x)", (void *)this);
	if (m_view) {
		AmbulantView *my_view = (AmbulantView *)m_view;
		[my_view ambulantWindowClosed];
	}
	m_view = NULL;
}

void
cg_window::need_redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cg_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (!m_view) {
		logger::get_logger()->fatal("cg_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
	CGRect my_rect = CGRectFromAmbulantRect(r);
	NSRectHolder *arect = [[NSRectHolder alloc] initWithRect: my_rect];
	// XXX Is it safe to cast C++ objects to ObjC id's?
//X	if (m_plugin_callback == NULL) {
		[my_view performSelectorOnMainThread: @selector(asyncRedrawForAmbulantRect:)
								  withObject: arect waitUntilDone: NO];
//X	} else {
//X		m_plugin_callback(m_plugin_data, (void*) &r);
//X	}
}

void
cg_window::redraw_now()
{
	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view performSelectorOnMainThread: @selector(syncDisplayIfNeeded:)
		withObject: nil waitUntilDone: NO];
}

void
cg_window::redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cg_window::redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_handler->redraw(r, this);
}

bool
cg_window::user_event(const point &where, int what)
{
	AM_DBG logger::get_logger()->debug("cg_window::user_event(0x%x, (%d, %d), %d)", (void *)this, where.x, where.y, what);
	return m_handler->user_event(where, what);
}

void
cg_window::need_events(bool want)
{
	// This code needs to be run in the main thread.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	AM_DBG logger::get_logger()->debug("cg_window::need_events(0x%x, %d)", (void *)this, want);

	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view ambulantNeedEvents: want];
	[pool release];

}

void
cg_window::set_size(lib::size bounds)
{
	AM_DBG logger::get_logger()->debug("cg_window::set_size(0x%x, %d, %d)", (void *)this, bounds.w, bounds.h);
	AmbulantView *view = (AmbulantView *)m_view;
	[view ambulantSetSize: bounds];
}

lib::size
cg_window_factory::get_default_size()
{
	if (m_defaultwindow_view == NULL)
		return lib::size(common::default_layout_width, common::default_layout_height);
	CGSize size = NSSizeToCGSize([(AmbulantView *)m_defaultwindow_view bounds].size);
	return lib::size((int)size.width, (int)size.height);
}

common::gui_window *
cg_window_factory::new_window(const std::string &name, size bounds, common::gui_events *handler)
{
	AM_DBG NSLog(@"view %@ responds %d", (AmbulantView *)m_defaultwindow_view, [(AmbulantView *)m_defaultwindow_view respondsToSelector: @selector(isAmbulantWindowInUse)]);
	if ([(AmbulantView *)m_defaultwindow_view isAmbulantWindowInUse]) {
		// XXXX Should create new toplevel window and put an ambulantview in it
		logger::get_logger()->error(gettext("Unsupported: AmbulantPlayer cannot open second toplevel window yet"));
		return NULL;
	}
	cg_window *window = new cg_window(name, bounds, m_defaultwindow_view, handler);
	// And we need to inform the object about it
	AmbulantView *view = (AmbulantView *)window->view();
	[view setAmbulantWindow: window];
	// And set the window size
	window->set_size(bounds);

	return (common::gui_window *)window;
}

common::bgrenderer *
cg_window_factory::new_background_renderer(const common::region_info *src)
{
	return new cg_background_renderer(src);
}

void
cg_gui_screen::get_size(int *width, int *height)
{
	AmbulantView *view = (AmbulantView *)m_view;
	CGRect bounds = NSRectToCGRect([view bounds]);
	*width = int(bounds.size.width);
	*height = int(bounds.size.height);
}

bool
cg_gui_screen::get_screenshot(const char *type, char **out_data, size_t *out_size)
{
#ifndef WITH_UIKIT
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	*out_data = NULL;
	*out_size = 0;
	NSBitmapImageFileType filetype;
	if ( strcmp(type, "tiff") == 0) filetype = NSTIFFFileType;
	else if (strcmp(type, "bmp") == 0) filetype = NSBMPFileType;
	else if (strcmp(type, "gif") == 0) filetype = NSGIFFileType;
	else if (strcmp(type, "jpeg") == 0) filetype = NSJPEGFileType;
	else if (strcmp(type, "png") == 0) filetype = NSPNGFileType;
	else {
		lib::logger::get_logger()->trace("get_screenshot: unknown filetype \"%s\"", type);
		[pool release];
		return false;
	}
	NSData *data;
	AmbulantView *view = (AmbulantView *)m_view;
	NSImage *image = [view _getOnScreenImage];
	
	NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:[image TIFFRepresentation]];
//	NSImageRep *rep = [image bestRepresentationForDevice: NULL]; // deprecated
	if (image == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get screen shot");
		goto bad;
	}
	if (rep == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get representation for screen shot");
		goto bad;
	}
	data = [rep representationUsingType: filetype properties: NULL];
	if (data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot convert screenshot to %s format", type);
		goto bad;
	}
	*out_data = (char *)malloc([data length]);
	if (*out_data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: out of memory");
		goto bad;
	}
	*out_size = [data length];
	[data getBytes: *out_data];
	[pool release];
	return true;
bad:
	[pool release];
#endif// ! WITH_UIKIT
	return false;
}

} // namespace cg

} // namespace gui

} //namespace ambulant

#ifdef	WITH_UIKIT
#import <QuartzCore/CALayer.h>
#endif //WITH_UIKIT

// Helper class: NSRect as an object
@implementation NSRectHolder

- (id) initWithRect: (CGRect)r
{
	rect = r;
	return self;
}

- (CGRect)rect
{
	return rect;
}
@end

@implementation AmbulantView

- (id)initWithFrame:(NSRect)frameRect
{
	AM_DBG NSLog(@"AmbulantView.initWithFrame(0x%@)", self);
	self = [super initWithFrame: frameRect];
	ambulant_window = NULL;
	transition_surface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL; // XXXJACK Why don't we initialize these?
	fullscreen_oldimage = NULL;
	fullscreen_engine = NULL;
	fullscreen_outtrans = NO;
	transition_pushed = NO;
	fullscreen_ended = NO;
	has_drawn = NO;
#ifndef	WITH_UIKIT
	myCGContext = NULL;
	myBounds = CGRectMake(0,0,0,0);
	myFrame = CGRectMake(0,0,0,0);
	mySize = CGSizeMake(0,0);
	plugin_callback = NULL;
	plugin_data = NULL;
	plugin_mainloop = NULL;
#endif// ! WITH_UIKIT

	return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
	AM_DBG NSLog(@"AmbulantView.initWithCoder(0x%@)", self);
	self = [super initWithCoder:aDecoder];
	ambulant_window = NULL;
	transition_surface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
	fullscreen_oldimage = NULL;
	fullscreen_engine = NULL;
	fullscreen_outtrans = NO;
	transition_pushed = NO;
	fullscreen_ended = NO;
	has_drawn = NO;
#ifndef	WITH_UIKIT
	myCGContext = NULL;
	myBounds = CGRectMake(0,0,0,0);
	myFrame = CGRectMake(0,0,0,0);
	mySize = CGSizeMake(0,0);
	plugin_callback = NULL;
	plugin_data = NULL;
	plugin_mainloop = NULL;
#endif// ! WITH_UIKIT
	
	return self;
}

- (void)dealloc {
	AM_DBG NSLog(@"AmbulantView.dealloc(0x%@)", self);
	if (transition_surface) {
		AM_DBG NSLog(@"CFGetRetainCount(transition_surface)=%ld",CFGetRetainCount(transition_surface));
		CFRelease(transition_surface);
		transition_surface = NULL;
	}
	if (fullscreen_oldimage) {
		CFRelease(fullscreen_oldimage);
		fullscreen_oldimage = NULL;
	}
	[super dealloc];

}

- (CGContextRef) getCGContext
{
#ifdef WITH_UIKIT
	return UIGraphicsGetCurrentContext();
#else// ! WITH_UIKIT
	return (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif// ! WITH_UIKIT
}

- (ambulant::lib::rect) ambulantRectForCGRect: (const CGRect *)nsrect
{
	ambulant::lib::rect arect = ambulant::lib::rect(
		ambulant::lib::point(int(CGRectGetMinX(*nsrect)), int(CGRectGetMinY(*nsrect))),
		ambulant::lib::size(int(CGRectGetWidth(*nsrect)), int(CGRectGetHeight(*nsrect))));
	return arect;
}

- (CGAffineTransform) transformForRect: (const CGRect *)rect flipped: (BOOL)flipped translated: (BOOL)translated
{
	CGFloat fy = flipped?-1:1;
	CGFloat tx = 0;
	CGFloat ty = 0;
	if (translated) {
		tx = CGRectGetMinX(*rect);
		if (flipped) {
			ty = CGRectGetMinY(*rect)+CGRectGetHeight(*rect);
		} else {
			ty = CGRectGetMinY(*rect);
		}
	} else if (flipped) {
		ty = 2*CGRectGetMinY(*rect)+CGRectGetHeight(*rect);
	}
	CGAffineTransform matrix = CGAffineTransformMake(1, 0, 0, fy, tx, ty);
	return matrix;
}

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect
{
	CGRect my_rect = [arect rect];
	[arect release];
	AM_DBG NSLog(@"AmbulantView.asyncRedrawForAmbulantRect: self=0x%@ ltrb=(%f,%f,%f,%f)", self, CGRectGetMinX(my_rect), CGRectGetMinY(my_rect), CGRectGetMaxX(my_rect), CGRectGetMaxY(my_rect));
#ifdef WITH_UIKIT
    [self setNeedsDisplayInRect: NSRectFromCGRect(my_rect)];
#else
	if (plugin_callback == NULL) { //AmbulantPlayer
		[self setNeedsDisplayInRect: NSRectFromCGRect(my_rect)];
	} else if (ambulant_window != NULL) {
		// npambulant: Firefox, Google Chrome need NPN_InvalidateRect to be called from main thread
		void npambulant_invalidateRect(void*, CGRect);
		npambulant_invalidateRect((void*) self, my_rect);
	}
#endif
}

- (void) syncDisplayIfNeeded: (id) dummy
{
#ifdef WITH_UIKIT
	[self setNeedsDisplay];
#else // AppKit
	[self setNeedsDisplay:true];
#endif//WITH_UIKIT
}

- (void)setAmbulantWindow: (ambulant::gui::cg::cg_window *)window
{
	AM_DBG NSLog(@"setAmbulantWindow:%p", window);
	ambulant_window = window;
}

- (void)ambulantWindowClosed
{
	AM_DBG NSLog(@"ambulantWindowClosed called");
	ambulant_window = NULL;
	has_drawn = NO;
	// XXXX Should we close the window too? Based on preference?
}

- (bool)isAmbulantWindowInUse
{
	return (ambulant_window != NULL);
}

- (bool)ignoreResize
{
	return false;
}

- (void)ambulantSetSize: (ambulant::lib::size) bounds
{
    // Remember frame and bounds and adapt the window reqested in the current view
	AM_DBG NSLog(@"setSize before: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
    original_bounds = bounds;
    CGRect newBounds = CGRectMake(0, 0, bounds.w, bounds.h);
    CGRect newFrame = NSRectToCGRect(self.frame);
    newFrame.size = newBounds.size;
    self.frame = NSRectFromCGRect(newFrame);
    self.bounds = NSRectFromCGRect(newBounds);
    AM_DBG NSLog(@"setSize after set bounds: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
    // Now we need to adapt the toplevel UI to our new document size. This could mean changing the
    // zoom factor and positioning (iPhone/iPad) or changing the toplevel window size (Mac)
	if ([[self superview] respondsToSelector:@selector(recomputeZoom)]) {
        // We get a warning here that is difficult to forestall...
		[[self superview] recomputeZoom];
    }

    AM_DBG NSLog(@"setSize after aDAR: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
}

- (void)ambulantNeedEvents: (bool)want
{
#ifndef WITH_UIKIT
    // In UIKit, we always pass through the events.
	NSWindow *my_window = [self window];
	AM_DBG NSLog(@"my_window acceptsMouseMovedEvents = %d", [my_window acceptsMouseMovedEvents]);
	// See whether the mouse is actually in our area
	NSPoint where = [my_window mouseLocationOutsideOfEventStream];
	if (!NSPointInRect(where, [self frame])) {
		AM_DBG NSLog(@"mouse outside our frame");
		return;
	}
	// Get the main thread to do the real work
	[self performSelectorOnMainThread: @selector(pseudoMouseMove:)
		withObject: nil waitUntilDone: NO];
#endif
}

#ifdef WITH_UIKIT
// A couple of methods that are UIKit specific

// XXXJACK thinks this method can go.
- (void)tappedWithPoint: (CGPoint) where
{
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	AM_DBG NSLog(@"%p: tappedWithPoint at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}

// Equivalent of mouse move/click on iPhone
- (BOOL) tappedAtPoint:(CGPoint) location {
// NSLog(@"tappedAtPoint: x=%f y=%f", location.x, location.y);
	[self setNeedsDisplay];
	ambulant::lib::point amwhere = ambulant::lib::point((int) location.x, (int) location.y);
	if (ambulant_window) {
		return ambulant_window->user_event(amwhere, 0);
	}
	return false;
}
#else // ! WITH_UIKIT
// A couple of methods that are AppKit-specific
- (BOOL)isFlipped
{
	return true;
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize
{
    AM_DBG NSLog(@"resizeWithOldSuperviewSize: %@", self);
    AM_DBG NSLog(@"frame: %f, %f, %f, %f", self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);
    AM_DBG NSLog(@"bounds: old %f,%f new %f, %f, %f, %f", oldBoundsSize.width, oldBoundsSize.height, self.bounds.origin.x, self.bounds.origin.y, self.bounds.size.width, self.bounds.size.height);
    [super resizeWithOldSuperviewSize: oldBoundsSize];
    NSSize realSize = {original_bounds.w, original_bounds.h};
    [self setBoundsSize: realSize];
}

- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	if (plugin_callback == NULL) {
		// player needs conversion, npambulant plugin does not
		where = [self convertPoint: where fromView: nil];
	}
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"0x%@: mouseDown outside our frame", (void*)self);
		return;
	}
	AM_DBG NSLog(@"0x%@: mouseDown at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}

- (void)mouseMoved: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	if (plugin_callback == NULL) {
		// player needs conversion, npambulant plugin does not
		where = [self convertPoint: where fromView: nil];
	}
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseMoved outside our frame");
		if (plugin_callback != NULL && plugin_mainloop != NULL) {
			if ([NSCursor currentCursor] != [NSCursor pointingHandCursor])
				[[NSCursor pointingHandCursor] set];
		}
		return;
	}
	AM_DBG NSLog(@"%p: mouseMoved at ambulant-point(%f, %f) plugin_callback=%p plugin_mainloop=%p", (void*)self, where.x, where.y, plugin_callback, plugin_mainloop);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (plugin_callback == NULL) {
		[[NSApplication sharedApplication] sendAction: @selector(resetMouse:) to: nil from: self];
	} else {
		plugin_mainloop->before_mousemove(0);
	}
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	if (plugin_callback == NULL) {
		[[NSApplication sharedApplication] sendAction: @selector(fixMouse:) to: nil from: self];
	} else {// from AmbulantPlayer->MyDocument.mm
		int cursor = plugin_mainloop->after_mousemove();
		AM_DBG NSLog(@"mouseMoved: cursor=%d", cursor);
		if (cursor == 0) {
			if ([NSCursor currentCursor] != [NSCursor arrowCursor]) {
				[[NSCursor arrowCursor] set];
			}
		} else if (cursor == 1) {
			if ([NSCursor currentCursor] != [NSCursor pointingHandCursor])
				[[NSCursor pointingHandCursor] set];
		} else {
			NSLog(@"Warning: unknown cursor index %d", cursor);
		}
	}
}

- (void)pseudoMouseMove: (id)dummy
{
	NSPoint where = [[self window] mouseLocationOutsideOfEventStream];
	where = [self convertPoint: where fromView: nil];
	AM_DBG NSLog(@"pseudoMouseMoved at (%f, %f)", where.x, where.y);
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseMoved outside our frame");
		return;
	}
	AM_DBG NSLog(@"0x%@: pseudoMouseMove at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	[[NSApplication sharedApplication] sendAction: @selector(resetMouse:) to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: @selector(fixMouse:) to: nil from: self];
}

#endif// ! WITH_UIKIT

- (BOOL)wantsDefaultClipping
{
	return (transition_count == 0);
}

- (void) incrementTransitionCount
{
	transition_count++;
	AM_DBG NSLog(@"incrementTransitionCount: count=%d", transition_count);
}

- (void) decrementTransitionCount
{
	assert(transition_count > 0);
	transition_count--;
	AM_DBG NSLog(@"decrementTransitionCount: count=%d", transition_count);
}

#ifndef	WITH_UIKIT
// Transition implementation methods for AppKit
	
- (NSImage *)_getOnScreenImage
{
	NSView *src_view = self;
	NSWindow *tmp_window = NULL;
	CGRect bounds = NSRectToCGRect([self bounds]);
	CGSize size = CGSizeMake(bounds.size.width, bounds.size.height);
	NSImage *rv = [[NSImage alloc] initWithSize: NSSizeFromCGSize(size)];
	[src_view lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[src_view unlockFocus];
	if (tmp_window) {
		[tmp_window orderOut: self];
		[tmp_window close];
	}
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getOnScreenImageForRect: (CGRect)bounds
{
	// Note: this method does not take overlaying things such as Quicktime
	// movies into account.
	CGSize size = CGSizeMake(bounds.size.width, bounds.size.height);
	NSImage *rv = [[NSImage alloc] initWithSize: NSSizeFromCGSize(size)];
	[self lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: NSRectFromCGRect(bounds)];
	[self unlockFocus];
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getTransitionNewSource
{
	CGRect bounds = NSRectToCGRect([self bounds]);
	CGSize size = CGSizeMake(bounds.size.width, bounds.size.height);
	NSImage *rv = [[NSImage alloc] initWithSize: NSSizeFromCGSize(size)];
	[rv setFlipped: YES];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[rv addRepresentation: [bits autorelease]];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "newsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

#else// WITH_UIKIT
// Transition implementation methods for UIKit

// Get an UIImage of the iPhone/iPad screen
// From: http://developer.apple.com/library/ios/#qa/qa2010/qa1703.html
//		  Technical Q&A QA1703.	Screen Capture in UIKit Applications
+ (UIImage*) UIImageFromScreen 
{
	// Create a graphics context with the target size
	// On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
	// On iOS prior to 4, fall back to use UIGraphicsBeginImageContext
	CGSize imageSize = [[UIScreen mainScreen] bounds].size;
	if (NULL != UIGraphicsBeginImageContextWithOptions)
		UIGraphicsBeginImageContextWithOptions(imageSize, NO, 0);
	else
		UIGraphicsBeginImageContext(imageSize);
	
	CGContextRef context = [AmbulantView currentCGContext];	
	// Iterate over every window from back to front
	for (UIWindow *window in [[UIApplication sharedApplication] windows]) 
	{
		if (![window respondsToSelector:@selector(screen)] || [window screen] == [UIScreen mainScreen])
		{
			// -renderInContext: renders in the coordinate space of the layer,
			// so we must first apply the layer's geometry to the graphics context
			CGContextSaveGState(context);
			// Center the context around the window's anchor point
			CGContextTranslateCTM(context, [window center].x, [window center].y);
			// Apply the window's transform about the anchor point
			CGContextConcatCTM(context, [window transform]);
			// Offset by the portion of the bounds left of and above the anchor point
			CGContextTranslateCTM(context,
								  -[window bounds].size.width * [[window layer] anchorPoint].x,
								  -[window bounds].size.height * [[window layer] anchorPoint].y);
			
			// Render the layer hierarchy to the current context
			[[window layer] renderInContext:context];
			
			// Restore the context
			CGContextRestoreGState(context);
		}
	}
	// Retrieve the screenshot image
	UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
	
	UIGraphicsEndImageContext();
	
	return image;
}

// Get the entire content of an UIView*  (without subviews) as an UIImage*
+ (UIImage*) UIImageFromUIView: (UIView*) view {
	
	CGSize imageSize = [view bounds].size;
	if (NULL != UIGraphicsBeginImageContextWithOptions)
		UIGraphicsBeginImageContextWithOptions(imageSize, NO, 0);
	else
		UIGraphicsBeginImageContext(imageSize);
	
	CGContextRef context = [AmbulantView currentCGContext];
	
	if (!view.window || ![view.window respondsToSelector:@selector(screen)] || [view.window screen] == [UIScreen mainScreen])
	{
		// -renderInContext: renders in the coordinate space of the layer,
		// so we must first apply the layer's geometry to the graphics context
		CGContextSaveGState(context);
		// Center the context around the view's anchor point
		CGContextTranslateCTM(context, [view center].x, [view center].y);
		// Apply the view's transform about the anchor point
		CGContextConcatCTM(context, CGAffineTransformMakeScale(1.0, -1.0));
		// Offset by the portion of the bounds left of and above the anchor point
		CGContextTranslateCTM(context,
            -[view bounds].size.width * [[view layer] anchorPoint].x,
            -[view bounds].size.height * [[view layer] anchorPoint].y);
		// Render the layer hierarchy to the current context
		[[view layer] renderInContext:context];
		
		// Restore the context
		CGContextRestoreGState(context);
	}
	
	// Retrieve the screenshot image
	UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
	
	UIGraphicsEndImageContext();
	
	return image;
}

// Get an UIImage* from the contents of a CGLayerRef
// From: http://www.gotow.net/creative/wordpress/?p=33 "Creating a UIImage from a CGLayer"
+ (UIImage*) UIImageFromCGLayer: (CGLayerRef) layer
{
	// Create the bitmap context
	CGContextRef bitmapContext = NULL;
	void * bitmapData;
	int bitmapByteCount;
	int bitmapBytesPerRow;
	CGSize size = CGLayerGetSize(layer);
	
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow = (size.width * 4);
	bitmapByteCount = (bitmapBytesPerRow * size.height);
	
	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	bitmapData = malloc(bitmapByteCount);
	if (bitmapData == NULL) {
		return nil;
	}
	
	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits
	// per component. Regardless of what the source image format is
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
	bitmapContext = CGBitmapContextCreate (bitmapData, size.width, size.height,8,bitmapBytesPerRow,
        CGColorSpaceCreateDeviceRGB(),kCGImageAlphaNoneSkipFirst);
	if (bitmapContext == NULL) {
		// error creating context
		free(bitmapData);
		return nil;
	}
	
	CGContextScaleCTM(bitmapContext, 1, -1);
	CGContextTranslateCTM(bitmapContext, 0, -size.height);
	
	// Draw the image to the bitmap context. Once we draw, the memory
	// allocated for the context for rendering will then contain the
	// raw image data in the specified color space.
	CGContextDrawLayerAtPoint(bitmapContext, CGPointZero, layer);
	CGImageRef img = CGBitmapContextCreateImage(bitmapContext);
	UIImage* ui_img = [UIImage imageWithCGImage: img];
	
	CGImageRelease(img);
	CGContextRelease(bitmapContext);
	free(bitmapData);
	
	return ui_img;
}

// write a CGImageRef to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpCGImage: (CGImageRef) img withId: (NSString*) id {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	UIImage* ui_image = [UIImage imageWithCGImage: img];
	NSData *imageData = UIImagePNGRepresentation(ui_image);
	static int indx;
	int i = indx++;
	if (indx == 9999) indx = 0;
	NSMutableString* str = [NSMutableString stringWithString: NSHomeDirectory()];
	[str appendString: @"/Documents/"];
	[str appendFormat: @"%.4d-", (int) i];
	[str appendString: id];
	[str appendString: @".png"];
	if ( ! [imageData writeToFile:str atomically:YES]) {
		NSError* err = NULL;
		[imageData writeToFile:str options: NSDataWritingAtomic error: &err];
		if (err != NULL) {
			NSString* desc = [err localizedDescription];
			NSString* reas = [err localizedFailureReason];
			NSString* sugg = [err localizedRecoverySuggestion];
			NSLog(@"Error:%@ Reason:%@ Suggestion: %@",desc,reas,sugg);
		}
	}
	[pool release];
	return i;
}

// write the contents of an iPhone/iPad screen to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpScreenWithId: (NSString*) id
{
	UIImage *image = [AmbulantView UIImageFromScreen];
	return [AmbulantView dumpCGImage:image.CGImage withId: id];
}

// write the contents of an UIView to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpUIView: (UIView*) view withId: (NSString*) id
{
	UIImage *image = [AmbulantView UIImageFromUIView:view];
	return [AmbulantView dumpCGImage:image.CGImage withId: id];
}

// write the contents of an CGLayer to the file: "$HOME/Documents/<number>.<id>.png" where
// $HOME refers to the Application home directory and number is a numeric string
// circular variying between "0000" and "9999", which is returned as an int.   
+ (int) dumpCGLayer: (CGLayerRef) cglr withId: (NSString*) id
{
	UIImage* image = [AmbulantView UIImageFromCGLayer: cglr];
	return [AmbulantView dumpCGImage:image.CGImage withId: id];
}

// From: http://developer.apple.com/library/ios/#documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/dq_context/dq_context.html%23//apple_ref/doc/uid/TP30001066-CH203-TPXREF101
// Quartz 2D Programming Guide, Graphics Contexts, Create Bitmap Graphics Context
CGContextRef
CreateBitmapContext (CGSize size)
{
	CGContextRef context = NULL;
	CGColorSpaceRef colorSpace; 
	void* bitmapData; 
	int bitmapByteCount, bitmapBytesPerRow;
	
	bitmapBytesPerRow = (size.width * 4);
	bitmapByteCount	= (bitmapBytesPerRow * size.height);
	colorSpace = CGColorSpaceCreateDeviceRGB(); 

	context = CGBitmapContextCreate (NULL,
        size.width,
        size.height,
        8,      // bits per component
        bitmapBytesPerRow,
        colorSpace,
        kCGImageAlphaPremultipliedLast);
    if (context== NULL) {
        free (bitmapData);
        NSLog(@"CreateBitmapContext: Context not created");
        return NULL;
    }
    CGColorSpaceRelease(colorSpace);
	
    return context;
}
#endif // WITH_UIKIT
	
+ (CGContextRef) currentCGContext {
#ifdef	WITH_UIKIT
	CGContextRef context = UIGraphicsGetCurrentContext();
#else // ! WITH_UIKIT
	NSGraphicsContext* nsContext = [NSGraphicsContext currentContext];
	CGContextRef context = (CGContextRef)[nsContext graphicsPort];
#endif// ! WITH_UIKIT
	return context;
}

// Create a new CGLayer containing a CGImage
+ (CGLayerRef) CGLayerCreateFromCGImage: (CGImageRef) image flipped: (BOOL) flip {
	CGContextRef context = [AmbulantView currentCGContext];
	CGRect layer_rect = CGRectMake(0, 0, CGImageGetWidth(image), CGImageGetHeight(image));
	CGLayerRef newCGLayer = CGLayerCreateWithContext(context, layer_rect.size, NULL);
	// Draw the image in the layer
	CGContextRef newContext = CGLayerGetContext(newCGLayer);
	CGContextSaveGState(newContext);
	if (flip) {
		CGContextTranslateCTM(newContext, 0.0, layer_rect.size.height);
		CGContextScaleCTM(newContext, 1.0, -1.0);
	}
	CGContextDrawImage(newContext, layer_rect, image);
	CGContextRestoreGState(newContext);
	return newCGLayer;
}
	
- (void) startScreenTransition: (BOOL) isOuttrans
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	AM_DBG NSLog(@"startScreenTransition");
	if (fullscreen_count)
		NSLog(@"Warning: multiple Screen transitions in progress");
	fullscreen_count++;
	fullscreen_outtrans = isOuttrans;
	fullscreen_ended = NO;
	if (fullscreen_oldimage == NULL && ! isOuttrans) {
#ifdef	WITH_UIKIT
		UIImage* oldFullScreenImage = [AmbulantView UIImageFromUIView: self];
		fullscreen_oldimage = [AmbulantView CGLayerCreateFromCGImage: oldFullScreenImage.CGImage flipped: NO];
#else   // ! WITH_UIKIT
		NSImage* oldFullScreenNSImage = [self _getOnScreenImage];
		NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:[oldFullScreenNSImage TIFFRepresentation]];		
		CGImageRef oldFullScreenImage = [rep CGImage];
		fullscreen_oldimage = [AmbulantView CGLayerCreateFromCGImage: oldFullScreenImage flipped: YES];
#endif  // ! WITH_UIKIT
	}
	CGContextDrawLayerInRect(CGLayerGetContext(self.getTransitionSurface), NSRectToCGRect(self.bounds), fullscreen_oldimage);
	[pool release];
}
	
- (void) endScreenTransition
{
	AM_DBG NSLog(@"endScreenTransition");
	assert(fullscreen_count > 0);
	fullscreen_count--;
	fullscreen_ended = YES; // let the drawing thread fix-up
}

- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
    elapsed: (ambulant::lib::transition_info::time_type)now
{
	AM_DBG NSLog(@"screenTransitionStep %d engine=0x%@", (int)now, (void*) engine);
	assert(fullscreen_count > 0);
	fullscreen_engine = engine;
	fullscreen_now = now;
}

- (void) _screenTransitionPreRedraw
{
	if (fullscreen_count == 0) return;
	// setup drawing to transition surface
	AM_DBG NSLog(@"_screenTransitionPreRedraw: fullscreen_outtrans=%d fullscreen_oldimage=0x%@",fullscreen_outtrans,fullscreen_oldimage);

	if (fullscreen_outtrans || fullscreen_oldimage == NULL) {
		return;
	}
	CGContextDrawLayerInRect([AmbulantView currentCGContext],  NSRectToCGRect(self.bounds), fullscreen_oldimage);
	[self pushTransitionSurface];
}

- (void) _screenTransitionPostRedraw
{
	AM_DBG NSLog(@"_screenTransitionPostRedraw: fullscreen_count=%d fullscreen_engine=0x%@", fullscreen_count, (void*) fullscreen_engine);
	if (fullscreen_count == 0 && fullscreen_oldimage == NULL) {
		// Neither in fullscreen transition nor winding one down.
		// Take a snapshot of the screen and return.
        //XXX No idea yet what to do here
        /*DBG	[self dump: fullscreen_previmage toImageID: "fsprev"]; */
		return;
	}
	if (transition_pushed) {
		[self popTransitionSurface];
	}
	
	// Do the transition step, or simply copy the bits
	// if no engine available.
	AM_DBG NSLog(@"_screenTransitionPostRedraw: fullscreen_count=%d fullscreen_engine=0x%@", fullscreen_count, (void*) fullscreen_engine);
	if (fullscreen_engine && ! fullscreen_ended) {
		fullscreen_engine->step(fullscreen_now);
	} else {
		AM_DBG NSLog(@"_screenTransitionPostRedraw: no screen transition engine");
		if (fullscreen_ended) { // fix-up interrupted transition step
			ambulant::lib::rect fullsrcrect = ambulant::lib::rect(ambulant::lib::point(0, 0), ambulant::lib::size(self.bounds.size.width,self.bounds.size.height));  // Original image size
			CGRect cg_fullsrcrect = ambulant::gui::cg::CGRectFromAmbulantRect(fullsrcrect);
			CGContextRef ctx = [AmbulantView currentCGContext];	
			CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [self getTransitionSurface]);
			[self releaseTransitionSurfaces];
		}		
	}
	if (transition_count == 0) {
		[self releaseTransitionSurfaces];
	}	
	if (fullscreen_count == 0) {
		// Finishing a fullscreen transition.
        //XXX No idea yet what to do here
		AM_DBG NSLog(@"_screenTransitionPostRedraw: cleanup after transition done");
		fullscreen_engine = NULL;
	}
}
	
#ifdef	WITH_UIKIT
#endif // WITH_UIKIT

	
- (CGLayerRef) getTransitionSurface
{
	if (transition_surface == NULL) {
		// It does not exist yet. Create it.
		CGContextRef ctxr = [self getCGContext];
		transition_surface = CGLayerCreateWithContext(ctxr, NSSizeToCGSize(self.bounds.size), NULL);
		assert(transition_surface);
		// clear the surface
	}
	return transition_surface;
}

	
- (CGLayerRef) getTransitionOldSource
{
	if (fullscreen_count &&  fullscreen_oldimage)
		return fullscreen_oldimage;
//TBD	return [self getOnScreenImageForRect: NSRectToCGRect([self bounds])];
	return NULL;
}

- (void) clearTransitionSurface
{
	if (transition_surface != NULL) {
		CGContextRef ts_ctxr = CGLayerGetContext(transition_surface);
		CGSize s = CGLayerGetSize(transition_surface);
		CGRect r = CGRectMake(0.0, 0.0, s.width, s.height); 
		CGContextClearRect(ts_ctxr, r);
	}
}

- (void) releaseTransitionSurfaces
{
	AM_DBG NSLog(@"releaseTransitionSurfaces: transition_count=%d CFGetRetainCount(transition_surface)=%ld",transition_count, CFGetRetainCount(transition_surface));
	if (transition_count > 0) {
		return;
	}
	if (transition_surface != NULL) {
		CFRelease(transition_surface);
		transition_surface = NULL;
	}
	if (fullscreen_oldimage != NULL) {
		CFRelease(fullscreen_oldimage);
		fullscreen_oldimage = NULL;
	}
}
	
- (void) pushTransitionSurface
{
	AM_DBG NSLog(@"pushTransitionSurface 0x%@fullscreen_outtrans=%c transition_pushed=%d", self, fullscreen_outtrans, transition_pushed);
	if ( ! transition_pushed) {
#ifdef	WITH_UIKIT
		CGLayerRef surf = [self getTransitionSurface];
		UIGraphicsPushContext(CGLayerGetContext(surf));
#else// ! WITH_UIKIT
		old_context = [NSGraphicsContext currentContext];
		[[NSGraphicsContext currentContext] saveGraphicsState];
		CGContextRef ts_ctxr = CGLayerGetContext([self getTransitionSurface]);
		NSGraphicsContext* gc = 
			[NSGraphicsContext graphicsContextWithGraphicsPort: (void*) ts_ctxr
													   flipped: false]; 
		[NSGraphicsContext setCurrentContext:gc];																			 
#endif// ! WITH_UIKIT
		transition_pushed = YES;
	} else { // programmer error
		// XXXJACK why is this assert disabled?
		//		assert ( ! transition_pushed);
	}
}
	
- (void) popTransitionSurface
{
	AM_DBG NSLog(@"popTransitionSurface 0x%@ fullscreen_outtrans=%d transition_pushed=%d", self, fullscreen_outtrans, transition_pushed);
	if (transition_pushed) {
#ifdef	WITH_UIKIT
		UIGraphicsPopContext();
#else// ! WITH_UIKIT
		[NSGraphicsContext setCurrentContext: old_context];
		[[NSGraphicsContext currentContext] restoreGraphicsState];
#endif// ! WITH_UIKIT
		transition_pushed = NO;
	} else { // programmer error
		assert (transition_pushed);
	}
}
#ifndef WITH_UIKIT
// extensions for npambulant
@synthesize plugin_callback, plugin_data, plugin_mainloop;
	
- (ambulant::gui::cg::cg_window *) getAmbulant_window {
	return ambulant_window;
}

// execute callback on behalf of asyncRedrawForAmbulantRect:CGRect
void npambulant_invalidateRect(void* obj, CGRect r) {
	AmbulantView* v = (AmbulantView*) obj;
	void* browser_data = v.plugin_data;
	void (*npambulant_callback)(void*, void*) = (void(*)(void*, void*)) [v plugin_callback];
	npambulant_callback(browser_data, (void*) &r);
}

// create a "fake" AmbulantView for use by npambulant
void* new_AmbulantView(CGContextRef ctxp, CGRect r, void* plugin_callback, void* plugin_data) {
	CGContextRef myContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
	if (myContext != NULL) {
		CGContextSaveGState(myContext);
	}
	NSGraphicsContext* ns_ctx = [NSGraphicsContext graphicsContextWithGraphicsPort: (void*) ctxp flipped:YES];
	[NSGraphicsContext setCurrentContext:ns_ctx];
	AmbulantView* v = [AmbulantView alloc];
	r =  CGContextGetClipBoundingBox(ctxp);
	AM_DBG NSLog(@"new_AmbulantView(%p): ctxp=%p r=(%f,%f,%f,%f)", (void*) v, ctxp, r.origin.x, r.origin.y,r.size.width,r.size.height);
		[v initWithFrame: NSRectFromCGRect(r)];
	v.plugin_callback = plugin_callback; //X 
	v.plugin_data = plugin_data;		 //X browser data for callback function
	return (void*) v;
}

CGSize get_bounds_AmbulantView(void* view) {
	NSSize s = ((AmbulantView*)view).bounds.size;
	AM_DBG NSLog(@"get_bounds_AmbulantView(%p): size=%f,%f))", view, s.width, s.height);
	return NSSizeToCGSize(s);
}

// call AmbulantView.drawRect directly with the given CGContext 
void* draw_rect_AmbulantView(void* obj, CGContextRef ctx, CGRect* rectp) {
	CGRect cg_rect = *rectp;
	AmbulantView* v = (AmbulantView*) obj;
	ambulant::lib::point p(cg_rect.origin.x, cg_rect.origin.y);
	ambulant::lib::size s(cg_rect.size.width, cg_rect.size.height);
	ambulant::lib::rect r(p, s);
	AM_DBG NSLog(@"draw_rect_AmbulantView(%p): need_redraw(cg_rect=(%f,%f,%f,%f)r=(%d,%d,%d,%d))", obj, cg_rect.origin.x,cg_rect.origin.y,cg_rect.size.width,cg_rect.size.height,r.x,r.y,r.w,r.h);
	NSGraphicsContext* ns_ctx = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
	[NSGraphicsContext setCurrentContext:ns_ctx];
	[v drawRect: NSRectFromCGRect(cg_rect)];
	return (void*) v;
}

// mouse event handler for npambulant
void handle_event_AmbulantView(void* obj, CGContext* ctx, void* NSEventTypeRef, void* data, void* mainloop) {
	AmbulantView* v = (AmbulantView*) obj;
	NSEventType e_type = *(NSEventType*) NSEventTypeRef;
	event_data e_data = *(event_data*) data; 
	NSPoint p = {e_data.x, e_data.y};
	v.plugin_mainloop = (ambulant::common::gui_player*) mainloop; //X needed for cursor appearance change
	NSGraphicsContext* ns_ctx = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
	AM_DBG NSLog(@"handle_event_AmbulantView(%p): e_type=%u e_data=(%f,%f) p=(%f,%f)", obj, (UInt) e_type, e_data.x, e_data.y, p.x, p.y);
	// NSEvent mouseEventWithType will crash on type=NSMouseEntered or NSMouseExited showing:
	// Invalid parameter not satisfying: NSEventMaskFromType(type) & (MouseMask|NSMouseMovedMask)
	// The idea is to just update the cursor shape in these cases, so make it an NSMouseMoved instead.
	if (e_type == NSMouseEntered || e_type == NSMouseExited) {
		e_type = NSMouseMoved;
	}
	NSEvent* nse = [NSEvent mouseEventWithType: e_type location: p modifierFlags: 0 timestamp: 0 windowNumber: v.window.windowNumber context:ns_ctx eventNumber:1 clickCount:1 pressure:0.0];
	switch (e_type) {
		case NSLeftMouseDown:
			[v mouseDown: nse];
			break;
		case NSMouseEntered:
		case NSMouseMoved:
		case NSMouseExited:
			// update mouse position and cursor shape
			[v mouseMoved: nse];
			break;
		default:
			break;
	}
}

// destructor for npambulant
void delete_AmbulantView(void* obj) {
	AM_DBG NSLog(@"delete_AmbulantView(%p):)", obj);
	AmbulantView* v = (AmbulantView*) obj;
	[v release];
}

const char* to_char_AmbulantView(void* obj, void* nstr) {
	return [((NSString*)nstr) UTF8String];
}
#endif// ! WITH_UIKIT
// drawRect has different signatures in AppKit and UIKit. Hence the ugly
// c preprocessor constructs here.

#ifdef WITH_UIKIT
- (void)drawRect:(CGRect)rect
{
#else
- (void)drawRect:(NSRect)_rect
	{
		CGRect rect = NSRectToCGRect(_rect);
#endif // WITH_UIKIT
		CGContextRef myContext = [self getCGContext];
		CGContextSaveGState(myContext);
#ifndef WITH_UIKIT
		//
		// CG has default coordinate system with the origin bottom-left, we want topleft.
		// But: if we are running in an NSView then the coordinate system has already
		// been setup wrt. isFlipped, so we do nothing.
		//
		if (![self isFlipped]) {
			float view_height = CGRectGetHeight(NSRectToCGRect(self.bounds));
			CGAffineTransform matrix = CGAffineTransformMake(1, 0, 0, -1, 0, view_height);
			CGContextConcatCTM(myContext, matrix);
			// Also adapt the dirty rect
			matrix = CGAffineTransformInvert(matrix);
			rect = CGRectApplyAffineTransform(rect, matrix);
		}
#endif// WITH_UIKIT
		
		AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%@ ltrb=(%f,%f,%f,%f)", self, CGRectGetMinX(rect), CGRectGetMinY(rect), CGRectGetMaxX(rect), CGRectGetMaxY(rect));
		
		if (!ambulant_window) {
			AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
		} else {
			// If we have seen transitions we always redraw the whole view
			// XXXJACK interaction of fullscreen transitions and overlay windows
			// is completely untested, and probably broken.
			if (transition_count) rect = NSRectToCGRect([self bounds]);
			ambulant::lib::rect arect = ambulant::gui::cg::ambulantRectFromCGRect(rect);
			if (has_drawn) {
				[self _screenTransitionPreRedraw];
			}
			AM_DBG NSLog(@"ambulantView: call redraw ambulant-ltrb=(%d, %d, %d, %d)", arect.left(), arect.top(), arect.right(), arect.bottom());
			ambulant_window->redraw(arect);
			if (has_drawn) {
				[self _screenTransitionPostRedraw];
			} else {
				has_drawn = YES;
			}
			//		[self _releaseTransitionSurface];
		}
		
		CGContextRestoreGState(myContext);
	}
@end
