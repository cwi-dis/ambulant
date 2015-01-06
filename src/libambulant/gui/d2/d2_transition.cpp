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

// Implementation notes for d2 transitions:
// ----------------------------------------
// All out transitions (except fade and push/slide) work via polylist drawn into a
// clipping layer, witin a rectangle for the whole area in reverse direction.
// This is probably not optimal, as d2dbg warns for this.
// Fullscreen in transitions work by taking a fullscreen snapshot at first redraw
// in d2player after the transition is started. This is the disappearing image.
// In addition, for fullscreen out transitions a snapshot of the fullscreen is saved
// (in d2player::m_fullscreen_orig_bitmap) at the moment just before the drawable that eventually
// will disappeaer in full screen transition is first drawn.
// The fullscreen_orig_bitmap is then later used to transition into.
// To achieve this, all drawables are checked at first redraw whether they will eventually
// disappear in fullscreen mode d2_transition_renderer::check_fullscreen_outtrans(lib::node).

#include "ambulant/gui/d2/d2_transition.h"
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/lib/logger.h"

#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace common;

namespace gui {

namespace d2 {

inline D2D1_RECT_F
d2_rectf(lib::rect r)
{
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

inline D2D1_RECT_U
d2_rectu(lib::rect r)
{
	return D2D1::RectU((UINT32) r.left(), (UINT32) r.top(), (UINT32) r.right(), (UINT32) r.bottom());
}

inline D2D1_SIZE_F
d2_sizef(lib::rect r)
{
	return D2D1::SizeF((float) r.width(), (float) r.height());
}

inline D2D1_SIZE_U
d2_sizeu(lib::rect r)
{
	return D2D1::SizeU((UINT32) r.width(), (UINT32) r.height());
}

// Helper function: translate a rectangle pointed to by `rp' to the top-left point of the surface `dst'
// and clip the rectangle with the clipped screen rectangle of the surface
lib::rect
translate_and_clip_rect (lib::rect r, common::surface* dst)
{
	if ( ! r.empty() && dst != NULL) {
		r.translate(dst->get_global_topleft());
		r &= dst->get_clipped_screen_rect();
	}
	return r;
}

// Helper function: add a counter clockwise defined rectangle to the path of a ID2D1GeometrySink
// This is used for out transitions to reverse the effect of the clockwise defined clipping paths
// by enclosing them in a counter clockwise defined rectangle for the whole region using the
// non-zero winding rule
void
add_counter_clockwise_rect(ID2D1GeometrySink* sink, D2D1_RECT_U rect)
{	
	UINT32 minX = rect.left, maxX = rect.right, minY = rect.bottom, maxY = rect.top;
	sink->BeginFigure(D2D1::Point2F((float) minX, (float) minY), D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine( D2D1::Point2F((float) maxX, (float) minY));
	sink->AddLine( D2D1::Point2F((float) maxX, (float) maxY));
	sink->AddLine( D2D1::Point2F((float) minX, (float) maxY));
	sink->AddLine( D2D1::Point2F((float) minX, (float) minX));
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
}

// Helper function: convert a lib::rect into a polygon (std::vector of lib::points)
std::vector<lib::point>
polygon_from_rect(lib::rect rect)
{
	lib::point right_top = lib::point(rect.right(), rect.top());
	lib::point left_bottom = lib::point(rect.left(), rect.bottom());
	std::vector <lib::point> rv = std::vector <lib::point>();
	rv.push_back(rect.left_top());
	rv.push_back(right_top);
	rv.push_back(rect.right_bottom());
	rv.push_back(left_bottom);
	return rv;
}

// Helper function: convert a std::list of lib::rect into a polyon list (std::list of std::vector of lib::points)
std::vector<std::vector<lib::point>>
polygon_list_from_rect_list(std::vector<lib::rect>* rect_list)
{
	std::vector<std::vector<lib::point>> rv =  std::vector<std::vector<lib::point>>();
	for (std::vector<lib::rect>::iterator it = rect_list->begin(); it != rect_list->end(); it++) {
		rv.push_back(polygon_from_rect(*it));
	}
	return rv;
}

// Helper function: add clipping path from the list of polygons
ID2D1PathGeometry*
path_from_polygon_list(
	ID2D1Factory* factory,
	const lib::point& origin,
	std::vector< std::vector<lib::point> > polygon_list,
	bool outtrans,
	lib::rect whole_rect)
{
	ID2D1PathGeometry* path = NULL;
	ID2D1GeometrySink* sink = NULL;
	D2D1_POINT_2F new_d2_point_2f = D2D1::Point2F();
	lib::point old_point = lib::point();
	std::vector<lib::point>::iterator newpoint_p;

	HRESULT hr = factory->CreatePathGeometry(&path);
	OnErrorGoto_cleanup(hr, "path_from_polygon_list() factory->CreatePathGeometry");
	hr = path->Open(&sink);
	OnErrorGoto_cleanup(hr, "path_from_polygon_list path->Open");
	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	if (outtrans) {
		add_counter_clockwise_rect(sink, d2_rectu(whole_rect));
	}
	for( std::vector< std::vector<lib::point> >::iterator polygon_p = polygon_list.begin(); polygon_p != polygon_list.end(); polygon_p++) {
		if ((*polygon_p).size() < 3) {
			lib::logger::get_logger()->debug("path_from_polygon_list: invalid polygon size=%d", (*polygon_p).size());
			continue;
		}
		newpoint_p = (*polygon_p).begin(); 
		old_point = *newpoint_p + origin;
		newpoint_p++;
		sink->BeginFigure(D2D1::Point2F((float) old_point.x, (float) old_point.y), D2D1_FIGURE_BEGIN_FILLED);
		for(;newpoint_p != (*polygon_p).end(); newpoint_p++) {
			lib::point p = *newpoint_p + origin;
			AM_DBG lib::logger::get_logger()->debug("path_from_polygon_list: point=%d, %d", p.x, p.y);
			new_d2_point_2f = D2D1::Point2F((float) p.x, (float) p.y);
			sink->AddLine(new_d2_point_2f);
			old_point  = p;
		}
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	}
	hr = sink->Close();
	OnErrorGoto_cleanup(hr, "path_from_polygon_list path->Close");

cleanup:
	SafeRelease(&sink);
	return path;
}

// Helper function: draw the current render target (from 'dst') using 'polyon list' as a clipping path
// This function does all the work necessary, including the EndDraw() of the bitmap render target (back buffer)
static void
_d2_polygon_list_update(
	common::surface* dst,
	std::vector< std::vector<lib::point> > polygon_list,
	bool outtrans,
	lib::rect whole_rect)
{
	gui_window *window = dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();
	const lib::point& dst_global_topleft = dst->get_global_topleft();
	whole_rect.translate(dst_global_topleft);
	whole_rect &= dst->get_clipped_screen_rect();
	
	ID2D1Layer* layer = NULL;
	ID2D1PathGeometry* path = NULL;
	D2D1_RECT_F d2_full_rect_f = D2D1::RectF();
	D2D1_SIZE_F d2_full_size_f = D2D1::SizeF();
	D2D1_RECT_F cliprect = d2_player->get_current_clip_rectf();
	D2D1::Matrix3x2F transform;
	ID2D1BitmapRenderTarget* brt = d2_player->get_fullscreen_rendertarget();
	if (brt == NULL) {
		brt = d2_player->get_transition_rendertarget();
	}
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	if (brt == NULL || rt == NULL) {
		SafeRelease(&rt);
		return; // nothing to do
	}
	HRESULT hr = rt->CreateLayer(&layer);
	if (FAILED(hr)) {
		lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::polygon[list]::update: CreateLayer returns 0x%x", hr);
	}
	OnErrorGoto_cleanup(hr, "_d2_polygon_list_update() rt->CreateLayer");
	hr = brt->EndDraw();
	OnErrorGoto_cleanup(hr, "_d2_polygon_list_update() brt->EndDraw()");
	d2_full_size_f = brt->GetSize();
	d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
	ID2D1Bitmap* bitmap = NULL;
	hr = brt->GetBitmap(&bitmap);
	OnErrorGoto_cleanup(hr, "_d2_polygon_list_update() brt->GetBitmap()");

	path = path_from_polygon_list(d2_player->get_D2D1Factory(), dst_global_topleft, polygon_list, outtrans, whole_rect);

	rt->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), path), layer);
	// the transtion render target has the same transformation matrix as the bitmap render target had during
	// drawing, therefore during DrawBitmap the rendertarget needs the identity transformation set; otherwise
	// the transformation matrix would be applied twice.
	rt->DrawBitmap(bitmap, d2_full_rect_f, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,	d2_full_rect_f);
	rt->PopLayer();
	hr = rt->Flush();
	OnErrorGoto_cleanup(hr, "_d2_polygon_list_update() rt->Flush()");
cleanup:
	SafeRelease(&layer);
	SafeRelease(&path);
	SafeRelease(&rt);
}

void
d2_transition_blitclass_fade::update()

{
	// AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_fade::update(%f)", m_progress);
	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();

	ID2D1Layer* layer = NULL;
	ID2D1Bitmap* bitmap = NULL;
	D2D1_LAYER_PARAMETERS layer_params = D2D1::LayerParameters();
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	ID2D1BitmapRenderTarget* brt = d2_player->get_fullscreen_rendertarget();
	if (brt == NULL) {
		brt = d2_player->get_transition_rendertarget();
	}
	if (rt == NULL || brt == NULL) {
		SafeRelease(&rt);
		return; // nothing to do
	}
	D2D1_RECT_F cliprect = d2_player->get_current_clip_rectf();	
	HRESULT hr = brt->EndDraw();
	OnErrorGoto_cleanup(hr, "d2_transition_blitclass_fade::update()  brt->EndDraw");
	hr = brt->GetBitmap(&bitmap);
	if (bitmap == NULL) {
		goto cleanup;
	}
	if (m_progress < 1.0) {
		hr = rt->CreateLayer(&layer);
		OnErrorGoto_cleanup(hr, "d2_transition_blitclass_fade::update()  rt->CreateLayer");
		layer_params.opacity = (FLOAT)(m_outtrans ? (1.0 - m_progress) : m_progress);
		// similar to the transformation, when a cliprect was installed it temporarily needs to be popped
		rt->PushLayer(layer_params, layer);
	}
	rt->DrawBitmap(bitmap);
	if (m_progress < 1.0) {
		rt->PopLayer();
	}
cleanup:
	SafeRelease(&layer);
	SafeRelease(&bitmap);
	SafeRelease(&rt);
}

void
d2_transition_blitclass_rect::update()
{
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_rect::update(%f)",m_progress);
	if (m_outtrans) {
		// draw using clipping paths
		std::vector< lib::rect > rect_list = std::vector< lib::rect >();
		rect_list.push_back(m_newrect);
		std::vector< std::vector<lib::point> > polygon_list = polygon_list_from_rect_list(&rect_list);
		_d2_polygon_list_update (m_dst, polygon_list, m_outtrans, m_dst->get_rect());
	} else {
		// Using AxisAlignedClip is more efficient than constructing a clipping path
		gui_window *window = m_dst->get_gui_window();
		d2_window *cwindow = (d2_window *)window;
		d2_player* d2_player = cwindow->get_d2_player();

		lib::rect newrect_whole = translate_and_clip_rect (m_newrect, m_dst);
		if (newrect_whole.empty())
			return;
		D2D1_RECT_F d2_new_rect_f;
		D2D1_RECT_F d2_full_rect_f;
		D2D1_SIZE_F d2_full_size_f;
		ID2D1BitmapRenderTarget* brt = d2_player->get_fullscreen_rendertarget();
		if (brt == NULL) {
			brt = d2_player->get_transition_rendertarget();
		}
		ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
		if (brt == NULL || rt == NULL) {
			SafeRelease(&rt);
			return; // nothing to do
		}
		HRESULT hr = brt->EndDraw();
		if (FAILED(hr)) {
			return;
		}
		D2D1_RECT_F cliprect = d2_player->get_current_clip_rectf();	
		d2_new_rect_f = d2_rectf(newrect_whole);
		d2_full_size_f = brt->GetSize();
		d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
		ID2D1Bitmap* bitmap = NULL;
		hr = brt->GetBitmap(&bitmap);
		if (SUCCEEDED(hr)) {
			rt->PushAxisAlignedClip(d2_new_rect_f, D2D1_ANTIALIAS_MODE_ALIASED);
			rt->DrawBitmap(bitmap,
					d2_full_rect_f,
					1.0f,
					D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
					d2_full_rect_f);
			rt->PopAxisAlignedClip();
			hr = rt->Flush();
			if (FAILED(hr)) {
				lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::rect::update: DrawBitmap returns 0x%x", hr);
			}
		}  // otherwise HRESULT failure is ignored, may happen e.g. when bitmap is empty
		rt->Release();
	}
}

// Helper function: return transform point `p' by applying matrix `m'
D2D1_POINT_2U
transform(D2D1_POINT_2U p, D2D1_MATRIX_3X2_F m)
{
	return D2D1::Point2U(UINT32(p.x*m._11+p.y*m._21+m._31),UINT32(p.x*m._12+p.y*m._22+m._32));
}

// Helper function: return transformed rectangle `r' by applying matrix `m'
D2D1_RECT_U
transform (D2D1_RECT_U r, D2D1_MATRIX_3X2_F m)
{
	D2D1_POINT_2U lt = transform(D2D1::Point2U(r.left, r.top), m);
	D2D1_POINT_2U rb = transform(D2D1::Point2U(r.right, r.bottom), m);
	return D2D1::RectU(lt.x, lt.y, rb.x, rb.y);
}

#ifdef	AM_DBG
void
draw_rect(lib::rect r, ID2D1RenderTarget* rt, D2D1::ColorF colorspec = D2D1::ColorF::Red, bool needBeginEnd = false)
{
	ID2D1SolidColorBrush* color;
	HRESULT hr = rt->CreateSolidColorBrush(
                D2D1::ColorF(colorspec),
                &color);
	if (needBeginEnd) {
		rt->BeginDraw();
	}
	rt->DrawRectangle(d2_rectf(r), color, 5.0);
	if (needBeginEnd) {
		rt->EndDraw();
	}
}
#endif//AM_DBG

// return the difference of 2 rectangles for those boundary cases where the result
// is also a single rectangle. Return an empty rectangle in all other cases.
// Used for pushWipe out transitions.
lib::rect
diff_rect(lib::rect A, lib::rect B)
{
	lib::rect rv = lib::rect();
	if (A != B) {
		if ((A.size().w < B.size().w) ^ /*XOR*/ (A.size().h < B.size().h)) {
			lib::rect tmp = A;
			A = B;
			B = tmp;
		}
		if (A.top() == B.top() && A.height() == B.height()) {
			if (A.left_top() == B.left_top() && A.right_bottom() != B.right_bottom()) {
				rv.x = A.x + B.w;
				rv.y = A.y;
				rv.w = A.w - B.w;
				rv.h = A.h;
			} else if (A.left_top() != B.left_top() && A.right_bottom() == B.right_bottom()) {
				rv.x = A.x;
				rv.y = A.y;
				rv.w = A.w - B.w;
				rv.h = A.h;
			}
		} else if (A.left() == B.left() && A.width() == B.width()) {
			if (A.left_top() == B.left_top() && A.right_bottom() != B.right_bottom()) {
				rv.x = A.x;
				rv.y = A.y + B.h;
				rv.w = A.w;
				rv.h = A.h - B.h;
			} else if (A.left_top() != B.left_top() && A.right_bottom() == B.right_bottom()) {
				rv.x = A.x;
				rv.y = A.y;
				rv.w = A.w;
				rv.h = A.h - B.h;
			}
		}
	}
	return rv;
}

// copy the area `rect1' on rendertarget `rt1' and draw it as a bitmap to `rect2' on rendertarget `rt2'
// if `cliprect` is has been pushed (and given) for `rt1', it will be popped from it and restored.
// Returns `false' on error
// Used for pushWipe/slideWipe transitions
bool
copy_draw_rectu(ID2D1RenderTarget* rt1, D2D1_RECT_U* rect1, ID2D1RenderTarget* rt2, D2D1_RECT_U* rect2,
				D2D1_RECT_F* cliprect = NULL)
{
	if (rt1 == NULL || rect1 == NULL || rt2 == NULL || rect2 == NULL) {
		return false;
	}
	HRESULT hr = E_FAIL;
	ID2D1Bitmap* bitmap = NULL;
	// we need to use ID2D1Bitmap::CopyFromRenderTarget, therefore we must create the `bitmap'
	// where w'll put the data into with equal properties as its data source `rt1'
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	D2D1::Matrix3x2F matrix;
	rt1->GetDpi(&props.dpiX, &props.dpiY);
	props.pixelFormat = rt1->GetPixelFormat();
	rt1->GetTransform(&matrix);
	D2D1_RECT_U tr_rect1 = transform(*rect1, matrix);
	D2D1_SIZE_U tr_size1 = D2D1::SizeU(tr_rect1.right - tr_rect1.left, tr_rect1.bottom - tr_rect1.top);
	D2D1_RECT_F rect2_f  = D2D1::RectF(float(rect2->left),float(rect2->top),float(rect2->right),float(rect2->bottom));

	hr = rt1->CreateBitmap(tr_size1, props, &bitmap);
	OnErrorGoto_cleanup(hr,"d2_transition::copy_draw_draw: rt1->CreateBitmap");

	// copy the bits of the pixels from `rt1' to the new destination on `rt2'
	// to be able to use CopyFromRenderTarget, the current cliprect must be temporary popped.
	if (cliprect != NULL && ! (cliprect->bottom == 0.0F && cliprect->left == 0.0F &&
								cliprect->right == 0.0F && cliprect->left == 0.0F)) {
		rt1->PopAxisAlignedClip();
	}
	hr = bitmap->CopyFromRenderTarget(NULL, rt1, &tr_rect1);

	if (cliprect != NULL && ! (cliprect->bottom == 0.0F && cliprect->left == 0.0F &&
								cliprect->right == 0.0F && cliprect->left == 0.0F)) {
		rt1->PushAxisAlignedClip(cliprect, D2D1_ANTIALIAS_MODE_ALIASED);
	}
	OnErrorGoto_cleanup(hr,"d2_transition::copy_draw_draw: bitmap->CopyFromRenderTarget");
	rt2->DrawBitmap(bitmap, rect2_f);
	hr = rt2->Flush();

cleanup:
	SafeRelease(&bitmap);
	return hr == S_OK;
}

// Used for pushWipe/slideWipe transitions
void
d2_transition_blitclass_r1r2r3r4::update()
{
	//AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_r1r2r3r4::update(%f)", m_progress);

	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();
	lib::rect newsrcrect = translate_and_clip_rect(m_newsrcrect, m_dst);
	lib::rect newdstrect = translate_and_clip_rect(m_newdstrect, m_dst);
	lib::rect oldsrcrect = translate_and_clip_rect(m_oldsrcrect, m_dst);
	lib::rect olddstrect = translate_and_clip_rect(m_olddstrect, m_dst);
	D2D1_RECT_U oldsrcrectu, olddstrectu, newsrcrectu, newdstrectu;

	ID2D1Bitmap* bitmap_old = NULL; // bitmap for the "old" stuff
	ID2D1Bitmap* bitmap_new = NULL; // bitmap for the "new" stuff
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	D2D1_MATRIX_3X2_F d2_rt_transform;
	D2D1_RECT_F cliprect = d2_player->get_current_clip_rectf();
	ID2D1BitmapRenderTarget* brt = d2_player->get_fullscreen_rendertarget();
	if (brt == NULL) {
		brt = d2_player->get_transition_rendertarget();
	}
	if (brt == NULL || rt == NULL) {
		SafeRelease(&rt);
		return; // nothing to do
	}
	ID2D1RenderTarget* dst_rt = rt, *old_rt = rt, *new_rt = brt;
	rt->GetTransform(&d2_rt_transform);
	HRESULT hr = brt->EndDraw();
	OnErrorGoto_cleanup(hr, "d2_transition_blitclass_r1r2r3r4() brt->EndDraw()");

	// Get needed parts of the old and new stuff (as bitmaps) and draw them at their final destinations
	if (m_outtrans) {
		// exchange "old" and "new" rects, but not the render targets
		if (m_info->m_type == slideWipe) {
			lib::rect dst_rect = m_dst->get_rect();
			dst_rect.translate(m_dst->get_global_topleft());
			oldsrcrect = diff_rect(dst_rect, oldsrcrect);
			olddstrect = diff_rect(dst_rect, olddstrect);
			newsrcrect = diff_rect(dst_rect, newsrcrect);
			newdstrect = diff_rect(dst_rect, newdstrect);
		} else { // pushWipe
			lib::rect tmp_rect;
			tmp_rect   = oldsrcrect;
			oldsrcrect = newsrcrect;
			newsrcrect = tmp_rect;
			tmp_rect   = olddstrect;
			olddstrect = newdstrect;
			newdstrect = tmp_rect;
		}
	}
	// convert to d2 rectangles
	oldsrcrectu = d2_rectu(oldsrcrect);
	olddstrectu = d2_rectu(olddstrect);
	newsrcrectu = d2_rectu(newsrcrect);
	newdstrectu = d2_rectu(newdstrect);
	copy_draw_rectu(old_rt, &oldsrcrectu, dst_rt, &olddstrectu, &cliprect);
	copy_draw_rectu(new_rt, &newsrcrectu, dst_rt, &newdstrectu);

cleanup:
	SafeRelease(&bitmap_old);
	SafeRelease(&bitmap_new);
}

void
d2_transition_blitclass_rectlist::update()
{
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_rectlist::update(%f)", m_progress);
	if (m_outtrans) {
		// draw using clipping paths
		std::vector< std::vector<lib::point> > polygon_list = polygon_list_from_rect_list(&m_newrectlist);
		_d2_polygon_list_update (m_dst, polygon_list, m_outtrans, m_dst->get_rect());
	} else {
// Using axis aligened clip is more effecient than using a path, though DrawBitmap is called inside a loop
		gui_window *window = m_dst->get_gui_window();
		d2_window *cwindow = (d2_window *)window;
		d2_player* d2_player = cwindow->get_d2_player();

		lib::rect newrect_whole = m_dst->get_rect();
		newrect_whole.translate(m_dst->get_global_topleft());
		newrect_whole &= m_dst->get_clipped_screen_rect();
		lib::point LT = newrect_whole.left_top();
		lib::point RB = newrect_whole.right_bottom();
		if (newrect_whole.empty())
			return;
		std::vector< lib::rect >::iterator newrect;
			
		ID2D1BitmapRenderTarget* brt = d2_player->get_fullscreen_rendertarget();
		if (brt == NULL) {
			brt = d2_player->get_transition_rendertarget();
		}
		ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
		if (brt == NULL || rt ==  NULL) {
			SafeRelease(&rt);
			return; // nothing to do
		}
		HRESULT hr = brt->EndDraw();
		if (FAILED(hr)) {
			return;
		}
		D2D1_RECT_F cliprect = d2_player->get_current_clip_rectf();	
		D2D1_SIZE_F d2_full_size_f = brt->GetSize();
		D2D1_RECT_F d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
		ID2D1Bitmap* bitmap = NULL;
		hr = brt->GetBitmap(&bitmap);
		if (FAILED(hr)) {
			SafeRelease(&rt);
			return;
		}
		for (newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
			lib::rect newrect_whole = *newrect;
			if (newrect_whole.empty()) {
				continue;
			}
			newrect_whole.translate(m_dst->get_global_topleft());
			newrect_whole &= m_dst->get_clipped_screen_rect();
			D2D1_RECT_F d2_new_rect_f = d2_rectf(newrect_whole);
			rt->PushAxisAlignedClip(d2_new_rect_f, D2D1_ANTIALIAS_MODE_ALIASED);
			rt->DrawBitmap(bitmap, d2_full_rect_f, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,	d2_full_rect_f);
			rt->PopAxisAlignedClip();
			hr = rt->Flush();
			if (FAILED(hr)) {
				lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::rectlist::update: DrawBitmap returns 0x%x", hr);
				break;
			}
		}
		AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_rectlist::update(%f) newrect_whole=(%d,%d),(%d,%d)",m_progress,LT.x,LT.y,RB.x,RB.y);
		SafeRelease(&rt);
	}
}

void
d2_transition_blitclass_poly::update()
{
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	std::vector< std::vector<lib::point> > polygon_list;
	polygon_list.push_back(m_newpolygon);
	_d2_polygon_list_update(m_dst, polygon_list, m_outtrans, m_dst->get_rect());
}

void
d2_transition_blitclass_polylist::update()
{
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_polylist::update(%f)", m_progress);
	_d2_polygon_list_update(m_dst, m_newpolygonlist, m_outtrans, m_dst->get_rect());
}

smil2::transition_engine *
d2_transition_engine(common::surface *dst, bool is_outtrans, const lib::transition_info *info)
{
	smil2::transition_engine *rv;

	switch(info->m_type) {
	// Series 1: edge wipes
	case lib::barWipe:
		rv = new d2_transition_engine_barwipe();
		break;
	case lib::boxWipe:
		rv = new d2_transition_engine_boxwipe();
		break;
	case lib::fourBoxWipe:
		rv = new d2_transition_engine_fourboxwipe();
		break;
	case lib::barnDoorWipe:
		rv = new d2_transition_engine_barndoorwipe();
		break;
	case lib::diagonalWipe:
		rv = new d2_transition_engine_diagonalwipe();
		break;
	case lib::miscDiagonalWipe:
		rv = new d2_transition_engine_miscdiagonalwipe();
		break;
	case lib::veeWipe:
		rv = new d2_transition_engine_veewipe();
		break;
	case lib::barnVeeWipe:
		rv = new d2_transition_engine_barnveewipe();
		break;
	case lib::zigZagWipe:
		rv = new d2_transition_engine_zigzagwipe();
		break;
	case lib::barnZigZagWipe:
		rv = new d2_transition_engine_barnzigzagwipe();
		break;
	case lib::bowTieWipe:
		rv = new d2_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case lib::irisWipe:
		rv = new d2_transition_engine_iriswipe();
		break;
	case lib::pentagonWipe:
		rv = new d2_transition_engine_pentagonwipe();
		break;
	case lib::arrowHeadWipe:
		rv = new d2_transition_engine_arrowheadwipe();
		break;
	case lib::triangleWipe:
		rv = new d2_transition_engine_trianglewipe();
		break;
	case lib::hexagonWipe:
		rv = new d2_transition_engine_hexagonwipe();
		break;
	case lib::eyeWipe:
		rv = new d2_transition_engine_eyewipe();
		break;
	case lib::roundRectWipe:
		rv = new d2_transition_engine_roundrectwipe();
		break;
	case lib::ellipseWipe:
		rv = new d2_transition_engine_ellipsewipe();
		break;
	case lib::starWipe:
		rv = new d2_transition_engine_starwipe();
		break;
	case lib::miscShapeWipe:
		rv = new d2_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case lib::clockWipe:
		rv = new d2_transition_engine_clockwipe();
		break;
	case lib::singleSweepWipe:
		rv = new d2_transition_engine_singlesweepwipe();
		break;
	case lib::doubleSweepWipe:
		rv = new d2_transition_engine_doublesweepwipe();
		break;
	case lib::saloonDoorWipe:
		rv = new d2_transition_engine_saloondoorwipe();
		break;
	case lib::windshieldWipe:
		rv = new d2_transition_engine_windshieldwipe();
		break;
	case lib::fanWipe:
		rv = new d2_transition_engine_fanwipe();
		break;
	case lib::doubleFanWipe:
		rv = new d2_transition_engine_doublefanwipe();
		break;
	case lib::pinWheelWipe:
		rv = new d2_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case lib::snakeWipe:
		rv = new d2_transition_engine_snakewipe();
		break;
	case lib::waterfallWipe:
		rv = new d2_transition_engine_waterfallwipe();
		break;
	case lib::spiralWipe:
		rv = new d2_transition_engine_spiralwipe();
		break;
	case lib::parallelSnakesWipe:
		rv = new d2_transition_engine_parallelsnakeswipe();
		break;
	case lib::boxSnakesWipe:
		rv = new d2_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case lib::pushWipe:
		rv = new d2_transition_engine_pushwipe();
		break;
	case lib::slideWipe:
		rv = new d2_transition_engine_slidewipe();
		break;
	case lib::fade:
	case lib::audioVisualFade:
		rv = new d2_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->trace("d2_transition_engine: transition type %s not yet implemented",
			repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}
} // namespace d2
} // namespace gui
} // namespace ambulant

