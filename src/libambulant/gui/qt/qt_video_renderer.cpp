/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/gui/qt/qt_factory_impl.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"

#include <stdlib.h>
#include "ambulant/common/playable.h"
#include "ambulant/smil2/test_attrs.h"

// WARNING: turning on AM_DBG globally in this file seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

extern const char qt_video_playable_tag[] = "video";
extern const char qt_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererQt");
extern const char qt_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererVideo");
extern const char qt_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererOpen");

common::playable_factory *
gui::qt::create_qt_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererQt"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	return new common::single_playable_factory<
		qt_video_renderer,
		qt_video_playable_tag,
		qt_video_playable_renderer_uri,
		qt_video_playable_renderer_uri2,
		qt_video_playable_renderer_uri3>(factory, mdp);
}

qt_video_renderer::qt_video_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	qt_renderer<common::video_renderer>(context, cookie, node, evp, factory, mdp),
	m_image(NULL),
	m_datasize(1)
{
	m_data =(uchar*) malloc(1);
	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer::qt_video_renderer(0x%x): context=0x%x, cookie=%d, node=0x%x evp=0x%x factory=0x%x", (void*) this, (void*)context, cookie, node, evp, factory);
}

qt_video_renderer::~qt_video_renderer()
{
	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer::~qt_video_renderer(0x%x) m_data=0x%x m_image=0x%x", (void*) this, (void*)m_data);
	m_lock.enter();
	if (m_data) free(m_data);
	if ( m_image) delete m_image;
	m_lock.leave();
}

void
qt_video_renderer::_push_frame(char* frame, size_t size)
{
	assert(frame);
	assert(size == (m_size.w*m_size.h*4));

	if (m_image) delete m_image;
	if (m_data) free(m_data);
	m_data = (uchar*)frame;
	m_image = new QImage(m_data,  m_size.w, m_size.h, 32, NULL, 0, QImage::IgnoreEndian);

	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer::_push_frame(0x%x): frame=0x%x, size=%d, m_image=0x%x", (void*) this, (void*)frame, (int)size, m_image);

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
		lib::rect croprect = m_dest->get_crop_rect(srcsize);
		lib::rect dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
#if ONLY_FOR_QT4
		QRect q_srcrect(srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
		QRect q_dstrect(dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
		paint.drawImage(q_dstrect, *m_image, q_srcrect);
#else
		AM_DBG lib::logger::get_logger()->debug("qt_video: %d,%d,%d,%d to %d,%d,%d,%d",
			srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height(),
			dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
		if (srcrect.size() == dstrect.size()) {
			paint.drawImage(dstrect.left(), dstrect.top(), *m_image,
				srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
		} else {
			lib::logger::get_logger()->debug("qt_video_renderer: correcting cutout");
			// Grmpf. We have to scale or transform.
			// First cut out the right portion of the source image
			QImage cutout(srcrect.width(), srcrect.height(), m_image->depth());
			cutout.fill(0);
			bitBlt(&cutout, 0, 0, m_image, srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
			// Next, we scale it.
			QImage scaled = cutout.smoothScale(dstrect.width(), dstrect.height(), QImage::ScaleFree);
			// Finally we render it
			paint.drawImage(dstrect.left(), dstrect.top(), scaled, 0, 0, dstrect.width(), dstrect.height());
		}
#endif
	} else {
		AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw(0x%x): no m_image", (void *) this);
	}
	paint.flush();
	paint.end();
	m_lock.leave();
}
