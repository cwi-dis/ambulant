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

#include "ambulant/gui/gtk/gtk_util.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace gtk {

inline guchar _blend_pixel (uchar c1, uchar c2, uchar weight)
	{
		return (c1==c2)?c1:(c1 + weight*(c2-c1)/256);
	}
void
gdk_pixbuf_blend (
	GdkPixbuf* dst,
	const lib::rect dst_rc,
	GdkPixbuf* src,
	const lib::rect src_rc,
	double opacity_in,
	double opacity_out,
	const lib::color_t chroma_low,
	const lib::color_t chroma_high,
	const lib::color_t mask_color)
{
  //TBD: dst(L,T,W,H) != src(L,T,W,H), alpha channel in dst/src
	assert(dst != NULL && src != NULL);
	guint	n_channels_dst = gdk_pixbuf_get_n_channels (dst);
	guint	n_channels_src = gdk_pixbuf_get_n_channels (src);
	assert (n_channels_dst >= 3 && n_channels_dst <= 4);
//	assert (n_channels_dst == n_channels_src);

	gint	dst_L = dst_rc.x, dst_T = dst_rc.y;
	guint	dst_W = dst_rc.w, dst_H = dst_rc.h;
	gint	src_L = src_rc.x, src_T = src_rc.y;
	guint	src_W = src_rc.w, src_H = src_rc.h;
	guchar* dst_col, * dst_row, * src_col, * src_row;
	gint	col, row;
	gint	L = dst_L <= src_L ? dst_L : src_L;
	gint	T = dst_T <= src_T ? dst_T : src_T;
	guint	W = dst_W <= src_W ? dst_W : src_W;
	guint	H = dst_H <= src_H ? dst_H : src_H;
	guint	R = L + W;
	guint	B = T + H;

	guint weight_in = static_cast<guint>(round(opacity_in*255.0));
	guint weight_out = static_cast<guint>(round(opacity_out*255.0));

	guchar r_l = redc(chroma_low), r_h = redc(chroma_high), r_m = redc(mask_color);
	guchar g_l = greenc(chroma_low), g_h = greenc(chroma_high), g_m = greenc(mask_color);
	guchar b_l = bluec(chroma_low), b_h = bluec(chroma_high), b_m = bluec(mask_color);
	AM_DBG logger::get_logger()->debug("blend_gdk_pixbuf:r_l=%3d,g_l=%3d,b_l=%3d,w_in=%d,w_out=%d", r_l,g_l,b_l,weight_in, weight_out);
AM_DBG logger::get_logger()->debug("blend_gdk_pixbuf:r_h=%3d,g_h=%3d,b_h=%3d", r_h,g_h,b_h);

	dst_row = gdk_pixbuf_get_pixels (dst);
	src_row = gdk_pixbuf_get_pixels (src);

	for (row = T; row < B; row++) {
		dst_col = dst_row;
		src_col = src_row;
		for (col = L; col < R; col++) {
			guchar r = src_col[0];
			guchar g = src_col[1];
			guchar b = src_col[2];
			if ( ! (mask_color
				&& r == r_m && g == g_m && b == b_m)) {
				guchar a = n_channels_src == 4 ? src_row[3] : 0xff;
//AM_DBG logger::get_logger()->debug("blend_gdk_pixbuf:rc=(%3d,%3d),dst=(%3d,%3d,%3d),src=(%3d,%3d,%3d)",row,col,dst_col[0],dst_col[1],dst_col[2],r,g,b);
			// chromakeying
				if ( // check all components in chromakey range
					r_l <= r && r <= r_h
					&&	g_l <= g && g <= g_h
					&&	b_l <= b && b <= b_h
				) {
					// blend the pixel from 'src' into 'dst'
					dst_col[0] = _blend_pixel(dst_col[0], r, weight_in);
					dst_col[1] = _blend_pixel(dst_col[1], g, weight_in);
					dst_col[2] = _blend_pixel(dst_col[2], b, weight_in);
					if (n_channels_dst == 4)
						dst_col[3] = _blend_pixel(dst_col[3], a, weight_in);
				} else {
					// blend the pixel from 'src' into 'dst'
					dst_col[0] = _blend_pixel(dst_col[0], r, weight_out);
					dst_col[1] = _blend_pixel(dst_col[1], g, weight_out);
					dst_col[2] = _blend_pixel(dst_col[2], b, weight_out);
					if (n_channels_dst == 4)
						dst_col[3] = _blend_pixel(dst_col[3], a, weight_out);
				}
			}
			dst_col += n_channels_dst;
			src_col += n_channels_src;
		}
		dst_row += gdk_pixbuf_get_rowstride(dst);
		src_row += gdk_pixbuf_get_rowstride(src);
	}

}

#ifdef	WITH_DUMPIMAGES
int
#if GTK_MAJOR_VERSION >= 3
cairo_surface_dump(cairo_surface_t* srf, std::string filename) {
	if (srf == NULL) return -1;
	guint W = cairo_image_surface_get_width (srf);
	guint H = cairo_image_surface_get_height (srf);
	if (W == 0 || H == 0) {
		return -1;
	}
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface (srf, 0, 0, W, H);
	int i = gdk_pixbuf_dump(pixbuf, filename);
	g_object_unref(G_OBJECT(pixbuf));
	return i;
}
#else // GTK_MAJOR_VERSION < 3
gdk_pixmap_dump(GdkPixmap* gpm, std::string filename) {
	if ( ! gpm) return -1;
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(NULL, gpm, 0, 0, 0, 0, 0, -1, -1);
	int i = gdk_pixbuf_dump(pixbuf, filename);
	g_object_unref(G_OBJECT(pixbuf));
	return i;
}
#endif // GTK_MAJOR_VERSION < 3

int
gdk_pixbuf_dump(GdkPixbuf* gpb, std::string filename) {
	if (gpb) {
		char buf[5];
		static int i;
		sprintf(buf,"%%%04d",i++);
		if (i == 10000) i = 0;
		std::string newfile = buf + std::string(filename) +".png";
		GError* error = NULL;
		gdk_pixbuf_save(gpb, newfile.c_str(), "png", &error, NULL);
		AM_DBG lib::logger::get_logger()->debug("gdk_pixmap_dump(%s)", newfile.c_str());
		return i == 0 ? 9999 : i-1;
	}
	return -1;
}
#endif//WITH_DUMPIMAGES

} // namespace gtk

} // namespace gui

} //namespace ambulant
