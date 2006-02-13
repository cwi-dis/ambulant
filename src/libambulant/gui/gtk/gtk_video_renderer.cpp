/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_video_renderer.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"

#define AM_DBG
//#ifndef AM_DBG
//#define AM_DBG if(0)
//#endif

using namespace ambulant;
using namespace gui::gtk;



gtk_video_renderer::gtk_video_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
    	common::factories *factory)
:	 common::video_renderer(context, cookie, node, evp, factory),
 	m_image(NULL),
  	m_data(NULL),
	m_img_displayed(0)
{
#if 0
    //This is ridiculous! how could something be in m_frames?
	while ( m_frames.size() > 0 ) {
		std::pair<int, char*> element = m_frames.front();
		//free(element.second);
		m_frames.pop();
	}
#endif
 //   assert(m_frames.size() == 0);
	
}

gtk_video_renderer::~gtk_video_renderer()
{
	if (m_data)
		free(m_data);
	if(m_image)
		g_object_unref (G_OBJECT (m_image));
}

void 
gtk_video_renderer::show_frame(const char* frame, int size)
{

	m_lock.enter();
	
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.show_frame: frame=0x%x, size=%d, this=0x%x", (void*) frame, size, (void*) this);
 
	if (m_data)
		free(m_data);	
	
    	m_data = (char*) malloc(size);

	// This is needed to convert the colors to GTK+
	for(int i=0;i < size;i=i+4){
		m_data[i] = frame[i+2];		/*R Red*/
		m_data[i+1] = frame[i+1];	/*G GREEN*/
		m_data[i+2] = frame[i];		/*B BLUE*/
		m_data[i+3] = frame[i+3];	/*  A ALPHA*/		    
	}
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.show_frame: About to calll need_redraw, (m_dest=0x%x)", (void*) m_dest);
	m_dest->need_redraw();	
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.show_frame: need_redraw called");

	m_lock.leave();
}


void
gtk_video_renderer::redraw(const lib::rect &dirty, common::gui_window* w) 
{
	if (m_data){
		//m_lock.enter();
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw(0x%x)",(void*) this);

		const lib::point p = m_dest->get_global_topleft();
		const lib::rect &r = m_dest->get_rect();
	
		// XXXX WRONG! This is the info for the region, not for the node!
		const common::region_info *info = m_dest->get_info();
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw: info=0x%x", info);

		ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

		// background drawing
		if (info && !info->get_transparent()) {
		// First find our whole area (which we have to clear to 
		// background color)
			lib::rect dstrect_whole = r;
			dstrect_whole.translate(m_dest->get_global_topleft());
			int L = dstrect_whole.left(),
		    T = dstrect_whole.top(),
		    W = dstrect_whole.width(),
		    H = dstrect_whole.height();
		// XXXX Fill with background color
			lib::color_t bgcolor = info->get_bgcolor();
			AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw: clearing to 0x%x", (long)bgcolor);
			GdkColor bgc;
			bgc.red = redc(bgcolor)*0x101;
			bgc.blue = bluec(bgcolor)*0x101;
			bgc.green = greenc(bgcolor)*0x101;
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_gc_set_rgb_fg_color (gc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
			g_object_unref (G_OBJECT (gc));
		}

		int width = m_size.w;
		int height = m_size.h;
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.show_frame(0x%x): width = %d, height = %d",(void *)this, width, height);
		m_image =  gdk_pixbuf_new_from_data ((const guchar*) m_data, GDK_COLORSPACE_RGB, TRUE, 8, width, height, (width*4), NULL, NULL);

		if (m_image) {
			int width = gdk_pixbuf_get_width(m_image);
			int height = gdk_pixbuf_get_height(m_image);
			size srcsize = size(width, height);
			lib::rect srcrect = lib::rect(lib::size(0,0));
			lib::rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
			dstrect.translate(m_dest->get_global_topleft());
			int L = dstrect.left(), 
		    	T = dstrect.top(),
		    	W = dstrect.width(),
		    	H = dstrect.height();
			AM_DBG lib::logger::get_logger()->debug(" gtk_video_renderer.redraw(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d)", (void *)this,L,T,W,H);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_pixbuf_render_to_drawable(m_image, GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE, 0, 0);
			g_object_unref (G_OBJECT (gc));			
		}
	//m_lock.leave();
	}
}
