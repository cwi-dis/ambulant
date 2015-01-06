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
#include "ambulant/gui/cg/cg_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// These two constants should match. Moreover, the optimal setting may depend on the
// specific hardware.
// XXXJACK: we should get rid of these, analoguous to what cocoa_dsvideo does:
// Get the information dynamically.
#if 1
#define MY_PIXEL_LAYOUT net::pixel_argb
#define MY_BITMAP_INFO (kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Host)
#define MY_BPP 4
#endif
#if 0
#define MY_PIXEL_LAYOUT net::pixel_rgba
#define MY_BITMAP_INFO (kCGImageAlphaNoneSkipLast|kCGBitmapByteOrder32Host)
#define MY_BPP 4
#endif
#if 0
#define MY_PIXEL_LAYOUT net::pixel_bgr
#define MY_BITMAP_INFO (kCGImageAlphaNone|kCGBitmapByteOrder32Host)
#define MY_BPP 3
#endif
namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_dsvideo_playable_tag[] = "video";
extern const char cg_dsvideo_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererOpen");
extern const char cg_dsvideo_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
// XXXJACK missing RendererVideo...
//xxxbo appending RendererVideo
extern const char cg_dsvideo_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
create_cg_dsvideo_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	return new common::single_playable_factory<
		cg_dsvideo_renderer,
		cg_dsvideo_playable_tag,
		cg_dsvideo_playable_renderer_uri,
		cg_dsvideo_playable_renderer_uri2,
		//cg_dsvideo_playable_renderer_uri2>(factory, mdp);
		cg_dsvideo_playable_renderer_uri3>(factory, mdp);
}

cg_dsvideo_renderer::cg_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	cg_renderer<common::video_renderer>(context, cookie, node, evp, factory, mdp),
	m_image(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer(): 0x%x created", (void*)this);
}

cg_dsvideo_renderer::~cg_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_dsvideo_renderer(0x%x)", (void *)this);
	if (m_image) CGImageRelease(m_image);
	m_image = NULL;
	m_lock.leave();
}

net::pixel_order
cg_dsvideo_renderer::pixel_layout()
{
	return MY_PIXEL_LAYOUT;
}

static void
my_free_frame(void *ptr, const void *ptr2, size_t size)
{
	free(ptr);
}

void
cg_dsvideo_renderer::_push_frame(char* frame, size_t size)
{
	m_lock.enter();
	if (m_image) {
		CGImageRelease(m_image);
		m_image = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer::push_frame: size=%d, w*h*3=%d", size, m_size.w * m_size.h * 4);
	assert(size == (m_size.w * m_size.h * MY_BPP));
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
	CGBitmapInfo bitmapInfo = MY_BITMAP_INFO;
	m_image = CGImageCreate( m_size.w, m_size.h, 8, MY_BPP*8, m_size.w*MY_BPP, genericColorSpace, bitmapInfo, provider, NULL, false, kCGRenderingIntentDefault);
	AM_DBG lib::logger::get_logger()->trace("0x%x: push_frame(0x%x, %d) -> 0x%x -> 0x%x", this, frame, (int)size, provider, m_image);
	CGDataProviderRelease(provider);
	CGColorSpaceRelease(genericColorSpace);
	if (!m_image) {
		logger::get_logger()->trace("cg_dsvideo_renderer::push_frame: cannot create CGImage");
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer::push_frame: created CGImage 0x%x", m_image);
	m_lock.leave();
}
    
CGLayerRef
cg_dsvideo_renderer::create_cglayer_from_cgimage(CGImageRef cgimage)
{
    // If we don't have an image we cannot do anything
    if (cgimage == NULL) return NULL;
 
    const common::region_info *ri = m_dest->get_info();
    const rect &r = m_dest->get_rect();
    AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.create_cglayer_from_cgimage(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
    rect size = lib::size(CGImageGetWidth(cgimage), CGImageGetHeight(cgimage));
        
    // Apply chroma keying.
    if (ri && ri->is_chromakey_specified()) {
        double opacity = ri->get_chromakeyopacity();
        if (opacity != 0.0 && opacity != 1.0) {
            lib::logger::get_logger()->trace("%s: only chromaKeyOpacity values 0.0 and 1.0 supported on MacOS", m_node->get_sig().c_str());
        }
        if (opacity < 0.5) {
            lib::color_t chromakey = ri->get_chromakey();
            lib::color_t tolerance = ri->get_chromakeytolerance();
            CGFloat components[8] = {
                redf(chromakey)-redf(tolerance), redf(chromakey)+redf(tolerance),
                greenf(chromakey)-greenf(tolerance), greenf(chromakey)+greenf(tolerance),
                bluef(chromakey)-bluef(tolerance), bluef(chromakey)+bluef(tolerance),
                0.0, 0.0
            };
            CGImageRef new_image = CGImageCreateWithMaskingColors(cgimage, components);
            if (new_image == NULL) {
                return NULL;
            }
            CGImageRelease(cgimage);
            cgimage = new_image;
        }
    }
    AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer._prepare_image: create cglayer");
        
    // Create the layer, initially with the same parameters as the current context.
    CGContextRef myContext = [AmbulantView currentCGContext];
    CGRect layer_rect = CGRectMake(0, 0, size.w, size.h);
    CGLayerRef cglayer = CGLayerCreateWithContext(myContext, layer_rect.size, NULL);
    assert(cglayer);
        
    // Draw the image in the layer
    myContext = CGLayerGetContext(cglayer);
    assert(myContext);
    CGContextDrawImage(myContext, layer_rect, cgimage);
    CFRelease (cgimage);
    return cglayer;
}

void
cg_dsvideo_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	_frame_was_displayed();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cg_window *cwindow = (cg_window *)window;
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
			AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cg_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif
	if (m_image) {
		// Now find both source and destination area for the bitblit.
		CGSize cg_srcsize = CGSizeMake(m_size.w, m_size.h);
		rect srcrect = rect(size(0, 0));
        rect croprect = m_dest->get_crop_rect(m_size);
		rect dstrect = m_dest->get_fit_rect(croprect, m_size, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());

		CGRect cg_dstrect = CGRectFromAmbulantRect(dstrect);
		CGRect cg_srcrect = CGRectFromAmbulantRect(srcrect);
		AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cg_srcsize.width, cg_srcsize.height, CGRectGetMinX(cg_dstrect), CGRectGetMinY(cg_dstrect), CGRectGetMaxX(cg_dstrect), CGRectGetMaxY(cg_dstrect));
        // get the 'fit' value to determine whether a cglayer is needed (can be avoide if pixles can be dropped as is).
        const common::region_info* ri = m_dest->get_info();
        const common::fit_t fit = ri->get_fit();
        bool need_cglayer = false;
        if (ri == NULL || ri->is_chromakey_specified()
            || (fit != common::fit_fill && (srcrect != dstrect))) {
                need_cglayer = true;
        }
//XXX	CGImageRef cropped_image = m_image; // No need to crop: the clipping does the trick.
        CGImageRef cropped_image = CGImageCreateWithImageInRect(m_image, cg_srcrect);
		CGContextRef myContext = [view getCGContext];
		double alfa = 1.0;
		if (ri) alfa = ri->get_mediaopacity();
		AM_DBG lib::logger::get_logger()->debug("0x%x: drawImage(0x%x, %f)", this, cropped_image, alfa);
		CGContextSaveGState(myContext);
        CGContextSetAlpha(myContext, (CGFloat)alfa);

		// We need to clip, also taking parent region clipping into account:
		rect clipRect = m_dest->get_clipped_screen_rect() & dstrect;
		CGRect cgClipRect = CGRectFromAmbulantRect(clipRect);
		CGContextClipToRect(myContext, cgClipRect);

        // We need to mirror the image, because CGImage uses bottom-left coordinates.
		CGAffineTransform matrix = [view transformForRect: &cg_dstrect flipped: YES translated: NO];
        CGContextConcatCTM(myContext, matrix);
        // scaling is done automatically by CGContextDrawLayerInRect/CGContextDrawImage
        if (need_cglayer) {
            CGLayerRef cglayer = create_cglayer_from_cgimage (cropped_image);
            if (cglayer != NULL) {
                CGContextDrawLayerInRect(myContext, cg_dstrect, cglayer);
                CFRelease (cglayer);                
            } else {
                lib::logger::get_logger()->trace("cg_dsvideo_renderer.redraw_body: could not create a CGLayer object for CGImage %p", cropped_image);
            }
        } else {
            CGContextDrawImage (myContext, cg_dstrect, cropped_image);
            CFRelease(cropped_image);
        }
        CGContextRestoreGState(myContext);
#ifdef LOGGER_VIDEOLATENCY
        logger::get_logger(LOGGER_VIDEOLATENCY)->trace("videolatency 7-display %lld %lld %s", 0LL, m_last_frame_timestamp, m_node->get_url("src").get_url().c_str());
#endif
	} else {
		AM_DBG lib::logger::get_logger()->debug("0x%x: cg_dsvideo.redraw: no image to show", this);
	}
#ifdef WITH_VIDEO_TRANSITION_UNTESTED
	if (surf) [surf unlockFocus];
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cg_dsvideo_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cg_dsvideo_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay);
	}
#endif

	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant

