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

#ifdef WITH_UIKIT
#define NSSizeToCGSize(x) (x)
#define NSRectToCGRect(x) (x)
#define NSSizeFromCGSize(x) (x)
#define NSRectFromCGRect(x) (x)
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif//AM_DBG

#if 0
/*AM_DBG*/
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
	CGSize size = NSSizeToCGSize([(AmbulantView *)m_defaultwindow_view bounds].size);
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
	CGRect bounds = NSRectToCGRect([view bounds]);
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
#endif//NOT_YET_UIKIT
	return false;
}

} // namespace cg

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__
#ifdef	WITH_UIKIT
#import <QuartzCore/CALayer.h>
#endif//WITH_UIKIT

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
	self = [super initWithFrame: NSRectFromCGRect(frameRect)];
	ambulant_window = NULL;
	transition_surface = NULL;
//	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
//	fullscreen_oldimage = NULL;
	fullscreen_engine = NULL;
//	overlay_window = NULL;
//	overlay_window_needs_unlock = NO;
//	overlay_window_needs_reparent = NO;
//	overlay_window_needs_flush = NO;
//	overlay_window_needs_clear = NO;
	return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
	AM_DBG NSLog(@"AmbulantView.initWithCoder(0x%x)", self);
	self = [super initWithCoder:aDecoder];
	ambulant_window = NULL;
	transition_surface = NULL;
//	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
//	fullscreen_previmage = NULL;
//	fullscreen_oldimage = NULL;
	fullscreen_engine = NULL;
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
	AM_DBG NSLog(@"AmbulantView.asyncRedrawForAmbulantRect: self=0x%x ltrb=(%f,%f,%f,%f)", self, CGRectGetMinX(my_rect), CGRectGetMinY(my_rect), CGRectGetMaxX(my_rect), CGRectGetMaxY(my_rect));
	[self setNeedsDisplayInRect: NSRectFromCGRect(my_rect)];
}

- (void) syncDisplayIfNeeded: (id) dummy
{
#ifdef WITH_UIKIT
	[self setNeedsDisplay];
#else // AppKit
	[self setNeedsDisplay:true];
#endif//WITH_UIKIT
}

- (void)drawRect:(CGRect)rect
{
//DBG	[AmbulantView dumpScreenWithId: @"rd0"];

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
#endif//CG_REDRAW_DEBUG

	if (!ambulant_window) {
		AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
	} else {
		// If we have seen transitions we always redraw the whole view
		// XXXJACK interaction of fullscreen transitions and overlay windows
		// is completely untested, and probably broken.
		if (transition_count) rect = NSRectToCGRect([self bounds]);
		ambulant::lib::rect arect = ambulant::gui::cg::ambulantRectFromCGRect(rect);
		[self _screenTransitionPreRedraw];
		AM_DBG NSLog(@"ambulantView: call redraw ambulant-ltrb=(%d, %d, %d, %d)", arect.left(), arect.top(), arect.right(), arect.bottom());
		ambulant_window->redraw(arect);
		[self _screenTransitionPostRedraw];
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
#endif// CG_REDRAW_DEBUG
    CGContextRestoreGState(myContext);
//DBG	[AmbulantView dumpScreenWithId: @"rd1"];
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

#ifdef	JNK
#ifdef	WITH_UIKIT
@synthesize original_bounds;
@synthesize current_transform;
- (void) adaptDisplayAfterRotation: (UIDeviceOrientation) orientation withAutoCenter: (BOOL) autoCenter withAutoResize: (bool) autoResize {
	if (ambulant_window == NULL ) {
		return;
	}
	if (self.alpha == 0.0) {
		// view disabled, another view is made visible (e.g. tabBarViewController)
		return;
	}
	// adapt the ambulant window needed (bounds) in the current View
	M_auto_center = autoCenter;
	M_auto_resize = autoResize;
	bool auto_resize = (bool) autoResize;
	bool auto_center = (bool) autoCenter;
    if (autoResize) {
        zoomState = zoomFillScreen;
    } else {
        zoomState = zoomNaturalSize;
    }
	CGSize mybounds;
	mybounds.width = original_bounds.w;
	mybounds.height = original_bounds.h;
#if PRESERVE_ZOOM
	// pan/zoom combined with auto scale/auto center does not work smoothly.
	// for now, rotating the device implies undo of all pan/zoom settings.
	// This is useable, albeit maybe not always desirable.
	// Shake gesture or UIDeviveOrientationFaceDown would be obvious
	// implementation for Undo pan/zoom (Shake is commonly used fo Undo/Redo).
	CGRect myframe = current_frame;
#else
	CGRect myframe = current_frame = original_frame;
#endif ///PRESERVE_ZOOM
	CGRect mainframe = [[UIScreen mainScreen] applicationFrame];
	AM_DBG NSLog(@"Mainscreen: %f,%f,%f,%f", mainframe.origin.x,mainframe.origin.y,mainframe.size.width,mainframe.size.height);
	BOOL wasRotated = false;
	if (orientation == UIDeviceOrientationLandscapeLeft
		|| orientation == UIDeviceOrientationLandscapeRight) {
		wasRotated = true;
		if (auto_center || auto_resize) {
			myframe.size.height = mainframe.size.width; // depends on nib
			myframe.size.width = mainframe.size.height;
		}
		[[UIApplication sharedApplication] setStatusBarHidden: YES withAnimation: UIStatusBarAnimationNone];
	} else if (orientation == UIDeviceOrientationPortrait 
			   || orientation == UIDeviceOrientationPortraitUpsideDown) {
		if (auto_center || auto_resize) {
			myframe.size.width = mainframe.size.width;
			myframe.size.height = mainframe.size.height;
		}
		[[UIApplication sharedApplication] setStatusBarHidden: NO withAnimation: UIStatusBarAnimationNone];
	} else {
		return;
	}
	float scale = 1.0;
	if (auto_resize) {
		float scale_x = myframe.size.width / mybounds.width;
		float scale_y = myframe.size.height / mybounds.height;
		// find the smallest scale factor for both x- and y-directions
		scale = scale_x < scale_y ? scale_x : scale_y;
	}
#if PRESERVE_ZOOM
	//self.transform = CGAffineTransformScale(self.transform, scale, scale);
#else
	self.transform = CGAffineTransformMakeScale(scale, scale);
#endif ///PRESERVE_ZOOM
	
	// center my frame in the available space
	float delta = 0;
	if (auto_center) {
		if (wasRotated) {
			delta = (myframe.size.width - mybounds.width * scale) / 2;
			myframe.origin.x += delta;
			myframe.size.width -= delta;
			delta = (myframe.size.height - mybounds.height * scale) / 2;
			myframe.origin.y += delta;
			myframe.size.height -= delta;
		} else {
			delta = (myframe.size.height - mybounds.height * scale) / 2;		
			myframe.origin.y += delta;
			myframe.size.height -= delta;
			delta = (myframe.size.width - mybounds.width * scale) / 2;
			myframe.origin.x += delta;
			myframe.size.width -= delta;
		}
	}
	AM_DBG ambulant::lib::logger::get_logger()->debug("adaptDisplayAfterRotation: myframe=orig(%d,%d),size(%d,%d)",(int)myframe.origin.x, (int)myframe.origin.y,(int)myframe.size.width,(int)myframe.size.height);
	self.frame = myframe;
	// invalidate transition surfaces
	[self releaseTransitionSurfaces];
	// redisplay AmbulantView using the new settings
	[self setNeedsDisplay];
	
}
#endif//WITH_UIKIT
#endif//JNK

- (void)ambulantSetSize: (ambulant::lib::size) bounds
{
    // Remember frame and bounds and adapt the window reqested in the current view
	AM_DBG NSLog(@"setSize before: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
//JNK original_bounds = bounds;
    CGRect newBounds = CGRectMake(0, 0, bounds.w, bounds.h);
    CGRect newFrame = NSRectToCGRect(self.frame);
    newFrame.size = newBounds.size;
    self.frame = NSRectFromCGRect(newFrame);
    self.bounds = NSRectFromCGRect(newBounds);
    AM_DBG NSLog(@"setSize after set bounds: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
	if ([[self superview] respondsToSelector:@selector(recomputeZoom)])
		[[self superview] recomputeZoom];

    AM_DBG NSLog(@"setSize after aDAR: %@ %f,%f", self, self.bounds.size.width, self.bounds.size.height);
#ifdef JNK
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
#endif // JNK
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

#ifdef JNK
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
#endif // JNK
#endif // WITH_UIKIT

#ifdef WITH_UIKIT
- (void)tappedWithPoint: (CGPoint) where
{
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	AM_DBG NSLog(@"0x%x: tappedWithPoint at ambulant-point(%f, %f)", (void*)self, where.x, where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}
#else// ! WITH_UIKIT

- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInq];
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
//  AM_DBG { id target=[[NSApplication sharedApplication] targetForAction: @selector(resetMouse:) ]; if(!target) { NSLog(@"No target for resetMouse: ??"); dumpResponderChain([[self window] firstResponder]);}}
	[[NSApplication sharedApplication] sendAction: @selector(resetMouse:) to: nil from: self];
	if (ambulant_window) ambulant_window->user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: @selector(fixMouse:) to: nil from: self];
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

#else// NOT_YET_UIKIT

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
	
	CGContextRef context = UIGraphicsGetCurrentContext();
	
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
	
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	if (!view.window || ![view.window respondsToSelector:@selector(screen)] || [view.window screen] == [UIScreen mainScreen])
	{
		// -renderInContext: renders in the coordinate space of the layer,
		// so we must first apply the layer's geometry to the graphics context
		CGContextSaveGState(context);
		// Center the context around the view's anchor point
		CGContextTranslateCTM(context, [view center].x, [view center].y);
		// Apply the view's transform about the anchor point
		CGContextConcatCTM(context, CGAffineTransformMakeScale(1.0, -1.0));
//		CGContextConcatCTM(context, [view transform]);
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
	CGContextRef    bitmapContext = NULL;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	CGSize          size = CGLayerGetSize(layer);
	
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (size.width * 4);
	bitmapByteCount     = (bitmapBytesPerRow * size.height);
	
	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	bitmapData = malloc( bitmapByteCount );
	if (bitmapData == NULL)
	{
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
	CGImageRef   img = CGBitmapContextCreateImage(bitmapContext);
	UIImage*     ui_img = [UIImage imageWithCGImage: img];
	
	CGImageRelease(img);
	CGContextRelease(bitmapContext);
	free(bitmapData);
	
	return ui_img;
}

// Create a new CGLayer containing a CGImage
+ (CGLayerRef) CGLayerCreateFromCGImage: (CGImageRef) image {
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGRect layer_rect = CGRectMake(0, 0, CGImageGetWidth(image), CGImageGetHeight(image));
	CGLayerRef newCGLayer = CGLayerCreateWithContext(context, layer_rect.size, NULL);
	// Draw the image in the layer
	CGContextRef newContext = CGLayerGetContext(newCGLayer);
	CGContextDrawImage(newContext, layer_rect, image);
	return newCGLayer;
}

// write a CGImageRef to the file: "$HOME/Documents/<number>.<id>.png" where
// where $HOME refers to the Application home directory and
// and number is a numeric string circular variying between "0000" and "9999".   
+ (void) dumpCGImage: (CGImageRef) img withId: (NSString*) id {
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
}

// write the contents of an iPhone/iPad screen to the file: "$HOME/Documents/<number>.<id>.png" where
// where $HOME refers to the Application home directory and
// and number is a numeric string circular variying between "0000" and "9999".   
+ (void) dumpScreenWithId: (NSString*) id
{
	UIImage *image = [AmbulantView UIImageFromScreen];
	[AmbulantView dumpCGImage:image.CGImage withId: id];
	//	[image release];
}

// write the contents of an UIView to the file: "$HOME/Documents/<number>.<id>.png" where
// where $HOME refers to the Application home directory and
// and number is a numeric string circular variying between "0000" and "9999".   
+ (void) dumpUIView: (UIView*) view withId: (NSString*) id
{
	UIImage *image = [AmbulantView UIImageFromUIView:view];
	[AmbulantView dumpCGImage:image.CGImage withId: id];
	//	[image release];
}

// write the contents of an CGLayer to the file: "$HOME/Documents/<number>.<id>.png" where
// where $HOME refers to the Application home directory and
// and number is a numeric string circular variying between "0000" and "9999".   
+ (void) dumpCGLayer: (CGLayerRef) cglr withId: (NSString*) id
{
	UIImage* image = [AmbulantView UIImageFromCGLayer: cglr];
	[AmbulantView dumpCGImage:image.CGImage withId: id];
}

// From: http://developer.apple.com/library/ios/#documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/dq_context/dq_context.html%23//apple_ref/doc/uid/TP30001066-CH203-TPXREF101
// Quartz 2D Programming Guide, Graphics Contexts, Create Bitmap Graphics Context
CGContextRef CreateBitmapContext (CGSize size)
{
	CGContextRef context = NULL;
	CGColorSpaceRef colorSpace; 
	void* bitmapData; 
	int bitmapByteCount, bitmapBytesPerRow;
	
	bitmapBytesPerRow	= (size.width * 4);
	bitmapByteCount	= (bitmapBytesPerRow * size.height);
	colorSpace = CGColorSpaceCreateDeviceRGB(); 

	context = CGBitmapContextCreate (NULL,
									 size.width,
									 size.height,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaPremultipliedLast);
    if (context== NULL)
    {
        free (bitmapData);
        NSLog(@"CreateBitmapContext: Context not created");
        return NULL;
    }
    CGColorSpaceRelease( colorSpace );
	
    return context;
}	

- (CGLayerRef) getTransitionSurface
{
	if (transition_surface == NULL) {
		// It does not exist yet. Create it.
		CGContextRef ctxr = [self getCGContext];
		transition_surface = CGLayerCreateWithContext(ctxr, self.bounds.size, NULL);
	}
	return transition_surface;
}

- (CGLayerRef) getTransitionTmpSurface
{
	if (transition_tmpsurface == NULL) {
		// It does not exist yet. Create it.
		CGContextRef ctxr = [self getCGContext];
		transition_tmpsurface = CGLayerCreateWithContext(ctxr, self.bounds.size, NULL);
	}
	return transition_tmpsurface;
}

static CGLayerRef oldFullScreen;

- (void) releaseTransitionSurfaces
{
	if (transition_surface != NULL) {
		CFRelease(transition_surface);
		transition_surface = NULL;
	}
	if (oldFullScreen != NULL) {
		CFRelease(oldFullScreen);
	}
}

- (void) startScreenTransition
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	AM_DBG NSLog(@"startScreenTransition");
	if (fullscreen_count)
		NSLog(@"Warning: multiple Screen transitions in progress");
	fullscreen_count++;
	if (oldFullScreen == NULL) {
		UIImage* oldFullScreenImage = [AmbulantView UIImageFromUIView: self];
		oldFullScreen = [AmbulantView CGLayerCreateFromCGImage: oldFullScreenImage.CGImage];
		CFRetain(oldFullScreen);
	}
	CGContextDrawLayerInRect(CGLayerGetContext([self getTransitionSurface]), [self bounds], oldFullScreen);
//BDG [AmbulantView dumpCGLayer: [self getTransitionSurface] withId: @"old"];
	[pool release];
}

- (void) endScreenTransition
{
	AM_DBG NSLog(@"endScreenTransition");
	if (oldFullScreen != NULL) {
		[oldFullScreen release];
		oldFullScreen = NULL;
	}
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
	// setup drawing to transition surface
	AM_DBG NSLog(@"_screenTransitionPreRedraw: setup for transition redraw");
	CGLayerRef surf = [self getTransitionSurface];
//DBG	[AmbulantView dumpScreenWithId: @"pre"];
	CGContextDrawLayerInRect(UIGraphicsGetCurrentContext(), [self bounds], oldFullScreen);
	UIGraphicsPushContext(CGLayerGetContext(surf));
}

- (void) _screenTransitionPostRedraw
{
	if (fullscreen_count == 0) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
//XXX No idea yet what to do here
/*DBG	[self dump: fullscreen_previmage toImageID: "fsprev"]; */
		return;
	}
	UIGraphicsPopContext();
//X	if (fullscreen_oldimage == NULL) {
//XXX No idea yet what to do here
		// Just starting a new fullscreen transition. Get the
		// background bits from the snapshot saved during the previous
		// redraw.
//X		fullscreen_oldimage = fullscreen_previmage;
//X		fullscreen_previmage = NULL;
//X	}
	
	// Do the transition step, or simply copy the bits
	// if no engine available.
	AM_DBG NSLog(@"_screenTransitionPostRedraw:");
//X	[[self getTransitionSurface] unlockFocus];
//	/*DBG*/ [self dump: [self getTransitionOldSource] toImageID: "fsold"];
//	/*DBG*/ [self dump: [self getTransitionNewSource] toImageID: "fsnew"];
	CGRect bounds = [self bounds];
	if (fullscreen_engine) {
		fullscreen_engine->step(fullscreen_now);
	} else {
		AM_DBG NSLog(@"_screenTransitionPostRedraw: no screen transition engine");
//XXX No idea yet what to do here
	}
	
	if (fullscreen_count == 0) {
		// Finishing a fullscreen transition.
//XXX No idea yet what to do here
		AM_DBG NSLog(@"_screenTransitionPostRedraw: cleanup after transition done");
		fullscreen_engine = NULL;
	}
//DBG	[AmbulantView dumpScreenWithId: @"pst"];
}
#endif // NOT_YET_UIKIT
@end
#endif // __OBJC__
