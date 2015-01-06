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
#include "ambulant/gui/cg/cg_image.h"
#include "ambulant/gui/cg/cg_transition.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_image_playable_tag[] = "img";
extern const char cg_image_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
extern const char cg_image_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererImg");

common::playable_factory *
create_cg_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		cg_image_renderer,
		cg_image_playable_tag,
		cg_image_playable_renderer_uri,
		cg_image_playable_renderer_uri2,
		cg_image_playable_renderer_uri2>(factory, mdp);
}

cg_image_renderer::~cg_image_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_image_renderer(0x%x)", (void *)this);
	if (m_image != NULL) {
		CGImageRelease(m_image);
		m_image = NULL;
	}
	if (m_nsdata != NULL) {
		m_nsdata = NULL;
	}
	if (m_cglayer != NULL) {
		CGLayerRelease(m_cglayer);
		m_cglayer = NULL;
	}
	m_lock.leave();
}

bool
cg_image_renderer::_prepare_image()
{
	// If we already have a cglayer we have done everything we can.
	if (m_cglayer) return true;

	// If we have no data yet we cannot do anything
	if (!m_data) return false;
	
	const common::region_info *ri = m_dest->get_info();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_image_renderer._prepare_image(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	//
	// First we load the image data into m_image, if not done previously.
	//
	if (!m_image) {
		AM_DBG logger::get_logger()->debug("cg_image_renderer._prepare_image: creating image");
		if (m_nsdata != NULL) {
			CFRelease(m_nsdata);
			m_nsdata = NULL;
		}

		// Get the data as an image source
		m_nsdata = (CFDataRef)[NSData dataWithBytesNoCopy: m_data length: (unsigned int)m_data_size freeWhenDone: NO];
		CGImageSourceRef rdr = CGImageSourceCreateWithData(m_nsdata, NULL);
		if (rdr == NULL) {
			logger::get_logger()->error("%s: could not create image reader", m_node->get_url("src").get_url().c_str());
			return false;
		}

		// Get the first image from the source
		m_image = CGImageSourceCreateImageAtIndex(rdr, 0, NULL);
		CFRelease(rdr);
		if (!m_image) {
			logger::get_logger()->error("%s: could not create CGImage", m_node->get_url("src").get_url().c_str());
			return false;
		}
		m_size = lib::size(CGImageGetWidth(m_image), CGImageGetHeight(m_image));

		// Apply chroma keying.
		// XXXJACK: by doing this here we disregard animation on chromaKeying
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
				CGImageRef new_image = CGImageCreateWithMaskingColors(m_image, components);
				CGImageRelease(m_image);
				m_image = new_image;
			}
		}
	}
	assert(m_image);

    AM_DBG lib::logger::get_logger()->debug("cg_image_renderer._prepare_image: create cglayer");
    
    // Create the layer, initially with the same parameters as the current context.
#ifdef JNK
#ifdef WITH_UIKIT
    CGContextRef myContext = UIGraphicsGetCurrentContext();
#else
    CGContextRef myContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
#else// ! JNK
	CGContextRef myContext = [AmbulantView currentCGContext];
#endif// ! JNK
    CGRect layer_rect = CGRectMake(0, 0, m_size.w, m_size.h);
    m_cglayer = CGLayerCreateWithContext(myContext, layer_rect.size, NULL);
    assert(m_cglayer);

    // Draw the image in the layer
    myContext = CGLayerGetContext(m_cglayer);
    assert(myContext);
    CGContextDrawImage(myContext, layer_rect, m_image);
    
    return true;
}

void
cg_image_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	if (!_prepare_image()) {
		m_lock.leave();
		return;
	}
	rect srcrect;
	rect dstrect;
	CGRect cg_dstrect;
	lib::point dest_origin = m_dest->get_global_topleft();
 	cg_window *cwindow = (cg_window *)window;
 	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGContextRef myContext = [view getCGContext];

	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		assert(m_cglayer);
		AM_DBG lib::logger::get_logger()->debug("cg_image_renderer.redraw: drawing tiled image");
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(m_size, dstrect);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			srcrect = (*it).first;
			dstrect = (*it).second;
			cg_dstrect = CGRectFromAmbulantRect(dstrect);

			CGContextSaveGState(myContext);
			// Draw the pre-rendered image in cglayer. First setup the destination parameters.
			CGContextClipToRect(myContext, cg_dstrect);
			// First setup the matrix so that drawing at point (0,0) will appear at the topleft of the destination
			// rectangle.
			CGAffineTransform matrix = [view transformForRect: &cg_dstrect flipped: YES translated: YES];
			CGContextConcatCTM(myContext, matrix);
			
			// Now setup the source parameters. Start with scale.
			float x_scale = (float)dstrect.width() / (float)srcrect.width();
			float y_scale = (float)dstrect.height() / (float)srcrect.height();
			matrix = CGAffineTransformMake(x_scale, 0, 0, y_scale, 0, 0);
			CGContextConcatCTM(myContext, matrix);
			
			// Next we do offset. This is a bit tricky, as our srcrect uses topleft-based coordinates and
			// CG uses cartesian.
			lib::rect fullsrcrect = lib::rect(lib::point(0, 0), m_size);  // Original image size
			fullsrcrect.translate(lib::point(-srcrect.left(), srcrect.bottom()-m_size.h)); // Translate so the right topleft pixel is in place
			CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
			AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw: draw layer to (%f, %f, %f, %f) clip (%f, %f, %f, %f) scale (%f, %f)",
				CGRectGetMinX(cg_fullsrcrect), CGRectGetMinY(cg_fullsrcrect), CGRectGetMaxX(cg_fullsrcrect), CGRectGetMaxY(cg_fullsrcrect),
				CGRectGetMinX(cg_dstrect), CGRectGetMinY(cg_dstrect), CGRectGetMaxX(cg_dstrect), CGRectGetMaxY(cg_dstrect),
				x_scale, y_scale);

			CGContextDrawLayerInRect(myContext, cg_fullsrcrect, m_cglayer);
			CGContextRestoreGState(myContext);
		}
		m_lock.leave();
		return;
	}
	//
	// Determine drawing parameters
	//
	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	//
	// Setup drawing parameters and draw
	//
    CGContextSaveGState(myContext);
	
	if (m_cglayer) {
		// Draw the pre-rendered image in cglayer. First setup the destination parameters.
		lib::rect croprect = m_dest->get_crop_rect(m_size);
		AM_DBG logger::get_logger()->debug("cg_image::redraw(0x%x) via cglayer, clip 0x%x (%d %d) -> (%d, %d, %d, %d) alpha %f", this, m_dest, m_size.w, m_size.h, croprect.x, croprect.y, croprect.w, croprect.h, alfa);
		
		dstrect = m_dest->get_fit_rect(croprect, m_size, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		cg_dstrect = CGRectFromAmbulantRect(dstrect);
		CGContextClipToRect(myContext, cg_dstrect);
		// First setup the matrix so that drawing at point (0,0) will appear at the topleft of the destination
		// rectangle.
		CGAffineTransform matrix = [view transformForRect: &cg_dstrect flipped: YES translated: YES];
		CGContextConcatCTM(myContext, matrix);
		
		// Now setup the source parameters. Start with scale.
		float x_scale = (float)dstrect.width() / (float)srcrect.width();
		float y_scale = (float)dstrect.height() / (float)srcrect.height();
		matrix = CGAffineTransformMake(x_scale, 0, 0, y_scale, 0, 0);
		CGContextConcatCTM(myContext, matrix);
		
		// Next we do offset. This is a bit tricky, as our srcrect uses topleft-based coordinates and
		// CG uses cartesian.
		lib::rect fullsrcrect = lib::rect(lib::point(0, 0), m_size);  // Original image size
		fullsrcrect.translate(lib::point(-srcrect.left(), srcrect.bottom()-m_size.h)); // Translate so the right topleft pixel is in place
		CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
		AM_DBG logger::get_logger()->debug("cg_image_renderer.redraw(0x%x, %s): draw layer to (%f, %f, %f, %f) clip (%f, %f, %f, %f) scale (%f, %f) alpha %f",
			this, m_node->get_sig().c_str(), CGRectGetMinX(cg_fullsrcrect), CGRectGetMinY(cg_fullsrcrect), CGRectGetMaxX(cg_fullsrcrect), CGRectGetMaxY(cg_fullsrcrect),
			CGRectGetMinX(cg_dstrect), CGRectGetMinY(cg_dstrect), CGRectGetMaxX(cg_dstrect), CGRectGetMaxY(cg_dstrect),
			x_scale, y_scale, alfa);

        if (alfa != 1.0) {
            CGContextSetAlpha(myContext, alfa);
        }
		CGContextDrawLayerInRect(myContext, cg_fullsrcrect, m_cglayer);
	} else {
		// No prerendering done so we render direct. This should only
		// happen if the pixels can be deposited as-is.
 		AM_DBG logger::get_logger()->debug("cg_image::redraw(0x%x) direct, alpha %f", this, m_dest, alfa);
       CGImage *imageToDraw = m_image;
		assert(dstrect.size() == srcrect.size());
		CGContextClipToRect(myContext, cg_dstrect); // XXXJACK DEBUG
        // We need to mirror the image, because CGImage uses bottom-left coordinates.
		CGAffineTransform matrix = [view transformForRect: &cg_dstrect flipped: YES translated: NO];
        CGContextConcatCTM(myContext, matrix);
		matrix = CGContextGetCTM(myContext);
        if (alfa != 1.0) {
            CGContextSetAlpha(myContext, alfa);
        }
		CGContextDrawImage(myContext, cg_dstrect, imageToDraw);
	}

    CGContextRestoreGState(myContext);
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant

