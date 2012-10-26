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


#include "ambulant/config/config.h"
#define INITGUID
#include <objbase.h>
#include <ddraw.h>
#include <uuids.h>
#include <windows.h>
#include <mmsystem.h>
// For older versions of DirectX, this could be d3d8types.h
#include <d3d9types.h>
#include <wincodec.h>

#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_audio_player.h" // Only to define the TPB GUID


#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"ddraw.lib")

#include "ambulant/lib/logger.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/gui/dx/dx_transition.h"

#include <algorithm>
#include <cassert>

// Windows Mobile 5 has some different names:
#ifdef DDLOCK_WAIT
#define AM_DDLOCK_WAIT DDLOCK_WAIT
#else
#define AM_DDLOCK_WAIT DDLOCK_WAITNOTBUSY
#endif

#ifdef DDBLT_WAIT
#define AM_DDBLT_WAIT DDBLT_WAIT
#else
#define AM_DDBLT_WAIT DDBLT_WAITNOTBUSY
#endif

using namespace ambulant;

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using lib::uint16;
using lib::uint32;
using lib::uchar;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;

// --------------------------------------------------------------

// DirectX parameter handling.
// white is used as transparent color
const lib::color_t CLR_DEFAULT		= RGB(255, 255, 255);
// almost white is used as alternative color for white
const lib::color_t CLR_ALTERNATIVE	= RGB(255, 255, 254);


class dxparams_rgb : public gui::dx::dxparams {
public:
	lib::color_t transparent_color() { return RGB(255, 255, 255); }
	lib::color_t transparent_replacement_color() { return RGB(255, 255, 254); }
	lib::color_t invalid_color() { return CLR_INVALID; }
	DWORD bmi_compression() { return BI_RGB; }
	const GUID& wic_format() { return GUID_WICPixelFormat32bppBGR; }
	void fill_ddsd(DDSURFACEDESC& sd, DWORD flags) {
		memset(&sd, 0, sizeof(DDSURFACEDESC));
		sd.dwSize = sizeof(DDSURFACEDESC);
		sd.dwFlags = flags;
	}
	struct _DDBLTFX *ddbltfx() { return NULL; }
};


class dxparams_rgba : public gui::dx::dxparams {
public:
	lib::color_t transparent_color() { return 0; }
	lib::color_t transparent_replacement_color() { return 0; }
	lib::color_t invalid_color() { return 0x00123456; } // fully-transparent random-value:-)
	DWORD bmi_compression() { return BI_BITFIELDS; }
	const GUID& wic_format() { return GUID_WICPixelFormat32bppBGRA; }
	void fill_ddsd(DDSURFACEDESC& sd, DWORD flags) {
		memset(&sd, 0, sizeof(DDSURFACEDESC));
		sd.dwSize = sizeof(DDSURFACEDESC);
		sd.dwFlags = flags | DDSD_PIXELFORMAT;
		sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		sd.ddpfPixelFormat.dwFlags = 
			DDPF_ALPHAPIXELS|DDPF_ALPHAPREMULT|DDPF_RGB;
		sd.ddpfPixelFormat.dwRGBBitCount = 32;
		sd.ddpfPixelFormat.dwRBitMask = 0xff0000;
		sd.ddpfPixelFormat.dwGBitMask = 0xff00;
		sd.ddpfPixelFormat.dwBBitMask = 0xff;
		sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
	}
	struct _DDBLTFX *ddbltfx() { return NULL; }
};

static gui::dx::dxparams* cur_dxparams = NULL;

gui::dx::dxparams* gui::dx::dxparams::I() {
	if (cur_dxparams == NULL) {
		cur_dxparams = new dxparams_rgb();
	}
	return cur_dxparams;
}

#define RELEASE(x) if(x) x->Release();x=NULL;

static lib::logger* viewport_logger = NULL;

static void
seterror(const char *funcname, HRESULT hr){
	TCHAR* pszmsg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &pszmsg,
		0,
		NULL
		);
	lib::textptr msg(pszmsg);
	viewport_logger->error( "%s failed, error = %x, %s", funcname, hr, msg.c_str());
	LocalFree(pszmsg);
}
// wrapper for Blt on primary surface to check for recoverable errors
#define MAX_RETRIES 1
static void
primary_Blt(IDirectDrawSurface* primary_surface, LPRECT lpDestRect,
			LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFX){
	HRESULT hr = DD_OK;
	int retries = MAX_RETRIES;
	while (retries--) {
		// Workaround by Jack for the problem that, under
		// Parallels and other virtual machines, and possibly also sometimes
		// on a real machine, content is drawn at the topleft of the scree
		// in stead of where it should be drawn. And this is despite the
		// use of a clipper on primary_surface...
		if (lpDestRect->left < 0) {
			lpSrcRect->left -= lpDestRect->left;
			lpDestRect->left = 0;
		}
		if (lpDestRect->top < 0) {
			lpSrcRect->top -= lpDestRect->top;
			lpDestRect->top = 0;
		}
		lib::size screensize = gui::dx::viewport::get_size(primary_surface);
		if (lpDestRect->right > (LONG)screensize.w) {
			lpSrcRect->right -= (lpDestRect->right - screensize.w);
			lpDestRect->right = screensize.w;
		}
		if (lpDestRect->bottom > (LONG)screensize.h) {
			lpSrcRect->bottom -= (lpDestRect->bottom - screensize.h);
			lpDestRect->bottom = screensize.h;
		}
		if (lpDestRect->left >= lpDestRect->right) return;
		if (lpDestRect->top >= lpDestRect->bottom) return;

		hr = primary_surface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFX);
		if (hr == DDERR_NOTFOUND) return; // XXXJACK
		if (hr == DDERR_SURFACELOST && retries >= 0) {
			viewport_logger->trace("primary_Blt recovering from DDERR_LOSTSURFACE retry=%d", MAX_RETRIES-retries);
			hr = primary_surface->Restore();
			if (FAILED(hr)) {
				seterror("primary_Blt/DirectDrawSurface::Restore()", hr);
				return;
			}
		} else break;
	}
	if (FAILED(hr))
		seterror("primary_Blt/DirectDrawSurface::Blt()", hr);
}

gui::dx::viewport::viewport(int width, int height, HWND hwnd)
:	m_width(width), m_height(height),
	m_direct_draw(NULL),
	m_primary_surface(NULL),
	m_surface(NULL),
	m_fstr_surface(NULL),
	m_fstransition(NULL),
	m_hwnd(hwnd?hwnd:GetDesktopWindow()),
	bits_size(24),
	red_bits(8), green_bits(8), blue_bits(8),
	lo_red_bit(16), lo_green_bit(8), lo_blue_bit(0),
	palette_entries(0),
	m_bgd(dxparams::I()->transparent_color()) {

	viewport_logger = lib::logger::get_logger();

	HRESULT hr;
	IDirectDraw	 *pDD1=NULL;
	hr = DirectDrawCreate(NULL, &pDD1, NULL);

	if (FAILED(hr)){
		seterror("CreateDirectDraw()", hr);
		return;
	}
	hr = pDD1->QueryInterface(IID_IDirectDraw2, (void**)&m_direct_draw);
	if (FAILED(hr)){
		seterror("QueryInterface(IID_IDirectDraw2,...)", hr);
		pDD1->Release();
		return;
	}
	pDD1->Release();

	DWORD flags = DDSCL_NORMAL;
	hr = m_direct_draw->SetCooperativeLevel(m_hwnd, flags);
	if (FAILED(hr)) {
		seterror("SetCooperativeLevel()", hr);
		return;
	}

	// create primary surface
	DDSURFACEDESC sd;
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_CAPS;
	sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	sd.dwWidth = m_width;
	sd.dwHeight = m_height;
	hr = m_direct_draw->CreateSurface(&sd, &m_primary_surface, NULL);
	if (FAILED(hr)) {
		seterror("DirectDraw::CreateSurface()", hr);
		return;
	}

	get_pixel_format();
	// Clip output to the provided window
	if(m_hwnd) {
		IDirectDrawClipper *clipper = NULL;
		hr = m_direct_draw->CreateClipper(0, &clipper, NULL);
		if (FAILED(hr))
			seterror("DirectDraw::CreateClipper()", hr);
		hr = clipper->SetHWnd(0, m_hwnd);
		if (FAILED(hr))
			seterror("DirectDrawSurface::SetHWnd()", hr);
		else {
			hr = m_primary_surface->SetClipper(clipper);
			if (FAILED(hr))
				seterror("DirectDrawSurface::SetClipper()", hr);
		}
		clipper->Release();
	}
	{
		// Here follows a bit of magic code. As explained in bug #2996614,
		// in some situations on Win7 (and vista) Ambulant can black out the whole
		// screen. It seems this somehow happens because the urface can be queried
		// before having been used. The following code seems to instantiate the
		// surface, and fixes this bug.
		// Note that Lock/Unlock used different LPRECT arguments (despite what the
		// MS docs say), using the same argument to Unlock causes a "not locked"
		// error. Sigh.
		RECT r = {0, 0, 0, 0};
		DDSURFACEDESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.dwSize = sizeof(desc);
		desc.dwFlags = 0;
		hr = m_primary_surface->Lock(&r, &desc, AM_DDLOCK_WAIT|DDLOCK_READONLY, NULL);
		hr = m_primary_surface->Unlock(0);
	}

	// create drawing surface
	dxparams::I()->fill_ddsd(sd, DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = m_width;
	sd.dwHeight = m_height;
	hr = m_direct_draw->CreateSurface(&sd, &m_surface, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return;
	}

	m_ddbgd = convert(m_bgd);

	// clear the back buffer
	clear();

	// create shared transition surface
	IDirectDrawSurface* surf;
	dxparams::I()->fill_ddsd(sd, DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = m_width;
	sd.dwHeight = m_height;
	hr = m_direct_draw->CreateSurface(&sd, &surf, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return;
	}
	m_surfaces.push_back(surf);
	dxparams::I()->fill_ddsd(sd, DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = m_width;
	sd.dwHeight = m_height;
	hr = m_direct_draw->CreateSurface(&sd, &m_fstr_surface, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return;
	}
}

gui::dx::viewport::~viewport() {
	std::list<IDirectDrawSurface*>::iterator it;
	for(it=m_surfaces.begin();it!=m_surfaces.end();it++)
		(*it)->Release();
	RELEASE(m_surface);
	RELEASE(m_primary_surface);
	RELEASE(m_fstr_surface);
	RELEASE(m_direct_draw);
	if(palette_entries != 0)
		delete[] palette_entries;
	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

// Sets the background color of this viewport
void
gui::dx::viewport::set_background(lib::color_t color) {
	m_bgd = (color == dxparams::I()->invalid_color())?dxparams::I()->transparent_color():color;
	m_ddbgd = convert(m_bgd);
}

// Creates a DD surface with the provided size.
// The surface is cleared using the specified color
IDirectDrawSurface*
gui::dx::viewport::create_surface(DWORD w, DWORD h) {
	IDirectDrawSurface* surface = 0;
	DDSURFACEDESC sd;
	dxparams::I()->fill_ddsd(sd, DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = w;
	sd.dwHeight = h;

	HRESULT hr = m_direct_draw->CreateSurface(&sd, &surface, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return 0;
	}
	return surface;
}

IDirectDrawSurface*
gui::dx::viewport::create_surface() {
	IDirectDrawSurface* surf = 0;
	if(m_surfaces.empty()) {
		return create_surface(m_width, m_height);
	}
	std::list<IDirectDrawSurface*>::iterator it = m_surfaces.begin();
	surf = *it;
	m_surfaces.erase(it);
	return surf;
}

void
gui::dx::viewport::release_surface(IDirectDrawSurface* surf) {
	assert (surf != NULL);
	m_surfaces.push_back(surf);
}

// Blt back buffer to primary surface
void
gui::dx::viewport::redraw() {
	if(!m_primary_surface || !m_surface)
		return;
	RECT src_rc = {0, 0, m_width, m_height};
	RECT dst_rc = {0, 0, m_width, m_height};
	lib::rect ourrect(lib::point(0,0), lib::size(m_width, m_height));
	DWORD flags = AM_DDBLT_WAIT;

	if (m_fstransition) {
		AM_DBG lib::logger::get_logger()->debug("viewport::redraw: Applying fullscreen transition");
		// Create a temp surface, determine bg/fg surfaces and copy the background
		IDirectDrawSurface *tmps = create_surface();
		clear_surface(tmps, lib::color_t(0xff0000), 1.0);
		IDirectDrawSurface *s1, *s2;
		if (m_fstransition->is_outtrans()) {
			s1 = m_surface;
			s2 = m_fstr_surface;
		} else {
			s1 = m_fstr_surface;
			s2 = m_surface;
		}
		//draw(s1, ourrect, ourrect, false, tmps);

		// Determine blitter type and blend in the fg surface
		smil2::blitter_type bt = m_fstransition->get_blitter_type();
		if (bt == smil2::bt_r1r2r3r4) {
			lib::rect src_rc_v = ourrect;
			lib::rect dst_rc_v = ourrect;
			clipto_r1r2r3r4(m_fstransition, src_rc_v, dst_rc_v);
			draw(s2, dst_rc_v, dst_rc_v, false, tmps);
		} else if (bt == smil2::bt_fade) {
			if ( ! blt_blend(tmps, s2, ourrect, m_fstransition->get_progress(), 0, 0xFFFFFFFF, true))
				draw(s2, ourrect, ourrect, false, tmps);
		} else {
			HRGN hrgn = NULL;
			switch (bt) {
				case smil2::bt_rect:
					hrgn = create_rect_region(m_fstransition);
					break;
				case smil2::bt_rectlist:
					hrgn = create_rectlist_region(m_fstransition);
					break;
				case smil2::bt_poly:
					hrgn = create_poly_region(m_fstransition);
					break;
				case smil2::bt_polylist:
					hrgn = create_polylist_region(m_fstransition);
					break;
			}
			HDC tmps_dc;
			HRESULT hr = tmps->GetDC(&tmps_dc);
			if (FAILED(hr)) {
				seterror("DirectDrawSurface::GetDC()", hr);
				return;
			}
			HDC s2_dc;
			hr = s2->GetDC(&s2_dc);
			if (FAILED(hr)) {
				seterror("DirectDrawSurface::GetDC()", hr);
				return;
			}
			if (hrgn) SelectClipRgn(tmps_dc, hrgn);
			int w, h;
			w = dst_rc.right - dst_rc.left;
			h = dst_rc.bottom - dst_rc.top;
			BitBlt(tmps_dc, dst_rc.left, dst_rc.top, w, h, s2_dc, dst_rc.left, dst_rc.top, SRCCOPY);
			tmps->ReleaseDC(tmps_dc);
			s2->ReleaseDC(s2_dc);
		}
		primary_Blt(m_primary_surface, to_screen_rc_ptr(dst_rc), tmps, &src_rc, flags, dxparams::I()->ddbltfx());
		release_surface(tmps);
	} else {
		// Copy to screen
		primary_Blt(m_primary_surface, to_screen_rc_ptr(dst_rc), m_surface, &src_rc, flags, dxparams::I()->ddbltfx());
		// Copy to backing store for posible fs transition later
		HRESULT hr = m_fstr_surface->Blt(&src_rc, m_surface, &src_rc, flags, NULL);
		if (hr == DDERR_NOTFOUND) return;
		if (FAILED(hr)) {
			seterror("viewport::redraw()/DirectDrawSurface::Blt() m_fstr_surface", hr);
		}
	}
}

void
gui::dx::viewport::redraw(const lib::rect& rc) {
	if(!m_primary_surface || !m_surface)
		return;
	RECT src_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	RECT dst_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};

	// Convert dst to screen coordinates
	to_screen_rc_ptr(dst_rc);
	assert(rc.left() <= m_width);
	assert(rc.right() <= m_width);
	assert(rc.top() <= m_height);
	assert(rc.bottom() <= m_height);
	// Verify:
	if(IsRectEmpty(&src_rc) || IsRectEmpty(&dst_rc))
		return;
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&src_rc, &src_rc, &vrc) || IsRectEmpty(&src_rc))
		return;

	// Blit:
	lib::rect ourrect(lib::point(0,0), lib::size(m_width, m_height));
	DWORD flags = AM_DDBLT_WAIT;

	if (m_fstransition) {
		AM_DBG lib::logger::get_logger()->debug("viewport::redraw: Applying fullscreen transition");
		// Create a temp surface, determine bg/fg surfaces and copy the background
		IDirectDrawSurface *tmps = create_surface();
		clear_surface(tmps, lib::color_t(0x00ff00), 1.0);
		IDirectDrawSurface *s1, *s2;
		if (m_fstransition->is_outtrans()) {
			s1 = m_surface;
			s2 = m_fstr_surface;
		} else {
			s1 = m_fstr_surface;
			s2 = m_surface;
		}
		draw(s1, ourrect, ourrect, false, tmps);

		// Determine blitter type and blend in the fg surface
		smil2::blitter_type bt = m_fstransition->get_blitter_type();
		if (bt == smil2::bt_r1r2r3r4) {
			lib::rect src_rc_v = ourrect;
			lib::rect dst_rc_v = ourrect;
			clipto_r1r2r3r4(m_fstransition, src_rc_v, dst_rc_v);
			draw(s2, dst_rc_v, dst_rc_v, false, tmps);
		} else if (bt == smil2::bt_fade) {
			if ( ! blt_blend(tmps, s2, ourrect, m_fstransition->get_progress(), 0xFFFFFFFF, 0, 0xFFFFFFFF))
				draw(s2, ourrect, ourrect, false, tmps);
		} else {
			HRGN hrgn = NULL;
			switch (bt) {
				case smil2::bt_rect:
					hrgn = create_rect_region(m_fstransition);
					break;
				case smil2::bt_rectlist:
					hrgn = create_rectlist_region(m_fstransition);
					break;
				case smil2::bt_poly:
					hrgn = create_poly_region(m_fstransition);
					break;
				case smil2::bt_polylist:
					hrgn = create_polylist_region(m_fstransition);
					break;
			}
			HDC tmps_dc;
			HRESULT hr = tmps->GetDC(&tmps_dc);
			if (FAILED(hr)) {
				seterror("DirectDrawSurface::GetDC()", hr);
				return;
			}
			HDC s2_dc;
			hr = s2->GetDC(&s2_dc);
			if (FAILED(hr)) {
				seterror("DirectDrawSurface::GetDC()", hr);
				return;
			}
			if (hrgn) SelectClipRgn(tmps_dc, hrgn);
			int w, h;
			w = src_rc.right - src_rc.left;
			h = src_rc.bottom - src_rc.top;
			BitBlt(tmps_dc, src_rc.left, src_rc.top, w, h, s2_dc, src_rc.left, src_rc.top, SRCCOPY);
			tmps->ReleaseDC(tmps_dc);
			s2->ReleaseDC(s2_dc);
		}
		primary_Blt(m_primary_surface, &dst_rc, tmps, &src_rc, flags, dxparams::I()->ddbltfx());
	} else {
		// Copy to screen
		primary_Blt(m_primary_surface, &dst_rc, m_surface, &src_rc, flags, dxparams::I()->ddbltfx());
		// Copy to backing store, for later use with transition
		// XXX Or should we copy the whole surface?
		HRESULT hr = m_fstr_surface->Blt(&src_rc, m_surface, &src_rc, flags, NULL);
		if (hr == DDERR_NOTFOUND) return; // XXXJACK
		if (FAILED(hr)) {
			lib::logger::get_logger()->trace("DirectDrawSurface::Blt: error 0x%x. rect (%d,%d,%d,%d)",
				hr, src_rc.left, src_rc.top, src_rc.right, src_rc.bottom);
			static bool warned = false;
			if (!warned) seterror("viewport::redraw()/DirectDrawSurface::Blt()", hr);
			warned = true;
		}
	}
}

void
gui::dx::viewport::schedule_redraw() {
	redraw();
}
void
gui::dx::viewport::schedule_redraw(const lib::rect& rc) {
	redraw(rc);
}

// Clears the back buffer using this viewport bgd color
void
gui::dx::viewport::clear() {
	if(!m_surface) return;
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = m_ddbgd;
	RECT dst_rc = {0, 0, m_width, m_height};
	HRESULT hr = m_surface->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | AM_DDBLT_WAIT, &bltfx);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

bool
gui::dx::viewport::blt_blend (IDirectDrawSurface* to, IDirectDrawSurface* from, const lib::rect& rc, double opacity_in,	 double opacity_out, lib::color_t low_chroma, lib::color_t high_chroma) {
	bool rv = true;
	uint32 low_ddclr = convert(low_chroma),	 high_ddclr = convert(high_chroma);
	HRESULT hr = S_OK;
	if (bits_size == 32) {
		hr = blt_blend32(rc, opacity_in, opacity_out, from, to, low_ddclr, high_ddclr);
	} else if( bits_size == 24) {
		hr = blt_blend24(rc, opacity_in, opacity_out, from, to, low_ddclr, high_ddclr);
	} else if (bits_size == 16) {
		hr = blt_blend16(rc, opacity_in, opacity_out, from, to, low_ddclr, high_ddclr);
	} else {
		rv = false;
	}
	if (FAILED(hr)) {
		seterror("blt_blend()", hr);
		rv = false;
	}
	return rv;
}

// Clears the specified back buffer rectangle using the provided color and taking into account any transition
void
gui::dx::viewport::clear(const lib::rect& rc, lib::color_t clr, double opacity, dx_transition *tr) {
	if(m_surface == NULL || opacity == 0) return;

	if(!tr) {
		clear(rc, clr, opacity, m_surface);
		return;
	}

	smil2::blitter_type bt = tr->get_blitter_type();

	if(bt == smil2::bt_r1r2r3r4) {
		lib::rect rc_v = rc;
		clipto_r1r2r3r4(tr, rc_v, rc_v);
		clear(rc_v, clr, opacity, m_surface);
		return;
	} else if(bt == smil2::bt_fade) {
		IDirectDrawSurface* s1 = create_surface();
		IDirectDrawSurface* s2 = create_surface();
		if(!s1 || !s2) {
			RELEASE(s1);
			RELEASE(s2);
			clear(rc, clr, opacity, m_surface);
			return;
		}
		clear(rc, clr, opacity, s1);
		copy_bgd_to(s2, rc);
		if (blt_blend(s2, s1, rc, tr->get_progress()*opacity, 0xFFFFFFFF, 0, 0xFFFFFFFF))
			draw_to_bgd(s2, rc, 0);
		else
			draw_to_bgd(s1, rc, 0);
		release_surface(s1);
		release_surface(s2);
		return;
	}

	HRGN hrgn = 0;
	switch(bt) {
		case smil2::bt_rect:
			hrgn = create_rect_region(tr);
			break;
		case smil2::bt_rectlist:
			hrgn = create_rectlist_region(tr);
			break;
		case smil2::bt_poly:
			hrgn = create_poly_region(tr);
			break;
		case smil2::bt_polylist:
			hrgn = create_polylist_region(tr);
			break;
	}

	if(!hrgn) {
		clear(rc, clr, opacity, m_surface);
		return;
	} else if(is_empty_region(hrgn)) {
		// nothing to paint
		return;
	}
	IDirectDrawSurface* s1 = create_surface();
	clear(rc, clr, opacity, s1);
	OffsetRgn(hrgn, rc.left(), rc.top());
	draw_to_bgd(s1, rc, hrgn);
	release_surface(s1);
	DeleteObject((HGDIOBJ)hrgn);
}

// Clears the specified surface rectangle using the provided color
void gui::dx::viewport::clear(const lib::rect& rc, lib::color_t clr, double opacity, IDirectDrawSurface* dstview) {
	if(!dstview || opacity == 0.0)
		return;
	HRESULT hr = S_OK;
	DWORD dwFlags = DDBLT_COLORFILL | AM_DDBLT_WAIT;
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = convert(clr);
	RECT dstRC = {rc.left(), rc.top(), rc.right(), rc.bottom()};

	// Verify:
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&dstRC, &dstRC, &vrc) || IsRectEmpty(&dstRC))
		return;
	if (opacity != 1.0) {
		IDirectDrawSurface* colorsurf = create_surface();
		hr = colorsurf->Blt(&dstRC, 0, 0, dwFlags, &bltfx);
		if (SUCCEEDED(hr)) {
			blt_blend(dstview, colorsurf, rc, opacity, 0xFFFFFFFF, 0, 0xFFFFFFFF);
			release_surface(colorsurf);
		} else {
			DDSURFACEDESC ddsd;
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
			HRESULT h = colorsurf->GetSurfaceDesc(&ddsd);
			seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
			lib::logger::get_logger()->debug("h=%d,f=0x%x,w=%d,h=%d",h,ddsd.dwFlags,ddsd.dwWidth);
		}
		} else {
		hr = dstview->Blt(&dstRC, 0, 0, dwFlags, &bltfx);
		if (hr == DDERR_NOTFOUND) return; // XXXJACK
		if (FAILED(hr))
			seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Clears a DD surface with the provided color.
void
gui::dx::viewport::clear_surface(IDirectDrawSurface* p, lib::color_t clr, double opacity) {
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = (clr == dxparams::I()->invalid_color())?m_ddbgd:convert(clr);
	RECT dst_rc;
	set_rect(p, &dst_rc);
	HRESULT hr = p->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | AM_DDBLT_WAIT, &bltfx);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::Blt()", hr);
	}
}

// Draw the whole DD surface to the back buffer and destination rectangle
void
gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& dst_rc, bool keysrc) {
	if(!m_surface || !src) return;
	DWORD flags = AM_DDBLT_WAIT;
	if(keysrc) flags |= DDBLT_KEYSRC;

	// Set srcRC to surf rect
	RECT srcRC;
	set_rect(src, &srcRC);
	RECT dstRC = {dst_rc.left(), dst_rc.top(), dst_rc.right(), dst_rc.bottom()};

	// Verify:
	// Dest within viewport
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&dstRC, &dstRC, &vrc) || IsRectEmpty(&dstRC))
		return;

	HRESULT hr = m_surface->Blt(&dstRC, src, &srcRC, flags, NULL);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// blend 'src' surface into m_surface when pixel color value is within/outside
// [chroma_low, chroma_high] using opacity_in/double opacity_out, resp.
void
gui::dx::viewport::blend_surface(const lib::rect& dst_rc, IDirectDrawSurface* src, const lib::rect& src_rc, bool keysrc, double opacity_in, double opacity_out, lib::color_t chroma_low, lib::color_t chroma_high) {
	IDirectDrawSurface* s1 = create_surface();
	IDirectDrawSurface* s2 = create_surface();
	if(!s1 || !s2) {
		RELEASE(s1);
		RELEASE(s2);
		draw(src, src_rc, dst_rc, keysrc, m_surface);
		return;
	}
	if(keysrc) copy_bgd_to(s1, dst_rc);
	draw(src, src_rc, dst_rc, keysrc, s1);
	copy_bgd_to(s2, dst_rc);
// in the next line,  0x88888888 can be used to test chromakeying
	if (blt_blend(s2, s1, dst_rc, opacity_in, opacity_out, chroma_low, chroma_high))
		draw_to_bgd(s2, dst_rc, 0);
	else
		draw_to_bgd(s1, dst_rc, 0);
	release_surface(s1);
	release_surface(s2);
	return;
}

// blend 'src' surface into 'dst' surface using	 using
// 'opacity_in'/'opacity_out' when the color of the pixel in 'from'
// is inside/outside the range [chroma_low, chroma_high]
void
gui::dx::viewport::blend_surface(IDirectDrawSurface* dst, const lib::rect& dst_rc, IDirectDrawSurface* src, const lib::rect& src_rc, bool keysrc, double opacity_in, double opacity_out, lib::color_t chroma_low, lib::color_t chroma_high) {
	if (dst == NULL || src == NULL)
		return;
	HRESULT hr = S_OK;
	int x_dst = dst_rc.left();
	int y_dst = dst_rc.top();
	int width = dst_rc.width();
	int height = dst_rc.height();
	int x_src = dst_rc.left();
	int y_src = dst_rc.top();
	DWORD raster_op = SRCCOPY;

	IDirectDrawSurface* tmp = create_surface();
	if( ! tmp) {
		RELEASE(tmp);
		draw (src, src_rc, dst_rc, keysrc, dst);
		return;
	}
	// copy dst surface to tmp surface
	draw (dst, dst_rc, dst_rc, keysrc, tmp);
	// blend tmp surface with src surface
	blt_blend(tmp, src, src_rc, opacity_in, opacity_out, chroma_low, chroma_high);
	// copy tmp surface to dst surface
	draw (tmp, dst_rc, dst_rc, false, dst);
	RELEASE(tmp);
}

// copy 'src_rc' rectangle in 'src' surface into 'dst' surface' 'dst_rc' rectangle
void
gui::dx::viewport::copy_surface(IDirectDrawSurface* dst, const lib::rect& dst_rc, IDirectDrawSurface* src, const lib::rect& src_rc) {
	draw (src, src_rc, dst_rc, false, dst);
}

// Draw the src_rc of the DD surface to the back buffer and destination rectangle
void
gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& src_rc,
	const lib::rect& dst_rc, bool keysrc, dx_transition *tr) {
	if(!m_surface || !src) return;

	if(!tr) {
		draw(src, src_rc, dst_rc, keysrc, m_surface);
		return;
	}

	smil2::blitter_type bt = tr->get_blitter_type();

	if(bt == smil2::bt_r1r2r3r4) {
// r.1.40 leads to #1619481
// r.1.39 doesn't have the problem
		lib::rect src_rc_v = src_rc;
		lib::rect dst_rc_v = dst_rc;
		clipto_r1r2r3r4(tr, src_rc_v, dst_rc_v);
		draw(src, src_rc_v, dst_rc_v, keysrc, m_surface);
		return;
	} else if(bt == smil2::bt_fade) {
		blend_surface(dst_rc, src, src_rc, keysrc, tr->get_progress(), 0xFFFFFF, lib::color_t(0x000000), lib::color_t(0xFFFFFF));
		return;
	}

	HRGN hrgn = 0;
	switch(bt) {
		case smil2::bt_rect:
			hrgn = create_rect_region(tr);
			break;
		case smil2::bt_rectlist:
			hrgn = create_rectlist_region(tr);
			break;
		case smil2::bt_poly:
			hrgn = create_poly_region(tr);
			break;
		case smil2::bt_polylist:
			hrgn = create_polylist_region(tr);
			break;
	}

	if(!hrgn) {
		draw(src, src_rc, dst_rc, keysrc, m_surface);
		return;
	} else if(is_empty_region(hrgn)) {
		// nothing to paint
		viewport_logger->trace("%s: Region is empty for transition",
			tr->get_type_str().c_str());
		DeleteObject((HGDIOBJ)hrgn);
		return;
	}

	IDirectDrawSurface* surf = create_surface();
	if(!tr->is_outtrans()) copy_bgd_to(surf, dst_rc);
	draw(src, src_rc, dst_rc, keysrc, surf);
	OffsetRgn(hrgn, dst_rc.left(), dst_rc.top());
	draw_to_bgd(surf, dst_rc, hrgn);
	release_surface(surf);
	DeleteObject((HGDIOBJ)hrgn);
}

void
gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& src_rc,
	const lib::rect& dst_rc, bool keysrc, IDirectDrawSurface* dstview) {
	if(!dstview || !src) return;

	RECT srcRC = {src_rc.left(), src_rc.top(), src_rc.right(), src_rc.bottom()};
	RECT dstRC = {dst_rc.left(), dst_rc.top(), dst_rc.right(), dst_rc.bottom()};

	// Verify:
	// 1. Src within surf
	RECT surfRC;
	set_rect(src, &surfRC);
	if(!IntersectRect(&srcRC, &srcRC, &surfRC) || IsRectEmpty(&srcRC))
		return;

	// 2. Dest within viewport
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&dstRC, &dstRC, &vrc) || IsRectEmpty(&dstRC))
		return;

	DWORD flags = AM_DDBLT_WAIT;
	if(keysrc) flags |= DDBLT_KEYSRC;
	AM_DBG lib::logger::get_logger()->debug("dx_viewport::redraw(0x%x): src=0x%x, flags=0x%x, dstRC(%d,%d,%d,%d), srcRC(%d,%d,%d,%d)", dstview, src, flags, dstRC.top,dstRC.bottom,dstRC.left,dstRC.right,srcRC.top,srcRC.bottom,srcRC.left,srcRC.right);
	HRESULT hr = dstview->Blt(&dstRC, src, &srcRC, flags, NULL);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror(":viewport::draw/DirectDrawSurface::Blt()", hr);
		viewport_logger->trace("Blt %s --> %s failed", repr(src_rc).c_str(), repr(dst_rc).c_str());
	}
}

// Paints the provided string
void
gui::dx::viewport::draw(const std::basic_string<text_char>& text, const lib::rect& rc, lib::color_t clr) {
	if(!m_surface || text.empty()) return;
	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = (clr == dxparams::I()->invalid_color())?::GetSysColor(COLOR_WINDOWTEXT):clr;
	::SetTextColor(hdc, crTextColor);
	RECT dstRC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	int res = ::DrawText(hdc, text.c_str(), int(text.length()), &dstRC, uFormat);
	if(res == 0)
		win_report_last_error("DrawText()");
	m_surface->ReleaseDC(hdc);
}

// Frames the provided rect
void
gui::dx::viewport::frame_rect(const lib::rect& rc, lib::color_t clr) {
	if(!m_surface) return;
	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	HBRUSH hbr = CreateSolidBrush(clr);
	if(FrameRect(hdc, &RC, hbr) == 0)
		win_report_last_error("FrameRect()");
	DeleteObject((HGDIOBJ) hbr);
	m_surface->ReleaseDC(hdc);
}

// Helper, that returns the size of a DD surface
// static
lib::size gui::dx::viewport::get_size(IDirectDrawSurface* p) {
	assert(p);
	DDSURFACEDESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	HRESULT hr = p->GetSurfaceDesc(&desc);
	assert(SUCCEEDED(hr));
	return lib::size(desc.dwWidth, desc.dwHeight);
}

// Draw the src_rc of the DD surface to the back buffer and destination rectangle
void
gui::dx::viewport::blit(IDirectDrawSurface* src, const lib::rect& src_rc,
	IDirectDrawSurface* dst, const lib::rect& dst_rc) {

	RECT srcRC = {src_rc.left(), src_rc.top(), src_rc.right(), src_rc.bottom()};
	RECT dstRC = {dst_rc.left(), dst_rc.top(), dst_rc.right(), dst_rc.bottom()};

	// Verify:
	// 1. Src within surf
	RECT srcSurfRC;
	set_rect(src, &srcSurfRC);
	if(!IntersectRect(&srcRC, &srcRC, &srcSurfRC) || IsRectEmpty(&srcRC))
		return;

	// 2. Dst within surf
	RECT dstSurfRC;
	set_rect(dst, &dstSurfRC);
	if(!IntersectRect(&dstRC, &dstRC, &dstSurfRC) || IsRectEmpty(&dstRC))
		return;

	DWORD flags = AM_DDBLT_WAIT;
	HRESULT hr = dst->Blt(&dstRC, src, &srcRC, flags, NULL);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror("viewport::blit/DirectDrawSurface::Blt()", hr);
		viewport_logger->trace("Blt %s --> %s failed", repr(src_rc).c_str(), repr(dst_rc).c_str());
	}
}

// Copies to the DD surface the back buffer within the from rect
void
gui::dx::viewport::rdraw(IDirectDrawSurface* dst, const lib::rect& from_rc) {
	if(!m_surface || !dst) return;
	DWORD flags = AM_DDBLT_WAIT;

	// Set srcRC to surf rect
	RECT surfRC;
	set_rect(dst, &surfRC);

	RECT fromRC = {from_rc.left(), from_rc.top(), from_rc.right(), from_rc.bottom()};

	// Verify:
	// Dest within viewport
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&fromRC, &fromRC, &vrc) || IsRectEmpty(&fromRC))
		return;

	HRESULT hr = dst->Blt(&surfRC, m_surface, &fromRC, flags, NULL);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror("viewport::rdraw/DirectDrawSurface::Blt()", hr);
	}
}

void
gui::dx::viewport::copy_bgd_to(IDirectDrawSurface* surf, const lib::rect& rc) {
	if(!m_surface || !surf) return;
	DWORD flags = AM_DDBLT_WAIT;
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&RC, &RC, &vrc) || IsRectEmpty(&RC)) return;
	HRESULT hr = surf->Blt(&RC, m_surface, &RC, flags, NULL);
	if (hr == DDERR_NOTFOUND) return; // XXXJACK
	if (FAILED(hr)) {
		seterror("viewport::copy/DirectDrawSurface::Blt()", hr);
	}
}

void
gui::dx::viewport::draw_to_bgd(IDirectDrawSurface* surf, const lib::rect& rc, HRGN hrgn) {
	if(!m_surface) return;
	DWORD flags = AM_DDBLT_WAIT;
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&RC, &RC, &vrc) || IsRectEmpty(&RC)) return;

	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	HDC hsurfdc;
	hr = surf->GetDC(&hsurfdc);
	if(FAILED(hr)) {
		m_surface->ReleaseDC(hdc);
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}

	if(hrgn)
		SelectClipRgn(hdc, hrgn);

	int w = RC.right - RC.left;
	int h = RC.bottom - RC.top;
	BOOL res = BitBlt(hdc, RC.left, RC.top, w, h, hsurfdc, RC.left, RC.top, SRCCOPY);
	if(!res) win_report_last_error("BitBlt");

	m_surface->ReleaseDC(hdc);
	surf->ReleaseDC(hsurfdc);
}

////////////////////////
// Internal pixel format related functions

uint16
gui::dx::viewport::low_bit_pos(uint32 dword) {
	uint32 test = 1;
	for(uint16 i=0;i<32;i++){
		if(dword & test)
			return i;
		test <<= 1;
	}
	return 0;
}

uint16
gui::dx::viewport::high_bit_pos(uint32 dword) {
	uint32 test = 1;
	test <<= 31;
	for(uint16 i=0;i<32;i++){
		if ( dword & test )
			return (uint16)(31-i);
		test >>= 1;
	}
	return 0;
}

void
gui::dx::viewport::get_pixel_format() {
	if(!m_primary_surface) return;

	DDPIXELFORMAT format;
	memset(&format, 0, sizeof(format));
	format.dwSize = sizeof(format);
	HRESULT hr = m_primary_surface->GetPixelFormat(&format);
	if (FAILED(hr)){
		seterror("DirectDrawSurface::GetPixelFormat()", hr);
		return;
	}
	bits_size = format.dwRGBBitCount;
	//viewport_logger->trace("bits_size: %u", bits_size);
	lo_red_bit = low_bit_pos( format.dwRBitMask );
	uint16 hi_red_bit = high_bit_pos( format.dwRBitMask );
	red_bits = (uint16)(hi_red_bit-lo_red_bit+1);

	lo_green_bit = low_bit_pos( format.dwGBitMask );
	uint16 hi_green_bit = high_bit_pos(format.dwGBitMask);
	green_bits=(uint16)(hi_green_bit-lo_green_bit+1);

	lo_blue_bit	 = low_bit_pos( format.dwBBitMask );
	uint16 hi_blue_bit	= high_bit_pos(format.dwBBitMask);
	blue_bits=(uint16)(hi_blue_bit-lo_blue_bit+1);
}

// Converts a lib::color_t to a DD color
uint32
gui::dx::viewport::convert(lib::color_t color) {
	return convert(lib::redc(color), lib::greenc(color), lib::bluec(color));
}

// Converts the RGB tuple to a DD color
uint32
gui::dx::viewport::convert(BYTE r, BYTE g, BYTE b) {
	uint32 ddcolor = 0;
	if(bits_size == 8){
		// find from palette
		return 0;
	} else if(bits_size == 16) {
		int rs = 8 - red_bits;
		int gs = 8 - green_bits;
		int bs = 8 - blue_bits;
		ddcolor = ((r >> rs) << lo_red_bit) | ((g>>gs) << lo_green_bit) | ((b>>bs) << lo_blue_bit);
	} else if (bits_size==24 || bits_size==32){
		ddcolor = (r << lo_red_bit) | (g << lo_green_bit) | (b << lo_blue_bit);
	}
	return ddcolor;
}

// Internal function
// Converts the provided rect to OS screen coordinates
RECT*
gui::dx::viewport::to_screen_rc_ptr(RECT& r) {
	POINT pt = {r.left, r.top};
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	ClientToScreen(m_hwnd, &pt);
	r.left = pt.x;
	r.top = pt.y;
	r.right = pt.x + w;
	r.bottom = pt.y + h;
	return &r;
}

__forceinline int blend(int w, int c1, int c2) {return (c1==c2)?c1:(c1 + w*(c2-c1)/256); }

void
gui::dx::viewport::get_low_high_values(uint32 low_ddclr, uint32 high_ddclr, BYTE* r_l, BYTE* r_h, BYTE* g_l, BYTE* g_h, BYTE* b_l, BYTE* b_h) {
	*r_h = (BYTE)((high_ddclr&0xFF0000) >> 16);
	*r_l = (BYTE)((low_ddclr&0xFF0000) >> 16);
	*g_h = (BYTE)((high_ddclr&0xFF00) >> 8);
	*g_l = (BYTE)((low_ddclr&0xFF00) >> 8);
	*b_h = (BYTE)(high_ddclr&0xFF);
	*b_l = (BYTE)(low_ddclr&0xFF);
}

HRESULT
gui::dx::viewport::blt_blend32(const lib::rect& rc,
	double opacity_in, double opacity_out,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2,
	uint32 low_ddclr, uint32 high_ddclr) {

	DDSURFACEDESC desc1, desc2;
	ZeroMemory(&desc1, sizeof(desc1));
	desc1.dwSize = sizeof(desc1);
	ZeroMemory(&desc2, sizeof(desc2));
	desc2.dwSize = sizeof(desc2);

	HRESULT hr = surf1->Lock(0, &desc1, AM_DDLOCK_WAIT | DDLOCK_READONLY, 0);
	if(hr!=DD_OK) return hr;

	hr = surf2->Lock(0, &desc2, AM_DDLOCK_WAIT, 0);
	if(hr != DD_OK) {
		surf1->Unlock(0);
		return hr;
	}

	lib::rect rcv(lib::point(0,0), lib::size(m_width,m_height));
	lib::rect rcc = rc;
	rcc &= rcv;
	int begin_row = rcc.bottom();
	int end_row = rcc.top();
	int begin_col = rcc.left();
	int end_col = rcc.right();
	BYTE r_l, r_h, g_l, g_h, b_l, b_h;
	get_low_high_values(low_ddclr, high_ddclr, &r_l, &r_h, &g_l, &g_h, &b_l, &b_h);

	int weight_in = int(opacity_in*256);
	int weight_out = int(opacity_out*256);
	for(int row = begin_row-1;row>=end_row;row--) {
		RGBQUAD* px1 = (RGBQUAD*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		RGBQUAD* px2 = (RGBQUAD*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=	begin_col;
		px2 +=	begin_col;
		AM_DBG if (row == end_row) lib::logger::get_logger()->debug("px1=0x%x px2=0x%x lox=0x%x high=0x%x",*px1,*px2, low_ddclr, high_ddclr);
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			if (px1->rgbRed >= r_l && px1->rgbRed <= r_h
				&& px1->rgbGreen >= g_l && px1->rgbGreen <= g_h
				&& px1->rgbBlue >= b_l && px1->rgbBlue <= b_h)
			{
				px2->rgbRed = (BYTE)blend(weight_in, px2->rgbRed, px1->rgbRed);
				px2->rgbGreen = (BYTE)blend(weight_in, px2->rgbGreen, px1->rgbGreen);
				px2->rgbBlue = (BYTE)blend(weight_in, px2->rgbBlue, px1->rgbBlue);
			} else {
				px2->rgbRed = (BYTE)blend(weight_out, px2->rgbRed, px1->rgbRed);
				px2->rgbGreen = (BYTE)blend(weight_out, px2->rgbGreen, px1->rgbGreen);
				px2->rgbBlue = (BYTE)blend(weight_out, px2->rgbBlue, px1->rgbBlue);
			}
			AM_DBG	if (row == end_row && col == begin_col) lib::logger::get_logger()->debug("px2=0x%x weight_in=%d weight_out=%d",*px2, weight_in,weight_out);
		}
	}
	surf1->Unlock(0);
	surf2->Unlock(0);
	return hr;
}

HRESULT
gui::dx::viewport::blt_blend24(const lib::rect& rc,
	double opacity_in, double opacity_out,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2,
	uint32 low_ddclr, uint32 high_ddclr) {

	DDSURFACEDESC desc1, desc2;
	ZeroMemory(&desc1, sizeof(desc1));
	desc1.dwSize = sizeof(desc1);
	ZeroMemory(&desc2, sizeof(desc2));
	desc2.dwSize = sizeof(desc2);

	HRESULT hr = surf1->Lock(0, &desc1, AM_DDLOCK_WAIT | DDLOCK_READONLY, 0);
	if(hr!=DD_OK) return hr;

	hr = surf2->Lock(0, &desc2, AM_DDLOCK_WAIT, 0);
	if(hr != DD_OK) {
		surf1->Unlock(0);
		return hr;
	}

	lib::rect rcv(lib::point(0,0), lib::size(m_width,m_height));
	lib::rect rcc = rc;
	rcc &= rcv;
	int begin_row = rcc.bottom();
	int end_row = rcc.top();
	int begin_col = rcc.left();
	int end_col = rcc.right();

	BYTE r_l, r_h, g_l, g_h, b_l, b_h;
	get_low_high_values(low_ddclr, high_ddclr, &r_l, &r_h, &g_l, &g_h, &b_l, &b_h);

	int weight_in = int(opacity_in*256);
	int weight_out = int(opacity_out*256);
	for(int row = begin_row-1;row>=end_row;row--) {
		RGBTRIPLE* px1 = (RGBTRIPLE*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		RGBTRIPLE* px2 = (RGBTRIPLE*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=	begin_col;
		px2 +=	begin_col;
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			if (px1->rgbtRed >= r_l && px1->rgbtRed <= r_h
				&& px1->rgbtGreen >= g_l && px1->rgbtGreen <= g_h
				&& px1->rgbtBlue >= b_l && px1->rgbtBlue <= b_h)
			{
				px2->rgbtRed = (BYTE)blend(weight_in, px2->rgbtRed, px1->rgbtRed);
				px2->rgbtGreen = (BYTE)blend(weight_in, px2->rgbtGreen, px1->rgbtGreen);
				px2->rgbtBlue = (BYTE)blend(weight_in, px2->rgbtBlue, px1->rgbtBlue);
			} else {
				px2->rgbtRed = (BYTE)blend(weight_out, px2->rgbtRed, px1->rgbtRed);
				px2->rgbtGreen = (BYTE)blend(weight_out, px2->rgbtGreen, px1->rgbtGreen);
				px2->rgbtBlue = (BYTE)blend(weight_out, px2->rgbtBlue, px1->rgbtBlue);
			}
		}
	}
	surf1->Unlock(0);
	surf2->Unlock(0);
	return hr;
}

struct trible565 {
	// bit format 16 bit bgr: bbbbbggggggrrrrr
	uint16 v;
	trible565() : v(0) {}
	trible565(int _r, int _g, int _b)  {
		lib::color_t bgr = (_b << 16) | (_g << 8) | _r ;
		v = (uint16)((bgr & 0xf80000)>> 8);
		v |= (uint16)(bgr & 0xfc00) >> 5;
		v |= (uint16)(bgr & 0xf8) >> 3;
	}
	trible565(uchar _r, uchar _g, uchar _b) {
		lib::color_t bgr = (_b << 16) | (_g << 8) | _r ;
		v = (uint16)((bgr & 0xf80000)>> 8); // blue
		v |= (uint16)(bgr & 0xfc00) >> 5; // green
		v |= (uint16)(bgr & 0xf8) >> 3; // red
	}

	trible565(lib::color_t bgr) {
		v = (uint16)((bgr & 0xf80000) >> 8);
		v |= (uint16)(bgr & 0xfc00) >> 5;
		v |= (uint16)(bgr & 0xf8) >> 3;
	}
	// mult and div used to ensure image doesn't become slightly dark
	BYTE blue() { return (((v & 0xf800) >> 11)*255)/31;}
	BYTE green() { return (((v & 0x7e0) >> 5)*255)/63;}
	BYTE red() { return ((v & 0x1f)*255)/31;}
};

HRESULT
gui::dx::viewport::blt_blend16(const lib::rect& rc,
	double opacity_in, double opacity_out,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2,
	uint32 low_ddclr, uint32 high_ddclr) {

	DDSURFACEDESC desc1, desc2;
	ZeroMemory(&desc1, sizeof(desc1));
	desc1.dwSize = sizeof(desc1);
	ZeroMemory(&desc2, sizeof(desc2));
	desc2.dwSize = sizeof(desc2);

	HRESULT hr = surf1->Lock(0, &desc1, AM_DDLOCK_WAIT | DDLOCK_READONLY, 0);
	if(hr!=DD_OK) return hr;

	hr = surf2->Lock(0, &desc2, AM_DDLOCK_WAIT, 0);
	if(hr != DD_OK) {
		surf1->Unlock(0);
		return hr;
	}

	lib::rect rcv(lib::point(0,0), lib::size(m_width,m_height));
	lib::rect rcc = rc;
	rcc &= rcv;
	int begin_row = rcc.bottom();
	int end_row = rcc.top();
	int begin_col = rcc.left();
	int end_col = rcc.right();

	BYTE r_l, r_h, g_l, g_h, b_l, b_h;
	get_low_high_values(low_ddclr, high_ddclr, &r_l, &r_h, &g_l, &g_h, &b_l, &b_h);

	int weight_in = int(opacity_in*256);
	int weight_out = int(opacity_out*256);
	for(int row = begin_row-1;row>=end_row;row--) {
		trible565* px1 = (trible565*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		trible565* px2 = (trible565*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=	begin_col;
		px2 +=	begin_col;
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			BYTE r = px1->red(), g = px1->green(), b = px1->blue();
			if (r >= r_l && r <= r_h && g >= g_l && g <= g_h && b >= b_l && b <= b_h) {
				r = (BYTE)blend(weight_in, px2->red(), r);
				g = (BYTE)blend(weight_in, px2->green(), g);
				b = (BYTE)blend(weight_in, px2->blue(), b);
			} else {
				r = (BYTE)blend(weight_out, px2->red(), r);
				g = (BYTE)blend(weight_out, px2->green(), g);
				b = (BYTE)blend(weight_out, px2->blue(), b);
			}
			*px2 = trible565(r, g, b);
		}
	}
	surf1->Unlock(0);
	surf2->Unlock(0);
	return hr;
}
