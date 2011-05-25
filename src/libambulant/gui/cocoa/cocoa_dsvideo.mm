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

/*
 * @$Id$
 */

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// There are two ways to render images: through CGImage or directly manipulating the NSBitmapImageRep. The former should
// be more efficient.
#define ENABLE_COCOA_CGIMAGE

static bool pixel_info_initialized = false;
static ambulant::net::pixel_order pixel_info_order;
static NSBitmapFormat pixel_info_format;
static int pixel_info_bpp;
#ifdef ENABLE_COCOA_CGIMAGE
static CGBitmapInfo pixel_info_bminfo;
#endif

static void
init_pixel_info() {
	if (pixel_info_initialized) return;
	pixel_info_initialized = true;
#ifdef ENABLE_COCOA_CGIMAGE
	if ([NSBitmapImageRep instancesRespondToSelector: @selector(initWithCGImage:)]) {
		// Only supported on 10.5, so fallback for 10.4
#if defined(__POWERPC__)
		// This is a hack: we see incorrect video color on PPC. We're going to drop
		// PPC support anyway, so this is a stopgap to make things work for the
		// 2.2 release. Sigh.
		pixel_info_order = ambulant::net::pixel_bgra;
#else
		pixel_info_order = ambulant::net::pixel_argb;
#endif
		pixel_info_format = (NSBitmapFormat)0;
		pixel_info_bpp = 4;
		pixel_info_bminfo = (kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Host);
		return;
	}
#endif
	pixel_info_order = ambulant::net::pixel_rgb;
	pixel_info_format = (NSBitmapFormat)0;
	pixel_info_bpp = 3;
}

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

extern const char cocoa_dsvideo_playable_tag[] = "video";
extern const char cocoa_dsvideo_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererOpen");
extern const char cocoa_dsvideo_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_dsvideo_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
create_cocoa_dsvideo_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	return new common::single_playable_factory<
		cocoa_dsvideo_renderer,
		cocoa_dsvideo_playable_tag,
		cocoa_dsvideo_playable_renderer_uri,
		cocoa_dsvideo_playable_renderer_uri2,
		cocoa_dsvideo_playable_renderer_uri3>(factory, mdp);
}


cocoa_dsvideo_renderer::cocoa_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	common::video_renderer(context, cookie, node, evp, factory, mdp),
	m_image(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer(): 0x%x created", (void*)this);
	init_pixel_info();
}

cocoa_dsvideo_renderer::~cocoa_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cocoa_dsvideo_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	m_image = NULL;
	m_lock.leave();
}

net::pixel_order
cocoa_dsvideo_renderer::pixel_layout()
{
	return pixel_info_order;
}

static void
my_free_frame(void *ptr, const void *ptr2, size_t size)
{
	free(ptr);
}

void
cocoa_dsvideo_renderer::_push_frame(char* frame, size_t size)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_image) {
		[m_image release];
		m_image = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer::_push_frame: size=%d, w*h*3=%d", size, m_size.w * m_size.h * 4);
	assert(size == (m_size.w * m_size.h * pixel_info_bpp));
	// XXXX Who keeps reference to frame?
	NSSize nssize = NSMakeSize(m_size.w, m_size.h);
	m_image = [[NSImage alloc] initWithSize: nssize];
	if (!m_image) {
		logger::get_logger()->trace("cocoa_dsvideo_renderer::_push_frame: cannot allocate NSImage");
		logger::get_logger()->error(gettext("Out of memory while showing video"));
		free(frame);
		[pool release];
		return;
	}
	NSBitmapImageRep *bitmaprep;
#ifdef ENABLE_COCOA_CGIMAGE
	if ([NSBitmapImageRep instancesRespondToSelector: @selector(initWithCGImage:)]) {
		// Step 1 - setup a data provider that reads our in-core image data
		CGDataProviderRef provider = CGDataProviderCreateWithData(frame, frame, size, my_free_frame);
		assert(provider);
		// Step 2 - create a CGImage that uses that data provider to initialize itself
		CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
		assert(genericColorSpace);
		// There may be room for improvement here, but I cannot find it. Did some experiments (on 4-core Intel Mac Pro)
		// with various values for kCGBitmapByteOrder32* and kCGImageAlpha*.
		// The only things that seem to make a difference:
		// - If the image does not have to be scaled then kCGBitmapByteOrder32Host|kCGImageAlphaNoneSkipFirst is a tiny bit faster. But
		//	 image draw times are dwarfed by background draw times (twice the image draw time!).
		// - If the image does need scaling things slow down by a factor of 4.
		//	 0 seems to be as good a value for bitmapInfo as any other value.
		// - If you also set shouldInterpolate=true you get an additional factor of 2 slowdown.
		CGBitmapInfo bitmapInfo = pixel_info_bminfo;
		CGImage *cgi = CGImageCreate( m_size.w, m_size.h, 8, pixel_info_bpp*8, m_size.w*pixel_info_bpp, genericColorSpace, bitmapInfo, provider, NULL, false, kCGRenderingIntentDefault);
		CGDataProviderRelease(provider);
		CGColorSpaceRelease(genericColorSpace);
		if (cgi == NULL) {
			logger::get_logger()->trace("cocoa_dsvideo_renderer::push_frame: cannot allocate CGImage");
			logger::get_logger()->error(gettext("Out of memory while showing video"));
			[pool release];
			return;
		}
		AM_DBG lib::logger::get_logger()->trace("0x%x: push_frame(0x%x, %d) -> 0x%x -> 0x%x", this, frame, size, provider, m_image);
		// Step 3 - Initialize an NSBitmapImageRep with this CGImage
		bitmaprep = [[NSBitmapImageRep alloc] initWithCGImage: cgi];
		CGImageRelease(cgi);
		if (!bitmaprep) {
			logger::get_logger()->trace("cocoa_dsvideo_renderer::push_frame: cannot allocate NSBitmapImageRep");
			logger::get_logger()->error(gettext("Out of memory while showing video"));
			return;
		}
		// Note that we do not free(frame), that happens when the provider calls my_free_frame.
	} else
#endif // ENABLE_COCOA_CGIMAGE
	{
		// On 10.4 or earlier (or if ENABLE_COCOA_CGIMAGE is not enabled) we go the old route with
		// an extra memcpy().
		bitmaprep = [[NSBitmapImageRep alloc]
			initWithBitmapDataPlanes: NULL
			pixelsWide: m_size.w
			pixelsHigh: m_size.h
			bitsPerSample: 8
			samplesPerPixel: 3
			hasAlpha: NO
			isPlanar: NO
			colorSpaceName: NSDeviceRGBColorSpace
			bitmapFormat: pixel_info_format
			bytesPerRow: m_size.w * pixel_info_bpp
			bitsPerPixel: pixel_info_bpp*8];
		if (!bitmaprep) {
			logger::get_logger()->trace("cocoa_dsvideo_renderer::_push_frame: cannot allocate NSBitmapImageRep");
			logger::get_logger()->error(gettext("Out of memory while showing video"));
			free(frame);
			[pool release];
			return;
		}
		memcpy([bitmaprep bitmapData], frame, size);
		free(frame);
	}
	[m_image addRepresentation: bitmaprep];
	[m_image setFlipped: true];
	[bitmaprep release];
	[pool release];
}

void
cocoa_dsvideo_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	assert(m_dest);
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
#ifdef WITH_VIDEO_TRANSITION_UNTESTED
	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if ([surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cocoa_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif

	if (m_image) {
		// Now find both source and destination area for the bitblit.
		NSSize cocoa_srcsize = [m_image size];
		size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
		rect srcrect = rect(size(0, 0));
		lib::rect croprect = m_dest->get_crop_rect(m_size);
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo::redraw, clip 0x%x (%d %d) -> (%d, %d, %d, %d)", m_dest, m_size.w, m_size.h, croprect.x, croprect.y, croprect.w, croprect.h);

		rect dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
		NSRect cocoa_srcrect = NSMakeRect(srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
		dstrect.translate(m_dest->get_global_topleft());
		NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));

		double alfa = 1.0;
		const common::region_info *ri = m_dest->get_info();
		if (ri) alfa = ri->get_mediaopacity();
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: (float)alfa];
	} else {
	}
#ifdef WITH_VIDEO_TRANSITION_UNTESTED
	if (surf) [surf unlockFocus];
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cocoa_dsvideo_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cocoa_dsvideo_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay);
	}
#endif

	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

