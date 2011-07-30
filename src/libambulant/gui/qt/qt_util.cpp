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

#include "ambulant/gui/qt/qt_util.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace qt {
//---------------------------------------------------------------------------
// Utility functions for internal use
//

// blending

inline unsigned int
_blend_pixel (
	int dr, int dg, int db, int da,
	int sr, int sg, int sb, int sa,
	bool alpha, unsigned int weight, unsigned int remain)
{
	unsigned int result = 0;
	if (alpha)
		result = qRgba(
			(dr*remain + sr*weight) >> 8,
			(dg*remain + sg*weight) >> 8,
			(db*remain + sb*weight) >> 8,
			(da*remain + sa*weight) >> 8);
	else
		result = qRgb(
			(dr*remain + sr*weight) >> 8,
			(dg*remain + sg*weight) >> 8,
			(db*remain + sb*weight) >> 8);
	return result;
}

void
qt_image_blend (
	QImage dst,
	const lib::rect dst_rc,
	QImage src,
	const lib::rect src_rc,
	double opacity_in,
	double opacity_out,
	const lib::color_t chroma_low,
	const lib::color_t chroma_high)
{
  //TBD: dst(L,T,W,H) != src(L,T,W,H), alpha channel in dst/src
	assert (dst != NULL && src != NULL);
	bool dst_has_alpha = dst.hasAlphaBuffer();
	bool src_has_alpha = src.hasAlphaBuffer();
	assert (dst_has_alpha == src_has_alpha);

	/* compute start/stop col/row */
	signed int dst_L = dst_rc.x, dst_T = dst_rc.y;
	unsigned int dst_W = dst_rc.w, dst_H = dst_rc.h;
	signed int src_L = src_rc.x, src_T = src_rc.y;
	unsigned int src_W = src_rc.w, src_H = src_rc.h;
	unsigned int dst_col, dst_row, src_col, src_row;
	unsigned int W = dst_W <= src_W ? dst_W : src_W;
	unsigned int H = dst_H <= src_H ? dst_H : src_H;
	unsigned int max_R = dst.width();
	unsigned int max_B = dst.height();
	unsigned int dst_R = dst_L + W;
	unsigned int dst_B = dst_T + H;
	/* ensure stop col/row within image*/
	if (dst_R > dst_L+dst_W)
		dst_R = dst_L+dst_W;
	if (dst_R > max_R)
		dst_R = max_R;
	if (dst_B > dst_T+dst_H)
		dst_B = dst_T+dst_H;
	if (dst_B > max_B)
		dst_B = max_B;


	unsigned int weight_in = static_cast<unsigned int>(round(opacity_in*255.0));
	unsigned int remain_in = 255 - weight_in;
	unsigned int weight_out = static_cast<unsigned int>(round(opacity_out*255.0));
	unsigned int remain_out = 255 - weight_out;

	uchar r_l = redc(chroma_low), r_h = redc(chroma_high);
	uchar g_l = greenc(chroma_low), g_h = greenc(chroma_high);
	uchar b_l = bluec(chroma_low), b_h = bluec(chroma_high);
	AM_DBG logger::get_logger()->debug("blend_qt_pixbuf:r_l=%3d,g_l=%3d,b_l=%3d,w_in=%d,w_out=%d", r_l,g_l,b_l,weight_in, weight_out);
	AM_DBG logger::get_logger()->debug("blend_qt_pixbuf:r_h=%3d,g_h=%3d,b_h=%3d", r_h,g_h,b_h);
	AM_DBG logger::get_logger()->debug("blend_qt_pixbuf:dst_L=%3d,dst_R=%3d,max_R=%3d,src_L=%3d", dst_L, dst_R, max_R, src_L);
	for (dst_col = dst_L, src_col = src_L;
		dst_col < dst_R;
		dst_col++, src_col++) {
		AM_DBG logger::get_logger()->debug("blend_qt_pixbuf:dst_T=%3d,dst_B=%3d,max_B=%3d,src_T=%3d", dst_T, dst_B, max_B,src_T);
		for (dst_row = dst_T, src_row = src_T; dst_row < dst_B; dst_row++, src_row++) {
			QRgb src_pixel = src.pixel(src_col,src_row);
			QRgb dst_pixel = dst.pixel(dst_col,dst_row);
			int dr = qRed(dst_pixel);
			int dg = qGreen(dst_pixel);
			int db = qBlue(dst_pixel);
			int da = src_has_alpha ? qAlpha(dst_pixel) : 0xff;
			int sr = qRed(src_pixel);
			int sg = qGreen(src_pixel);
			int sb = qBlue(src_pixel);
			int sa = src_has_alpha ? qAlpha(src_pixel) : 0xff;

			if ( // check all components in chromakey range
				r_l <= sr && sr <= r_h
				&&	g_l <= sg && sg <= g_h
				&&	b_l <= sb && sb <= b_h
			) {
				// blend the pixel from 'src' into 'dst'
//logger::get_logger()->debug("dst[%d,%d]=0x%x%x%x, src[%d,%d]=0x%x%x%x, blend=0x%x", dst_col,dst_row,dr,dg,db,src_col,src_row,sr,sg,sb, _blend_pixel(dr,dg,db,da,sr,sg,sb,sa,src_has_alpha, weight, remain));
				dst.setPixel(dst_col,dst_row,
					_blend_pixel(dr,dg,db,da,
						sr,sg,sb,sa,
						src_has_alpha,
						weight_in, remain_in));
			} else {
				// blend the pixel from 'src' into 'dst'
				dst.setPixel(dst_col,dst_row,
					_blend_pixel(dr,dg,db,da,
						sr,sg,sb,sa,
						src_has_alpha,
						weight_out, remain_out));
			}
		}
	}
}

color_t
QColor2color_t(QColor c) {
	return to_color(c.red(), c.green(), c.blue());
}

QColor
color_t2QColor(lib::color_t c) {
	return QColor(redc(c), greenc(c), bluec(c));
}

#ifdef	WITH_DUMPIMAGES
static QImage* oldImageP;
static bool
isEqualToPrevious(QImage* img) {
	if (oldImageP != NULL && *img == *oldImageP) {
		AM_DBG lib::logger::get_logger()->debug("isEqualToPrevious: new image not different from old one");
		return true;
	} else {
		if (oldImageP != NULL) delete oldImageP;
		oldImageP = new QImage(*img);
		return false;
	}
}

int
qt_image_dump(QImage* img, std::string id) {
	if ( ! img) return -1;
	if ( ! isEqualToPrevious(img)) {
		static int i;
		char buf[5];
		sprintf(buf,"%04d",i++);
		if (i == 10000) i = 0;
		std::string newfile = buf + std::string(id) +".png";
		img->save(newfile, "PNG");
		AM_DBG lib::logger::get_logger()->debug("qt_image_dump(%s)", newfile.c_str());
		return i == 0 ? 9999 : i - 1;
	}
	return -1;
}

int
qt_pixmap_dump(QPixmap* qpm, std::string id) {
	if ( ! qpm) return -1;
	QImage img = qpm->convertToImage();
	return qt_image_dump(&img, id);
}
#endif /* WITH_DUMPIMAGES */

} // namespace qt

} // namespace gui

} //namespace ambulant
