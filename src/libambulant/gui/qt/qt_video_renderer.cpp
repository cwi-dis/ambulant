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

#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"
//#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;



qt_video_renderer::qt_video_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
    	common::factories *factory)
:	qt_renderer<common::video_renderer>(context, cookie, node, evp, factory),
	m_image(NULL),
	m_datasize(1)
{
	m_data =(uchar*) malloc(1);
}

qt_video_renderer::~qt_video_renderer()
{
	m_lock.enter();
    if (m_data) free(m_data);
	if ( m_image) delete m_image;
	m_lock.leave();
}

void 
qt_video_renderer::show_frame(const char* frame, int size)
{
	m_lock.enter();
	assert(frame);
	assert(size == (int)(m_size.w*m_size.h*4));

    // First copy the data (XXXX Not needed, to be removed later)
	// NOTE: data given to QImage should remain valid during the life of the image
	if (size > m_datasize) {
		m_data = (uchar*) realloc((void*) m_data, size);
		if (!m_data) {
        lib::logger::get_logger()->trace("qt_video_renderer.show_frame: out of memory");
        return;
		}
		m_datasize = size;
	}
    memcpy(m_data, frame, size);
    
    if (m_image) delete m_image;
    m_image = new QImage(m_data,  m_size.w, m_size.h, 32, NULL, 0, QImage::IgnoreEndian);

	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame(0x%x): frame=0x%x, size=%d, m_image=0x%x", (void*) this, (void*)frame, size, m_image);
    assert(m_dest);
	m_dest->need_redraw();	

	m_lock.leave();
}



void
qt_video_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w) 
{
    AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw(0x%x)",(void*) this);

    ambulant_qt_window* aqw = (ambulant_qt_window*) w;
    QPainter paint;
	//XXXX locking at this point may result in deadly embrace with internal lock,
	//XXXX but as far as we know this has never happened
	m_lock.enter();
    paint.begin(aqw->get_ambulant_pixmap());

    if ( m_image ) {
        lib::size srcsize = lib::size(m_size.w, m_size.h);
        lib::rect srcrect;
        lib::rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
        dstrect.translate(m_dest->get_global_topleft());
        int L = dstrect.left(), 
            T = dstrect.top(),
            W = dstrect.width(),
            H = dstrect.height();
        AM_DBG lib::logger::get_logger()->debug(" qt_video_renderer.redraw(0x%x): drawImage 0x%x at (L=%d,T=%d,W=%d,H=%d)", (void *)this,m_image,L,T,W,H);
        // XXX This is wrong: it does not take srcrect into account, and hence it
        // does not scale the video.
        paint.drawImage(L,T,*m_image,0,0,W,H);
    } else {
        AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw(0x%x): no m_image", (void *) this);
    }
    paint.flush();
    paint.end();
	m_lock.leave();
}
