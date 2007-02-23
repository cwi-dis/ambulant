// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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
#include "ambulant/gui/cocoa/cocoa_fill.h"
#include "ambulant/gui/cocoa/cocoa_video.h"
#include "ambulant/gui/cocoa/cocoa_dsvideo.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/preferences.h"

#include <Cocoa/Cocoa.h>

// Defines for image dump debugging
#define DUMP_IMAGES_FORMAT @"/tmp/amdump/ambulant_dump_%03d_%s.tiff"
//#define DUMP_REDRAW
//#define DUMP_TRANSITION

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

common::playable_factory *
create_cocoa_renderer_factory(common::factories *factory)
{
    return new cocoa_renderer_factory(factory);
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
	AmbulantView *my_view = (AmbulantView *)m_view;
	[my_view performSelectorOnMainThread: @selector(syncDisplayIfNeeded:) 
		withObject: nil waitUntilDone: YES];
}

void
cocoa_window::redraw(const rect &r)
{
	AM_DBG logger::get_logger()->debug("cocoa_window::redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_handler->redraw(r, this);
}

void
cocoa_window::user_event(const point &where, int what)
{
	AM_DBG logger::get_logger()->debug("cocoa_window::user_event(0x%x, (%d, %d), %d)", (void *)this, where.x, where.y, what);
	m_handler->user_event(where, what);
}

void
cocoa_window::need_events(bool want)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	AM_DBG logger::get_logger()->debug("cocoa_window::need_events(0x%x, %d)", (void *)this, want);
		
	AmbulantView *my_view = (AmbulantView *)m_view;
	NSWindow *my_window = [my_view window];
	AM_DBG NSLog(@"my_window acceptsMouseMovedEvents = %d", [my_window acceptsMouseMovedEvents]);
	// Synthesize a mouseMoved event
	NSPoint where = [my_window mouseLocationOutsideOfEventStream];
	if (!NSPointInRect(where, [my_view frame])) {
		AM_DBG NSLog(@"mouse outside our frame");
		return;
	}
	// Convert from window to frame coordinates
	where.x -= NSMinX([my_view frame]);
	where.y -= NSMinY([my_view frame]);
#ifndef USE_COCOA_BOTLEFT
	// Mouse clicks are not flipped, even if the view is
	where.y = NSMaxY([my_view bounds]) - where.y;
#endif
	AM_DBG NSLog(@"pseudoMouseMoved at (%f, %f)", where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("resetMouse:") to: nil from: my_view];
	user_event(amwhere, 1);
	// XXX Set correct cursor
	[[NSApplication sharedApplication] sendAction: SEL("fixMouse:") to: nil from: my_view];
	[pool release];
}

playable *
cocoa_renderer_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp)
{
	playable *rv;
	
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		rv = new cocoa_image_renderer(context, cookie, node, evp, m_factory);
		AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_image_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "text") {
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "text/html") {
			rv = new cocoa_html_renderer(context, cookie, node, evp);
			AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_html_renderer 0x%x", (void *)node, (void *)rv);
		} else {
			rv = new cocoa_text_renderer(context, cookie, node, evp, m_factory);
			AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_text_renderer 0x%x", (void *)node, (void *)rv);
		}
	} else if ( tag == "brush") {
		rv = new cocoa_fill_renderer(context, cookie, node, evp);
		AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_fill_renderer 0x%x", (void *)node, (void *)rv);
#ifdef WITH_COCOA_AUDIO
	} else if ( tag == "audio") {
		rv = new cocoa_audio_playable(context, cookie, node, evp);
		AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_audio_renderer 0x%x", (void *)node, (void *)rv);
#endif
	} else if ( tag == "video") {
		if (common::preferences::get_preferences()->m_prefer_ffmpeg ) {
			rv = new cocoa_dsvideo_renderer(context, cookie, node, evp, m_factory);
			if (rv) {
				logger::get_logger()->trace("video: using native Ambulant renderer");
				AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_dsvideo_renderer 0x%x", (void *)node, (void *)rv);
			} else {
				rv = new cocoa_video_renderer(context, cookie, node, evp);
				if (rv) logger::get_logger()->trace("video: using QuickTime renderer");
				AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_video_renderer 0x%x", (void *)node, (void *)rv);
			}
		} else {
			rv = new cocoa_video_renderer(context, cookie, node, evp);
			if (rv) {
				logger::get_logger()->trace("video: using QuickTime renderer");
				AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_video_renderer 0x%x", (void *)node, (void *)rv);
			} else {
				rv = new cocoa_dsvideo_renderer(context, cookie, node, evp, m_factory);
				if (rv) logger::get_logger()->trace("video: using ffmpeg renderer");
				AM_DBG logger::get_logger()->debug("cocoa_renderer_factory: node 0x%x: returning cocoa_dsvideo_renderer 0x%x", (void *)node, (void *)rv);
			}
		}
	} else {
		// logger::get_logger()->error(gettext("cocoa_renderer_factory: no Cocoa renderer for tag \"%s\""), tag.c_str());
		return NULL;
	}
	return rv;
}

playable *
cocoa_renderer_factory::new_aux_audio_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
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
	if (superview) {
		NSRect rect = [superview convertRect: [view frame] toView: nil];
		origin = rect.origin;
	}
	// And set the window size
	AM_DBG NSLog(@"Size changed request: (%d, %d)", bounds.w, bounds.h);
	NSSize cocoa_size = NSMakeSize(bounds.w + origin.x, bounds.h + origin.y);
	[[view window] setContentSize: cocoa_size];
	AM_DBG NSLog(@"Size changed on %@ to (%f, %f)", [view window], cocoa_size.width, cocoa_size.height);
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
	return false;
}

bool
cocoa_gui_screen::set_overlay(const char *type, const char *data, size_t size)
{
	AmbulantView *view = (AmbulantView *)m_view;
	lib::logger::get_logger()->trace("set_overlay: not implemented yet");
	return false;
}

bool
cocoa_gui_screen::clear_overlay()
{
	AmbulantView *view = (AmbulantView *)m_view;
	lib::logger::get_logger()->trace("clear_overlay: not implemented yet");
	return false;
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__
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
	return self;
}

- (void)dealloc {
	if (transition_surface) [transition_surface release];
	transition_surface = NULL;
	if (transition_tmpsurface) [transition_tmpsurface release];
	transition_tmpsurface = NULL;
    [super dealloc];

}- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::rect *)arect
{
#ifdef USE_COCOA_BOTLEFT
	float bot_delta = NSMaxY([self bounds]) - arect->bottom();
	return NSMakeRect(arect->left(), bot_delta, arect->width(), arect->height());
#else
	return NSMakeRect(arect->left(), arect->top(), arect->width(), arect->height());
#endif
}

- (ambulant::lib::rect) ambulantRectForNSRect: (const NSRect *)nsrect
{
#ifdef USE_COCOA_BOTLEFT
	float top_delta = NSMaxY([self bounds]) - NSMaxY(*nsrect);
	ambulant::lib::rect arect = ambulant::lib::rect(
                ambulant::lib::point(int(NSMinX(*nsrect)), int(top_delta)),
				ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));
#else
	ambulant::lib::rect arect = ambulant::lib::rect(
                ambulant::lib::point(int(NSMinX(*nsrect)), int(NSMinY(*nsrect))),
				ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));
	 
#endif
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
//	[self displayIfNeeded];
	[self display];
}

- (void)drawRect:(NSRect)rect
{
    AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x rect=(%f,%f,%f,%f)", self, NSMinX(rect), NSMinY(rect), NSMaxX(rect), NSMaxY(rect));
//    redraw_lock.enter();
	if (!ambulant_window) {
        AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
    } else {
		// If we have seen transitions we always redraw the whole view
		if (transition_count) rect = [self bounds];
        ambulant::lib::rect arect = [self ambulantRectForNSRect: &rect];
		[self _screenTransitionPreRedraw];
        ambulant_window->redraw(arect);
		[self _screenTransitionPostRedraw];
#ifdef DUMP_REDRAW
		// Debug code: dump the contents of the view into an image
		[self dumpToImageID: "redraw"];
#endif
		[self _releaseTransitionSurface];
    }
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

- (BOOL)isFlipped
{
#ifdef USE_COCOA_BOTLEFT
	return false;
#else
	return true;
#endif
}

- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	where = [self convertPoint: where fromView: nil];
	if (!NSPointInRect(where, [self bounds])) {
		AM_DBG NSLog(@"mouseDown outside our frame");
		return;
	}
	AM_DBG NSLog(@"mouseDown at ambulant-point(%f, %f)", where.x, where.y);
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

- (void) dumpToImageID: (char *)ident
{
	[self lockFocus];
	NSBitmapImageRep *image = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[self unlockFocus];
	[self dump: image toImageID: ident];
}

- (void) dump: (id)image toImageID: (char *)ident
{
	static int seqnum = 0;
	NSString *filename = [NSString stringWithFormat: DUMP_IMAGES_FORMAT, seqnum++, ident];
	NSData *tiffrep = [image TIFFRepresentation];
	[tiffrep writeToFile: filename atomically: NO];
	AM_DBG NSLog(@"dump:toImageFile: created %@", filename);
}

- (BOOL)wantsDefaultClipping
{
#ifdef DUMP_REDRAW
	return NO;
#else
	return (transition_count == 0);
#endif
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
		transition_surface = [self _getOnScreenImage];
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
		transition_tmpsurface = [self _getOnScreenImage];
		[transition_tmpsurface retain];
		[transition_tmpsurface setFlipped: NO];
	}
	return transition_tmpsurface;
}

- (NSImage *)_getOnScreenImage
{
	NSRect bounds = [self bounds];
	NSSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
	NSImage *rv = [[NSImage alloc] initWithSize: size];
	[self lockFocus];
	NSBitmapImageRep *bits = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [self bounds]];
	[self unlockFocus];
	[rv addRepresentation: [bits autorelease]];
	[rv setFlipped: YES];
#ifdef DUMP_TRANSITION
	[self dump: rv toImageID: "oldsrc"];
#endif
	rv = [rv autorelease];
	return rv;
}

- (NSImage *)getOnScreenImageForRect: (NSRect)bounds
{
	NSSize size = NSMakeSize(NSWidth(bounds), NSHeight(bounds));
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
	return [self _getOnScreenImage];
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
		fullscreen_previmage = [[self _getOnScreenImage] retain];
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
//	/*DBG*/	[self dump: [self getTransitionOldSource] toImageID: "fsold"];
//	/*DBG*/	[self dump: [self getTransitionNewSource] toImageID: "fsnew"];
	NSRect bounds = [self bounds];
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

@end
#endif // __OBJC__
