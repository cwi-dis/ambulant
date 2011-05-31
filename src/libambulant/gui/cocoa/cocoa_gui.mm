// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#undef WITH_COCOA_AUDIO

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_audio.h"
#include "ambulant/gui/cocoa/cocoa_text.h"
#include "ambulant/gui/cocoa/cocoa_html.h"
#include "ambulant/gui/cocoa/cocoa_image.h"
#include "ambulant/gui/cocoa/cocoa_ink.h"
#include "ambulant/gui/cocoa/cocoa_fill.h"
#include "ambulant/gui/cocoa/cocoa_video.h"
#include "ambulant/gui/cocoa/cocoa_dsvideo.h"
#include "ambulant/gui/cocoa/cocoa_smiltext.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/preferences.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#include <Cocoa/Cocoa.h>


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

//static ambulant::lib::critical_section redraw_lock;

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

common::window_factory *
create_cocoa_window_factory(void *view)
{

	return new cocoa_window_factory(view);
}

cocoa_window::~cocoa_window()
{
	if (m_view) {
		AmbulantView *my_view = (AmbulantView *)m_view;
		[my_view ambulantWindowClosed];
	}
	m_view = NULL;
}

void
cocoa_window::need_redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cocoa_cocoa_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (!m_view) {
		logger::get_logger()->fatal("cocoa_cocoa_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
	NSRect my_rect = [my_view NSRectForAmbulantRect: &r];
	NSRectHolder *arect = [[NSRectHolder alloc] initWithRect: my_rect];
	// XXX Is it safe to cast C++ objects to ObjC id's?
	[my_view performSelectorOnMainThread: @selector(asyncRedrawForAmbulantRect:)
		withObject: arect waitUntilDone: NO];
}

void
cocoa_window::redraw_now()
{
#if 0
    // Disabled by Jack: this seems to block the main thread and eat up a lot of CPU
	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view performSelectorOnMainThread: @selector(syncDisplayIfNeeded:)
		withObject: nil waitUntilDone: NO];
#endif
}

void
cocoa_window::redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cocoa_window::redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_handler->redraw(r, this);
}

bool
cocoa_window::user_event(const point &where, int what)
{
	AM_DBG logger::get_logger()->debug("cocoa_window::user_event(0x%x, (%d, %d), %d)", (void *)this, where.x, where.y, what);
	return m_handler->user_event(where, what);
}

void
cocoa_window::need_events(bool want)
{
	// This code needs to be run in the main thread.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	AM_DBG logger::get_logger()->debug("cocoa_window::need_events(0x%x, %d)", (void *)this, want);

	AmbulantView *my_view = (AmbulantView *)m_view;
	NSWindow *my_window = [my_view window];
	AM_DBG NSLog(@"my_window acceptsMouseMovedEvents = %d", [my_window acceptsMouseMovedEvents]);
	// See whether the mouse is actually in our area
	NSPoint where = [my_window mouseLocationOutsideOfEventStream];
	if (!NSPointInRect(where, [my_view frame])) {
		AM_DBG NSLog(@"mouse outside our frame");
		return;
	}
	// Get the main thread to do the real work
	[my_view performSelectorOnMainThread: @selector(pseudoMouseMove:)
		withObject: nil waitUntilDone: NO];

	[pool release];
}

lib::size
cocoa_window_factory::get_default_size()
{
	if (m_defaultwindow_view == NULL)
		return lib::size(default_layout_width, default_layout_height);
	NSSize size = [(AmbulantView *)m_defaultwindow_view bounds].size;
	return lib::size((int)size.width, (int)size.height);
}

gui_window *
cocoa_window_factory::new_window(const std::string &name, size bounds, gui_events *handler)
{
	if ([(AmbulantView *)m_defaultwindow_view isAmbulantWindowInUse]) {
		// XXXX Should create new toplevel window and put an ambulantview in it
		logger::get_logger()->error(gettext("Unsupported: AmbulantPlayer cannot open second toplevel window yet"));
		return NULL;
	}
	cocoa_window *window = new cocoa_window(name, bounds, m_defaultwindow_view, handler);
	// And we need to inform the object about it
	AmbulantView *view = (AmbulantView *)window->view();
	[view setAmbulantWindow: window];
	// And set the window size
	init_window_size(window, name, bounds);

	return (gui_window *)window;
}

void
cocoa_window_factory::init_window_size(cocoa_window *window, const std::string &name, lib::size bounds)
{
	AmbulantView *view = (AmbulantView *)window->view();
	// Get the position of our view in window coordinates
	NSPoint origin = NSMakePoint(0,0);
	NSView *superview = [view superview];
	int32_t shieldLevel = CGShieldingWindowLevel();
	if ([view ignoreResize] || [[view window] level] >= shieldLevel) {
		// We don't muck around with fullscreen windows or windows in other apps (browsers, etc).
		// What we should actually do is recenter the content, but that is for later.
	} else {
		if (superview) {
			NSRect rect = [superview convertRect: [view frame] toView: nil];
			origin = rect.origin;
		}
		// And set the window size
		AM_DBG NSLog(@"Size changed request: (%d, %d)", bounds.w, bounds.h);
		NSSize cocoa_size = NSMakeSize(bounds.w + origin.x, bounds.h + origin.y);
		[[view window] setContentSize: cocoa_size];
		// Assert that we actually get the size we asked for. Bugs in the Interface Builder file
		// could make this assert trigger.
		assert([view bounds].size.height == bounds.h);
		assert([view bounds].size.width == bounds.w);
		AM_DBG NSLog(@"Size changed on %@ to (%f, %f)", [view window], cocoa_size.width, cocoa_size.height);
	}
	[[view window] makeKeyAndOrderFront: view];
}

common::bgrenderer *
cocoa_window_factory::new_background_renderer(const common::region_info *src)
{
	return new cocoa_background_renderer(src);
}

void
cocoa_gui_screen::get_size(int *width, int *height)
{
	AmbulantView *view = (AmbulantView *)m_view;
	NSRect bounds = [view bounds];
	*width = int(bounds.size.width);
	*height = int(bounds.size.height);
}

bool
cocoa_gui_screen::get_screenshot(const char *type, char **out_data, size_t *out_size)
{
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
	if (image == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get screen shot");
		[pool release];
		return false;
	}
	NSBitmapImageRep *rep = (NSBitmapImageRep *)[image bestRepresentationForDevice: NULL];
	if (rep == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot get representation for screen shot");
//		[image release];
		[pool release];
		return false;
	}
    // Make sure we got a bitmap 
    assert([rep respondsToSelector: @selector(representationUsingType:properties:)]);
	data = [rep representationUsingType: filetype properties: NULL];
//	[image release];
	if (data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: cannot convert screenshot to %s format", type);
		[pool release];
		return false;
	}
	*out_data = (char *)malloc([data length]);
	if (*out_data == NULL) {
		lib::logger::get_logger()->trace("get_screenshot: out of memory");
//		[data release];
		[pool release];
		return false;
	}
	*out_size = [data length];
	[data getBytes: *out_data];
//	[data release];
	[pool release];
	return true;
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__

// Helper class: flipped view
@interface MyFlippedView : NSView
- (BOOL) isFlipped;
@end
@implementation MyFlippedView
- (BOOL) isFlipped
{
	return YES;
}
@end

// Helper class: NSRect as an object
@implementation NSRectHolder

- (id) initWithRect: (NSRect)r
{
	rect = r;
	return self;
}

- (NSRect)rect
{
	return rect;
}
@end

@implementation AmbulantView

- (id)initWithFrame:(NSRect)frameRect
{
	[super initWithFrame: frameRect];
	ambulant_window = NULL;
	transition_surface = NULL;
	transition_tmpsurface = NULL;
	transition_count = 0;
	fullscreen_count = 0;
	fullscreen_previmage = NULL;
	fullscreen_oldimage = NULL;
	fullscreen_engine = NULL;
	overlay_window = NULL;
	overlay_window_needs_unlock = NO;
	overlay_window_needs_reparent = NO;
	overlay_window_needs_flush = NO;
	overlay_window_needs_clear = NO;
	[self updateScreenSize];
	return self;
}

- (void)dealloc {
	if (transition_surface) [transition_surface release];
	transition_surface = NULL;
	if (transition_tmpsurface) [transition_tmpsurface release];
	transition_tmpsurface = NULL;
	if (overlay_window) [overlay_window release];
	overlay_window = NULL;
	if (fullscreen_previmage) [fullscreen_previmage release];
	fullscreen_previmage = NULL;
	if (fullscreen_oldimage) [fullscreen_oldimage release];
	fullscreen_oldimage = NULL;
	[super dealloc];

}- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::rect *)arect
{
	return NSMakeRect(arect->left(), arect->top(), arect->width(), arect->height());
}

- (ambulant::lib::rect) ambulantRectForNSRect: (const NSRect *)nsrect
{
	ambulant::lib::rect arect = ambulant::lib::rect(
		ambulant::lib::point(int(NSMinX(*nsrect)), int(NSMinY(*nsrect))),
		ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));

	return arect;
}

- (void) asyncRedrawForAmbulantRect: (NSRectHolder *)arect
{
	NSRect my_rect = [arect rect];
	[arect release];
	AM_DBG NSLog(@"AmbulantView.asyncRedrawForAmbulantRect: self=0x%x rect=(%f,%f,%f,%f)", self, NSMinX(my_rect), NSMinY(my_rect), NSMaxX(my_rect), NSMaxY(my_rect));
	[self setNeedsDisplayInRect: my_rect];
}

- (void) syncDisplayIfNeeded: (id) dummy
{
	[self displayIfNeeded];
//	[self display];
}

- (void)drawRect:(NSRect)rect
{
	AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x rect=(%f,%f,%f,%f)", self, NSMinX(rect), NSMinY(rect), NSMaxX(rect), NSMaxY(rect));
//	redraw_lock.enter();
#ifdef WITH_QUICKTIME_OVERLAY
	// If our main view has been reparented since the last redraw we need
	// to move the overlay window.
	if (overlay_window_needs_reparent) {
		assert(overlay_window);
		NSWindow *window = [self window];
		overlay_window_needs_reparent = NO;
		[[overlay_window parentWindow] removeChildWindow: overlay_window];
		[window addChildWindow: overlay_window ordered: NSWindowAbove];

		// XXXJACK This goes wrong when going back to windowed mode: for some reason
		// [self frame].origin still has the position as it was during fullscreen mode...
		NSPoint baseOrigin = NSMakePoint([self frame].origin.x, [self frame].origin.y);
		NSPoint screenOrigin = [window convertBaseToScreen: baseOrigin];
		AM_DBG NSLog(@"viewDidMoveToWindow: new origin (%f, %f)", screenOrigin.x, screenOrigin.y);
		[overlay_window setFrameOrigin: screenOrigin];
	}

	// If something was drawn into the overlay window during the last redraw
	// we need to clear the overlay window.
	if (overlay_window_needs_clear) {
		assert(overlay_window);
		NSView *oview = [overlay_window contentView];
		[oview lockFocus];
		[[NSColor clearColor] set];
		NSRect area = [oview bounds];
		AM_DBG NSLog(@"clear %f %f %f %f in %@", area.origin.x, area.origin.y, area.size.width, area.size.height, oview);
		NSRectFill(area);
		overlay_window_needs_clear = NO;
		overlay_window_needs_flush = YES;
		[oview unlockFocus];
	}
#endif // WITH_QUICKTIME_OVERLAY

	if (!ambulant_window) {
		AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
	} else {
		// If we have seen transitions we always redraw the whole view
		// XXXJACK interaction of fullscreen transitions and overlay windows
		// is completely untested, and probably broken.
		if (transition_count) rect = [self bounds];
		ambulant::lib::rect arect = [self ambulantRectForNSRect: &rect];
		[self _screenTransitionPreRedraw];
		ambulant_window->redraw(arect);
		[self _screenTransitionPostRedraw];
		[self _releaseTransitionSurface];
	}
#ifdef WITH_QUICKTIME_OVERLAY
	// If the overlay window was actually used (and possibly drawn into)
	// we need to unlock it. We also prepare for flushing it, and clearing
	// it the next redraw cycle.
	if (overlay_window_needs_unlock) {
		assert(overlay_window);
		[[overlay_window contentView] unlockFocus];
		overlay_window_needs_unlock = NO;
		overlay_window_needs_flush = YES;
		overlay_window_needs_clear = YES;
	}
	// Finally we flush the window, if required.
	if (overlay_window_needs_flush) {
		assert(overlay_window);
		[overlay_window flushWindow];
		overlay_window_needs_flush = NO;
	}
#endif // WITH_QUICKTIME_OVERLAY

//	redraw_lock.leave();
}

- (void)setAmbulantWindow: (ambulant::gui::cocoa::cocoa_window *)window
{
//	[[self window] setAcceptsMouseMovedEvents: true];
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

- (BOOL)isFlipped
{
	return true;
}

- (void)updateScreenSize
{
	// XXX Should be called for NSApplicationDidChangeScreenParametersNotification as well
	NSScreen *screen = [[self window] screen];
	if (screen) {
		NSRect rect = [screen frame];
		ambulant::smil2::test_attrs::set_current_screen_size(int(NSHeight(rect)), int(NSWidth(rect)));
	}
}

-(void)viewDidMoveToWindow
{
	[self updateScreenSize];
}

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
#ifdef WITH_QUICKTIME_OVERLAY
	if (overlay_window) {
		NSLog(@"Doing screenshot with overlay window");
		[[self window] makeKeyAndOrderFront: self];
		tmp_window = [[NSWindow alloc] initWithContentRect:[overlay_window frame] styleMask:NSBorderlessWindowMask
					backing:NSBackingStoreNonretained defer:NO];
		[tmp_window setBackgroundColor:[NSColor clearColor]];
		[tmp_window setLevel:[overlay_window level]];
		[tmp_window setHasShadow:NO];
		[tmp_window setAlphaValue:(float)0.0];
		src_view = [[NSView alloc] initWithFrame:[self bounds]];
		[tmp_window setContentView:src_view];
		[tmp_window orderFront:self];
	}
#endif /* WITH_QUICKTIME_OVERLAY */
	NSRect bounds = [self bounds];
	NSSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
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
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getOnScreenImageForRect: (NSRect)bounds
{
	// Note: this method does not take overlaying things such as Quicktime
	// movies into account.
	NSSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *flipped_rv = [[NSImage alloc] initWithSize: size];
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[self lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: bounds];
	[self unlockFocus];
	[flipped_rv addRepresentation: [bits autorelease]];
	[flipped_rv setFlipped: YES];
	[flipped_rv lockFocus];
	bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: NSMakeRect(0, 0, NSWidth(bounds), NSHeight(bounds))];
	[flipped_rv unlockFocus];
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
	[flipped_rv release];
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
	NSRect bounds = [self bounds];
	NSSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[rv setFlipped: YES];
	[transition_surface lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[transition_surface unlockFocus];
	[rv addRepresentation: [bits autorelease]];
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
	NSRect bounds = [self bounds];
	if (fullscreen_engine) {
		[[self getTransitionOldSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0f];
		fullscreen_engine->step(fullscreen_now);
	} else {
		AM_DBG NSLog(@"_screenTransitionPostRedraw: no screen transition engine");
//		[[self getTransitionNewSource] compositeToPoint: NSZeroPoint
//			operation: NSCompositeCopy];
		[[self getTransitionNewSource] drawInRect: bounds
			fromRect: bounds
			operation: NSCompositeCopy
			fraction: 1.0f];
	}

	if (fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG NSLog(@"_screenTransitionPostRedraw: cleanup after transition done");
		if (fullscreen_oldimage) [fullscreen_oldimage release];
		fullscreen_oldimage = NULL;
		fullscreen_engine = NULL;
	}
}

// Called by a renderer if it requires an overlay window.
// The overlay window is refcounted.
- (void) requireOverlayWindow
{
	[self performSelectorOnMainThread:@selector(_createOverlayWindow:) withObject: nil waitUntilDone:YES];
}

- (void) _createOverlayWindow: (id)dummy
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"requireOverlayWindow");
	if (overlay_window) {
		// XXXJACK shoould inc refcount here
		return;
	}
	// Find out where to position the window
	NSPoint baseOrigin = NSMakePoint([self frame].origin.x, [self frame].origin.y);
	NSPoint screenOrigin = [[self window] convertBaseToScreen: baseOrigin];

	// Create the window
	overlay_window = [[NSWindow alloc] initWithContentRect:
		NSMakeRect(screenOrigin.x,screenOrigin.y,[self frame].size.width,[self frame].size.height)
		styleMask:NSBorderlessWindowMask
		backing:NSBackingStoreBuffered
		defer:YES];
	NSView *oview = [[MyFlippedView alloc] initWithFrame: [self bounds]];
	[overlay_window setContentView: oview];
	[overlay_window setBackgroundColor: [NSColor clearColor]];
	//[overlay_window setBackgroundColor: [NSColor colorWithCalibratedRed: 0.5 green: 0.0 blue:0.0 alpha: 0.5]]; // XXXJACK
	[overlay_window setOpaque:NO];
	[overlay_window setHasShadow:NO];
	[overlay_window setIgnoresMouseEvents:YES];
	[[self window] addChildWindow: overlay_window ordered: NSWindowAbove];
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by a renderer redraw() if subsequent redraws in the current redraw sequence
// should go to the overlay window
- (void) useOverlayWindow
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"useOverlayWindow");
	assert(overlay_window);
	if (overlay_window_needs_unlock) {
		NSLog(@"userOverlayWindow: already lockFocus'sed");
		return;
	}
	NSView *oview = [overlay_window contentView];
	overlay_window_needs_unlock = YES;
	[oview lockFocus];
	// No need to clear, did that in drawRect: already
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by a renderer if the overlay window is no longer required.
- (void) releaseOverlayWindow
{
#ifdef WITH_QUICKTIME_OVERLAY
	AM_DBG NSLog(@"releaseOverlayWindow");
	// XXXJACK Currently we don't actually delete the overlay window once it
	// has been created (until the presentation stops). Need to work out whether
	// this is indeed a good idea.
#endif // WITH_QUICKTIME_OVERLAY
}

// Called by the window manager when our view has moved to a different window.
// XXXJACK not sure wheter implementing this or viewDidMoveToSuperview is better,
// both seem to work.
- (void) viewDidMoveToSuperview
{
	[self updateScreenSize];
#ifdef WITH_QUICKTIME_OVERLAY
	if (overlay_window == nil) return;
	AM_DBG NSLog(@"viewDidMoveToWindow");
	NSWindow *window = [self window];
	if (window == nil) {
		// Remove. Ignore, assume another call is coming when we're re-attached
		// to another window.
		AM_DBG NSLog(@"Ignore viewDidMoveToWindow -> nil");
		return;
	}
	overlay_window_needs_reparent = YES;
#endif // WITH_QUICKTIME_OVERLAY
}

@end
#endif // __OBJC__
