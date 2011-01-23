// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_text.h"
//#include "ambulant/gui/cg/cg_html.h"
#include "ambulant/gui/cg/cg_image.h"
//#include "ambulant/gui/cg/cg_ink.h"
#include "ambulant/gui/cg/cg_fill.h"
//#include "ambulant/gui/cg/cg_video.h"
#include "ambulant/gui/cg/cg_dsvideo.h"
#ifdef WITH_SMIL30
//#include "ambulant/gui/cg/cg_smiltext.h"
#endif
#include "ambulant/lib/mtsync.h"
//#include <CoreGraphics/CoreGraphics.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
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
	[my_view performSelectorOnMainThread: @selector(asyncRedrawForAmbulantRect:)
		withObject: arect waitUntilDone: NO];
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

	AmbulantView *view = (AmbulantView *)m_view;
	[view ambulantSetSize: bounds];
}

lib::size
cg_window_factory::get_default_size()
{
	if (m_defaultwindow_view == NULL)
		return lib::size(common::default_layout_width, common::default_layout_height);
	CGSize size = [(AmbulantView *)m_defaultwindow_view bounds].size;
	return lib::size((int)size.width, (int)size.height);
}

common::gui_window *
cg_window_factory::new_window(const std::string &name, size bounds, common::gui_events *handler)
{
	NSLog(@"view %@ responds %d", (AmbulantView *)m_defaultwindow_view, [(AmbulantView *)m_defaultwindow_view respondsToSelector: @selector(isAmbulantWindowInUse)]);
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
	CGRect bounds = [view bounds];
	*width = int(bounds.size.width);
	*height = int(bounds.size.height);
}

bool
cg_gui_screen::get_screenshot(const char *type, char **out_data, size_t *out_size)
{
#if NOT_YET_UIKIT
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
		goto bad;
	}
	NSData *data;
	AmbulantView *view = (AmbulantView *)m_view;
	NSImage *image = [view _getOnScreenImage];
	if (image == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get screen shot");
		goto bad;
	}
	NSImageRep *rep = [image bestRepresentationForDevice: NULL];
	if (rep == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get representation for screen shot");
//		[image release];
		goto bad;
	}
	data = [rep representationUsingType: filetype properties: NULL];
//	[image release];
	if (data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot convert screenshot to %s format", type);
		goto bad;
	}
	*out_data = (char *)malloc([data length]);
	if (*out_data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: out of memory");
//		[data release];
		goto bad;
	}
	*out_size = [data length];
	[data getBytes: *out_data];
//	[data release];
	[pool release];
	return true;
bad:
	[pool release];
#endif
	return false;
}

} // namespace cg

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__


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

- (id)initWithFrame:(CGRect)frameRect
{
	/*AM_DBG*/ NSLog(@"AmbulantView.initWithFrame(0x%x)", self);
	self = [super initWithFrame: frameRect];
	ambulant_window = NULL;
//	transition_surface = NULL;
//	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
//	fullscreen_oldimage = NULL;
//	fullscreen_engine = NULL;
//	overlay_window = NULL;
//	overlay_window_needs_unlock = NO;
//	overlay_window_needs_reparent = NO;
//	overlay_window_needs_flush = NO;
//	overlay_window_needs_clear = NO;
	return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
	/*AM_DBG*/ NSLog(@"AmbulantView.initWithCoder(0x%x)", self);
	self = [super initWithCoder:aDecoder];
	ambulant_window = NULL;
//	transition_surface = NULL;
//	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
//	fullscreen_oldimage = NULL;
//	fullscreen_engine = NULL;
//	overlay_window = NULL;
//	overlay_window_needs_unlock = NO;
//	overlay_window_needs_reparent = NO;
//	overlay_window_needs_flush = NO;
//	overlay_window_needs_clear = NO;
	return self;
}

- (void)dealloc {
//	if (transition_surface) [transition_surface release];
//	transition_surface = NULL;
//	if (transition_tmpsurface) [transition_tmpsurface release];
//	transition_tmpsurface = NULL;
//	if (overlay_window) [overlay_window release];
//	overlay_window = NULL;
	[super dealloc];

}

- (CGContextRef) getCGContext
{
#ifdef WITH_UIKIT
	return UIGraphicsGetCurrentContext();
#else
	return (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
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
	AM_DBG NSLog(@"AmbulantView.asyncRedrawForAmbulantRect: self=0x%x ltrb=(%f,%f,%f,%f)", self, CGRectGetMinX(my_rect), CGRectGetMinY(my_rect), CGRectGetMaxX(my_rect), CGRectGetMaxY(my_rect));
	[self setNeedsDisplayInRect: my_rect];
}

- (void) syncDisplayIfNeeded: (id) dummy
{
#ifdef WITH_UIKIT
	[self setNeedsDisplay];
#else // AppKit
	[self setNeedsDisplay:true];
#endif
}

- (void)drawRect:(CGRect)rect
{
    CGContextRef myContext = [self getCGContext];
    CGContextSaveGState(myContext);
#ifndef WITH_UIKIT
    //
    // CG has default coordinate system with the origin bottom-left, we want topleft.
    // But: if we are running in an NSView then the coordinate system has already
    // been setup wrt. isFlipped, so we do nothing.
    //
    if (![self isFlipped]) {
        float view_height = CGRectGetHeight(self.bounds);
        CGAffineTransform matrix = CGAffineTransformMake(1, 0, 0, -1, 0, view_height);
        CGContextConcatCTM(myContext, matrix);
        // Also adapt the dirty rect
        matrix = CGAffineTransformInvert(matrix);
        rect = CGRectApplyAffineTransform(rect, matrix);
    }
#endif
    
	AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x ltrb=(%f,%f,%f,%f)", self, CGRectGetMinX(rect), CGRectGetMinY(rect), CGRectGetMaxX(rect), CGRectGetMaxY(rect));
#undef CG_REDRAW_DEBUG
#ifdef CG_REDRAW_DEBUG
	{
		CGFloat components[] = { 0, 1, 1, 1};
		CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
		CGContextSetStrokeColorSpace(myContext, genericColorSpace);
		CGColorSpaceRelease(genericColorSpace);
		CGContextSetStrokeColor(myContext, components);
        CGPoint points[] = {
            rect.origin,
            {rect.origin.x + rect.size.width, rect.origin.y},
            {rect.origin.x, rect.origin.y + rect.size.height},
            {rect.origin.x + rect.size.width, rect.origin.y + rect.size.height},
            rect.origin};
        CGContextAddLines(myContext, points, sizeof(points)/sizeof(points[0]));
        CGContextStrokePath(myContext);
//		CGContextSynchronize(myContext);
//		CGContextFlush(myContext);
//		sleep(1);
	}
#endif

	if (!ambulant_window) {
		AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
	} else {
		// If we have seen transitions we always redraw the whole view
		// XXXJACK interaction of fullscreen transitions and overlay windows
		// is completely untested, and probably broken.
		if (transition_count) rect = [self bounds];
		ambulant::lib::rect arect = ambulant::gui::cg::ambulantRectFromCGRect(rect);
//		[self _screenTransitionPreRedraw];
		AM_DBG NSLog(@"ambulantView: call redraw ambulant-ltrb=(%d, %d, %d, %d)", arect.left(), arect.top(), arect.right(), arect.bottom());
		ambulant_window->redraw(arect);
//		[self _screenTransitionPostRedraw];
//		[self _releaseTransitionSurface];
	}

#ifdef CG_REDRAW_DEBUG
	{
		CGFloat components[] = { 1, 0, 1, 0.2};
		CGContextSetStrokeColor(myContext, components);
        CGPoint points[] = {
            rect.origin,
            {rect.origin.x + rect.size.width, rect.origin.y},
            {rect.origin.x, rect.origin.y + rect.size.height},
            {rect.origin.x + rect.size.width, rect.origin.y + rect.size.height},
            rect.origin};
        CGContextAddLines(myContext, points, sizeof(points)/sizeof(points[0]));
        CGContextStrokePath(myContext);
	}
#endif
    CGContextRestoreGState(myContext);
}

- (void)setAmbulantWindow: (ambulant::gui::cg::cg_window *)window
{
	ambulant_window = window;
}

- (void)ambulantWindowClosed
{
	AM_DBG NSLog(@"ambulantWindowClosed called");
	ambulant_window = NULL;
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

#ifdef	WITH_UIKIT
@synthesize original_bounds;

#endif//WITH_UIKIT

- (void)ambulantSetSize: (ambulant::lib::size) bounds
{
    // Remember frame and bounds and adapt the window reqested in the current view
	AM_DBG NSLog(@"setSize before: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
    original_bounds = bounds;
    CGRect newBounds = CGRectMake(0, 0, bounds.w, bounds.h);
    self.bounds = newBounds;
    CGRect newFrame = self.frame;
    newFrame.size = newBounds.size;
    self.frame = newFrame;
    AM_DBG NSLog(@"setSize after set bounds: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
	if ([[self superview] respondsToSelector:@selector(recomputeZoom)])
		[[self superview] recomputeZoom];

    AM_DBG NSLog(@"setSize after aDAR: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
#ifndef WITH_UIKIT
	// Get the position of our view in window coordinates
	NSPoint origin = NSMakePoint(0,0);
	NSView *superview = [self superview];
	NSWindow *window = [self window];
	int32_t shieldLevel = CGShieldingWindowLevel();
	if ([self ignoreResize] || [window level] >= shieldLevel) {
		// We don't muck around with fullscreen windows or windows in other apps (browsers, etc).
		// What we should actually do is recenter the content, but that is for later.
	} else {
		if (superview) {
			NSRect rect = [superview convertRect: [self frame] toView: nil];
			origin = rect.origin;
		}
		// And set the window size
		AM_DBG NSLog(@"Size changed request: (%d, %d)", bounds.w, bounds.h);
		NSSize ns_size = NSMakeSize(bounds.w + origin.x, bounds.h + origin.y);
		[window setContentSize: ns_size];
		AM_DBG NSLog(@"Size changed on %@ to (%f, %f)", window, ns_size.width, ns_size.height);
	}
	[window makeKeyAndOrderFront: self];
#endif // !WITH_UIKIT
}

#ifdef WITH_UIKIT
// Equivalent of mouse move/click on iPhone
- (BOOL) tappedAtPoint:(CGPoint) location {
// NSLog(@"tappedAtPoint: x=%f y=%f", location.x, location.y);
	[self setNeedsDisplay];
	ambulant::lib::point amwhere = ambulant::lib::point(
							(int) location.x, (int) location.y);
	if (ambulant_window) {
		return ambulant_window->user_event(amwhere, 0);
	}
	return false;
}

- (void)drawTestRect:(CGRect)rect;
{   
    CGContextRef context = UIGraphicsGetCurrentContext(); 
    CGContextSetRGBStrokeColor(context, 1.0, 1.0, 0.0, 1.0); // yellow line
	
    CGContextBeginPath(context);
	
    CGContextMoveToPoint(context, rect.origin.x, rect.origin.y); //start point
    CGContextAddLineToPoint(context, rect.origin.x, rect.origin.y+rect.size.height);
    CGContextAddLineToPoint(context, rect.origin.x+rect.size.width, rect.origin.y+rect.size.height);
    CGContextAddLineToPoint(context, rect.origin.x+rect.size.width, rect.origin.y); // end path
	
    CGContextClosePath(context); // close path
	
    CGContextSetLineWidth(context, 8.0); // this is set from now on until you explicitly change it
	
    CGContextStrokePath(context); // do actual stroking
	
	CGContextSetRGBFillColor(context, 0.0, 1.0, 0.0, 0.5); // green color, half transparent
	CGContextFillRect(context, CGRectMake(rect.origin.x, rect.origin.y, rect.size.width/4, rect.size.height/4)); // a square at the bottom left-hand corner
}
#endif//WITH_UIKIT

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

#ifndef WITH_UIKIT
- (BOOL)isFlipped
{
	return true;
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize
{
    /*AM_DBG*/ NSLog(@"resizeWithOldSuperviewSize: %@", self);
    /*AM_DBG*/ NSLog(@"frame: %f, %f, %f, %f", self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);
    /*AM_DBG*/ NSLog(@"bounds: old %f,%f new %f, %f, %f, %f", oldBoundsSize.width, oldBoundsSize.height, self.bounds.origin.x, self.bounds.origin.y, self.bounds.size.width, self.bounds.size.height);
    [super resizeWithOldSuperviewSize: oldBoundsSize];
    NSSize realSize = {original_bounds.w, original_bounds.h};
    [self setBoundsSize: realSize];
#if 0
    float xscale = self.bounds.size.width / self.frame.size.width;
    float yscale = self.bounds.size.height / self.frame.size.height;
    float scale = (xscale < yscale) ? xscale : yscale;
    NSSize realFrameSize = { self.bounds.size.width * scale, self.bounds.size.height * scale};
    [self setFrameSize: realFrameSize];
#endif
        
}

#endif // WITH_UIKIT

#ifdef WITH_UIKIT
- (void)tappedWithPoint: (CGPoint) where
{
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	AM_DBG NSLog(@"0x%x: tappedWithPoint at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}
#else

- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	where = [self convertPoint: where fromView: nil];
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"0x%x: mouseDown outside our frame", (void*)self);
		return;
	}
	AM_DBG NSLog(@"0x%x: mouseDown at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}

- (void)mouseMoved: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	where = [self convertPoint: where fromView: nil];
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseMoved outside our frame");
		return;
	}
	AM_DBG NSLog(@"0x%x: mouseMoved at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	[[NSApplication sharedApplication] sendAction: SEL("resetMouse:") to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("fixMouse:") to: nil from: self];
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
	AM_DBG NSLog(@"0x%x: pseudoMouseMove at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	[[NSApplication sharedApplication] sendAction: SEL("resetMouse:") to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("fixMouse:") to: nil from: self];
}
#endif

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
	// XXXX Should we delete transition_surface?
	// XXXX Should we delete transition_tmpsurface?
}

#if NOT_YET_UIKIT
- (NSImage *)getTransitionSurface
{
	if (!transition_surface) {
		// It does not exist yet. Create it.
		transition_surface = [self getOnScreenImageForRect: [self bounds]];
		[transition_surface retain];
	}
	return transition_surface;
}

- (void)_releaseTransitionSurface
{
	if (transition_surface) {
		[transition_surface release];
		transition_surface = NULL;
	}
}

- (NSImage *)getTransitionTmpSurface
{
	if (!transition_tmpsurface) {
		// It does not exist yet. Create it.
		transition_tmpsurface = [self getOnScreenImageForRect: [self bounds]];
		[transition_tmpsurface retain];
		[transition_tmpsurface setFlipped: NO];
	}
	return transition_tmpsurface;
}

- (NSImage *)_getOnScreenImage
{
	NSView *src_view = self;
	NSWindow *tmp_window = NULL;
	CGRect bounds = [self bounds];
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
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
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[self lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: bounds];
	[self unlockFocus];
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getTransitionOldSource
{
	if (fullscreen_count && fullscreen_oldimage)
		return fullscreen_oldimage;
	return [self getOnScreenImageForRect: [self bounds]];
}

- (NSImage *)getTransitionNewSource
{
	CGRect bounds = [self bounds];
	CGSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[rv setFlipped: YES];
	[transition_surface lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[transition_surface unlockFocus];
	[rv addRepresentation: [bits autorelease]];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "newsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (void) startScreenTransition
{
	AM_DBG NSLog(@"startScreenTransition");
	if (fullscreen_count)
		NSLog(@"Warning: multiple Screen transitions in progress");
	fullscreen_count++;
	if (fullscreen_oldimage) [fullscreen_oldimage release];
	fullscreen_oldimage = fullscreen_previmage;
	fullscreen_previmage = NULL;
}

- (void) endScreenTransition
{
	AM_DBG NSLog(@"endScreenTransition");
	assert(fullscreen_count > 0);
	fullscreen_count--;
}

- (void) screenTransitionStep: (ambulant::smil2::transition_engine *)engine
		elapsed: (ambulant::lib::transition_info::time_type)now
{
	AM_DBG NSLog(@"screenTransitionStep %d", (int)now);
	assert(fullscreen_count > 0);
	fullscreen_engine = engine;
	fullscreen_now = now;
}

- (void) _screenTransitionPreRedraw
{
	if (fullscreen_count == 0) return;
	// XXX setup drawing to transition surface
	AM_DBG NSLog(@"_screenTransitionPreRedraw: setup for transition redraw");
	[[self getTransitionSurface] lockFocus];
}

- (void) _screenTransitionPostRedraw
{
	if (fullscreen_count == 0 && fullscreen_oldimage == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		if (fullscreen_previmage) [fullscreen_previmage release];
		fullscreen_previmage = [[self getOnScreenImageForRect: [self bounds]] retain];
		/*DBG	[self dump: fullscreen_previmage toImageID: "fsprev"]; */
		return;
	}
	if (fullscreen_oldimage == NULL) {
		// Just starting a new fullscreen transition. Get the
		// background bits from the snapshot saved during the previous
		// redraw.
		fullscreen_oldimage = fullscreen_previmage;
		fullscreen_previmage = NULL;
	}

	// Do the transition step, or simply copy the bits
	// if no engine available.
	AM_DBG NSLog(@"_screenTransitionPostRedraw: bitblit");
	[[self getTransitionSurface] unlockFocus];
//	/*DBG*/ [self dump: [self getTransitionOldSource] toImageID: "fsold"];
//	/*DBG*/ [self dump: [self getTransitionNewSource] toImageID: "fsnew"];
	CGRect bounds = [self bounds];
	if (fullscreen_engine) {
		[[self getTransitionOldSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0];
		fullscreen_engine->step(fullscreen_now);
	} else {
		AM_DBG NSLog(@"_screenTransitionPostRedraw: no screen transition engine");
//		[[self getTransitionNewSource] compositeToPoint: NSZeroPoint
//			operation: NSCompositeCopy];
		[[self getTransitionNewSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0];
	}

	if (fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG NSLog(@"_screenTransitionPostRedraw: cleanup after transition done");
		if (fullscreen_oldimage) [fullscreen_oldimage release];
		fullscreen_oldimage = NULL;
		fullscreen_engine = NULL;
	}
}

#endif // NOT_YET_UIKIT
@end
#endif // __OBJC__
