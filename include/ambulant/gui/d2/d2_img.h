/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_GUI_D2_IMG_H
#define AMBULANT_GUI_D2_IMG_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/d2/d2_renderer.h"

interface IWICImagingFactory;
interface IWICBitmapSource;
interface ID2D1Bitmap;

namespace ambulant {

namespace gui {

namespace d2 {

common::playable_factory *create_d2_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class d2_img_renderer : public d2_renderer<renderer_playable> {
  public:
	d2_img_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	~d2_img_renderer();
	void start(double t);
	bool stop();
	void seek(double t) {}
	bool user_event(const lib::point& pt, int what);
	void redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget*);

	void recreate_d2d();
	void discard_d2d();

	static void initwic();
  private:

	static IWICImagingFactory *s_wic_factory;
	IWICBitmapSource *m_original;	// The original image data reader
	ID2D1Bitmap *m_d2bitmap;		// The bitmap in Direct2D form
	char *m_databuf;				// Buffered image data, unless the image comes from local file.
	common::factories *m_factory;
};

} // namespace d2

} // namespace gui

} // namespace ambulant
#endif // AMBULANT_GUI_D2_IMG_H
