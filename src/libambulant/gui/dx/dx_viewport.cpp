// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$ 
 */


#include "ambulant/config/config.h"
#ifndef AMBULANT_PLATFORM_WIN32_WCE
#define INITGUID
#endif
#include <objbase.h>
#ifdef AMBULANT_DDRAW_EX
#include <ddrawex.h>
#else
#include <ddraw.h>
#endif
#include <uuids.h>
#include <windows.h>
#include <mmsystem.h>

#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_audio_player.h" // Only to define the TPB GUID


#ifndef AMBULANT_PLATFORM_WIN32_WCE
#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"dxguid.lib")
#endif
#pragma comment (lib,"ddraw.lib")

#include "ambulant/lib/logger.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
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

static struct error {
	HRESULT hr;
	char *name;
} errorlist [] = {
#ifndef AMBULANT_PLATFORM_WIN32_WCE
	{DDERR_ALREADYINITIALIZED, "DDERR_ALREADYINITIALIZED"},
	{DDERR_CANNOTATTACHSURFACE,"DDERR_CANNOTATTACHSURFACE"},
	{DDERR_CANNOTDETACHSURFACE,"DDERR_CANNOTDETACHSURFACE"},
	{DDERR_CURRENTLYNOTAVAIL,"DDERR_CURRENTLYNOTAVAIL"},
	{DDERR_EXCEPTION,"DDERR_EXCEPTION"},
	{DDERR_GENERIC,"DDERR_GENERIC"},
	{DDERR_HEIGHTALIGN,"DDERR_HEIGHTALIGN"},
	{DDERR_INCOMPATIBLEPRIMARY, "DDERR_INCOMPATIBLEPRIMARY"},
	{DDERR_INVALIDCAPS, "DDERR_INVALIDCAPS"},
	{DDERR_INVALIDCLIPLIST, "DDERR_INVALIDCLIPLIST"},
	{DDERR_INVALIDMODE, "DDERR_INVALIDMODE"},
	{DDERR_INVALIDOBJECT, "DDERR_INVALIDOBJECT"},
	{DDERR_INVALIDPARAMS, "DDERR_INVALIDPARAMS"},
	{DDERR_INVALIDPIXELFORMAT, "DDERR_INVALIDPIXELFORMAT"},
	{DDERR_INVALIDRECT, "DDERR_INVALIDRECT"},
	{DDERR_LOCKEDSURFACES, "DDERR_LOCKEDSURFACES"},
	{DDERR_NO3D, "DDERR_NO3D"},
	{DDERR_NOALPHAHW, "DDERR_NOALPHAHW"},
#ifdef DDERR_NOSTEREOHARDWARE
	{DDERR_NOSTEREOHARDWARE, "DDERR_NOSTEREOHARDWARE"},
#endif
#ifdef DDERR_NOSURFACELEFT
	{DDERR_NOSURFACELEFT, "DDERR_NOSURFACELEFT"},
#endif
	{DDERR_NOCLIPLIST, "DDERR_NOCLIPLIST"},
	{DDERR_NOCOLORCONVHW, "DDERR_NOCOLORCONVHW"},
	{DDERR_NOCOOPERATIVELEVELSET, "DDERR_NOCOOPERATIVELEVELSET"},
	{DDERR_NOCOLORKEY, "DDERR_NOCOLORKEY"},
	{DDERR_NOCOLORKEYHW, "DDERR_NOCOLORKEYHW"},
	{DDERR_NODIRECTDRAWSUPPORT, "DDERR_NODIRECTDRAWSUPPORT"},
	{DDERR_NOEXCLUSIVEMODE, "DDERR_NOEXCLUSIVEMODE"},
	{DDERR_NOFLIPHW, "DDERR_NOFLIPHW"},
	{DDERR_NOGDI, "DDERR_NOGDI"},
	{DDERR_NOMIRRORHW, "DDERR_NOMIRRORHW"},
	{DDERR_NOTFOUND, "DDERR_NOTFOUND"},
	{DDERR_NOOVERLAYHW, "DDERR_NOOVERLAYHW"},
#ifdef DDERR_OVERLAPPINGRECTS
	{DDERR_OVERLAPPINGRECTS, "DDERR_OVERLAPPINGRECTS"},
#endif
	{DDERR_NORASTEROPHW, "DDERR_NORASTEROPHW"},
	{DDERR_NOROTATIONHW, "DDERR_NOROTATIONHW"},
	{DDERR_NOSTRETCHHW, "DDERR_NOSTRETCHHW"},
	{DDERR_NOT4BITCOLOR, "DDERR_NOT4BITCOLOR"},
	{DDERR_NOT4BITCOLORINDEX, "DDERR_NOT4BITCOLORINDEX"},
	{DDERR_NOT8BITCOLOR, "DDERR_NOT8BITCOLOR"},
	{DDERR_NOTEXTUREHW, "DDERR_NOTEXTUREHW"},
	{DDERR_NOVSYNCHW, "DDERR_NOVSYNCHW"},
	{DDERR_NOZBUFFERHW, "DDERR_NOZBUFFERHW"},
	{DDERR_NOZOVERLAYHW, "DDERR_NOZOVERLAYHW"},
	{DDERR_OUTOFCAPS, "DDERR_OUTOFCAPS"},
	{DDERR_OUTOFMEMORY, "DDERR_OUTOFMEMORY"},
	{DDERR_OUTOFVIDEOMEMORY, "DDERR_OUTOFVIDEOMEMORY"},
	{DDERR_OVERLAYCANTCLIP, "DDERR_OVERLAYCANTCLIP"},
	{DDERR_OVERLAYCOLORKEYONLYONEACTIVE, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"},
	{DDERR_PALETTEBUSY, "DDERR_PALETTEBUSY"},
	{DDERR_COLORKEYNOTSET, "DDERR_COLORKEYNOTSET"},
	{DDERR_SURFACEALREADYATTACHED, "DDERR_SURFACEALREADYATTACHED"},
	{DDERR_SURFACEALREADYDEPENDENT, "DDERR_SURFACEALREADYDEPENDENT"},
	{DDERR_SURFACEBUSY, "DDERR_SURFACEBUSY"},
	{DDERR_CANTLOCKSURFACE, "DDERR_CANTLOCKSURFACE"},
	{DDERR_SURFACEISOBSCURED, "DDERR_SURFACEISOBSCURED"},
	{DDERR_SURFACELOST, "DDERR_SURFACELOST"},
	{DDERR_SURFACENOTATTACHED, "DDERR_SURFACENOTATTACHED"},
	{DDERR_TOOBIGHEIGHT, "DDERR_TOOBIGHEIGHT"},
	{DDERR_TOOBIGSIZE, "DDERR_TOOBIGSIZE"},
	{DDERR_TOOBIGWIDTH, "DDERR_TOOBIGWIDTH"},
	{DDERR_UNSUPPORTED, "DDERR_UNSUPPORTED"},
	{DDERR_UNSUPPORTEDFORMAT, "DDERR_UNSUPPORTEDFORMAT"},
	{DDERR_UNSUPPORTEDMASK, "DDERR_UNSUPPORTEDMASK"},
#ifdef DDERR_INVALIDSTREAM
	{DDERR_INVALIDSTREAM, "DDERR_INVALIDSTREAM"},
#endif
	{DDERR_VERTICALBLANKINPROGRESS, "DDERR_VERTICALBLANKINPROGRESS"},
	{DDERR_WASSTILLDRAWING, "DDERR_WASSTILLDRAWING"},
#ifdef DDERR_DDSCAPSCOMPLEXREQUIRED
	{DDERR_DDSCAPSCOMPLEXREQUIRED, "DDERR_DDSCAPSCOMPLEXREQUIRED"},
#endif
	{DDERR_XALIGN, "DDERR_XALIGN"},
	{DDERR_INVALIDDIRECTDRAWGUID, "DDERR_INVALIDDIRECTDRAWGUID"},
	{DDERR_DIRECTDRAWALREADYCREATED, "DDERR_DIRECTDRAWALREADYCREATED"},
	{DDERR_NODIRECTDRAWHW, "DDERR_NODIRECTDRAWHW"},
	{DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS"},
	{DDERR_NOEMULATION, "DDERR_NOEMULATION"},
	{DDERR_REGIONTOOSMALL, "DDERR_REGIONTOOSMALL"},
	{DDERR_CLIPPERISUSINGHWND, "DDERR_CLIPPERISUSINGHWND"},
	{DDERR_NOCLIPPERATTACHED, "DDERR_NOCLIPPERATTACHED"},
	{DDERR_NOHWND, "DDERR_NOHWND"},
	{DDERR_HWNDSUBCLASSED, "DDERR_HWNDSUBCLASSED"},
	{DDERR_HWNDALREADYSET, "DDERR_HWNDALREADYSET"},
	{DDERR_NOPALETTEATTACHED, "DDERR_NOPALETTEATTACHED"},
	{DDERR_NOPALETTEHW, "DDERR_NOPALETTEHW"},
	{DDERR_BLTFASTCANTCLIP, "DDERR_BLTFASTCANTCLIP"},
	{DDERR_NOBLTHW, "DDERR_NOBLTHW"},
	{DDERR_NODDROPSHW, "DDERR_NODDROPSHW"},
	{DDERR_OVERLAYNOTVISIBLE, "DDERR_OVERLAYNOTVISIBLE"},
	{DDERR_NOOVERLAYDEST, "DDERR_NOOVERLAYDEST"},
	{DDERR_INVALIDPOSITION, "DDERR_INVALIDPOSITION"},
	{DDERR_NOTAOVERLAYSURFACE, "DDERR_NOTAOVERLAYSURFACE"},
	{DDERR_EXCLUSIVEMODEALREADYSET, "DDERR_EXCLUSIVEMODEALREADYSET"},
	{DDERR_NOTFLIPPABLE, "DDERR_NOTFLIPPABLE"},
	{DDERR_CANTDUPLICATE, "DDERR_CANTDUPLICATE"},
	{DDERR_NOTLOCKED, "DDERR_NOTLOCKED"},
	{DDERR_CANTCREATEDC, "DDERR_CANTCREATEDC"},
	{DDERR_NODC, "DDERR_NODC"},
	{DDERR_WRONGMODE, "DDERR_WRONGMODE"},
	{DDERR_IMPLICITLYCREATED, "DDERR_IMPLICITLYCREATED"},
	{DDERR_NOTPALETTIZED, "DDERR_NOTPALETTIZED"},
	{DDERR_UNSUPPORTEDMODE, "DDERR_UNSUPPORTEDMODE"},
	{DDERR_NOMIPMAPHW, "DDERR_NOMIPMAPHW"},
	{DDERR_INVALIDSURFACETYPE, "DDERR_INVALIDSURFACETYPE"},
	{DDERR_NOOPTIMIZEHW, "DDERR_NOOPTIMIZEHW"},
	{DDERR_NOTLOADED, "DDERR_NOTLOADED"},
	{DDERR_NOFOCUSWINDOW, "DDERR_NOFOCUSWINDOW"},
#ifdef DDERR_NOTONMIPMAPSUBLEVEL
	{DDERR_NOTONMIPMAPSUBLEVEL, "DDERR_NOTONMIPMAPSUBLEVEL"},
#endif
	{DDERR_DCALREADYCREATED, "DDERR_DCALREADYCREATED"},
	{DDERR_NONONLOCALVIDMEM, "DDERR_NONONLOCALVIDMEM"},
	{DDERR_CANTPAGELOCK, "DDERR_CANTPAGELOCK"},
	{DDERR_CANTPAGEUNLOCK, "DDERR_CANTPAGEUNLOCK"},
	{DDERR_NOTPAGELOCKED, "DDERR_NOTPAGELOCKED"},
	{DDERR_MOREDATA, "DDERR_MOREDATA"},
#ifdef DDERR_EXPIRED
	{DDERR_EXPIRED, "DDERR_EXPIRED"},
#endif
#ifdef DDERR_TESTFINISHED
	{DDERR_TESTFINISHED, "DDERR_TESTFINISHED"},
#endif
#ifdef DDERR_NEWMODE
	{DDERR_NEWMODE, "DDERR_NEWMODE"},
#endif
#ifdef DDERR_D3DNOTINITIALIZED
	{DDERR_D3DNOTINITIALIZED, "DDERR_D3DNOTINITIALIZED"},
#endif
	{DDERR_VIDEONOTACTIVE, "DDERR_VIDEONOTACTIVE"},
#ifdef DDERR_NOMONITORINFORMATION
	{DDERR_NOMONITORINFORMATION, "DDERR_NOMONITORINFORMATION"},
#endif
#ifdef DDERR_NODRIVERSUPPORT
	{DDERR_NODRIVERSUPPORT, "DDERR_NODRIVERSUPPORT"},
#endif
	{DDERR_DEVICEDOESNTOWNSURFACE, "DDERR_DEVICEDOESNTOWNSURFACE"},
	{DDERR_NOTINITIALIZED, "DDERR_NOTINITIALIZED"},
#else
	{0, "null"},
#endif
};

#define RELEASE(x) if(x) x->Release();x=NULL;

static lib::logger* viewport_logger = NULL;

static void
seterror(const char *funcname, HRESULT hr){
	char* pszmsg;
	FormatMessage( 
		 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,
		 hr,
		 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		 (LPTSTR) &pszmsg,
		 0,
		 NULL 
		);
	for(error *p = errorlist; p->name; p++)
		if (p->hr == hr){
			viewport_logger->error("%s failed, error = %s (0x%x), %s", funcname, p->name, hr, pszmsg);
			LocalFree(pszmsg);
			return;
		}
	viewport_logger->error( "%s failed, error = %x, %s", funcname, hr, pszmsg);
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
		hr = primary_surface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFX);
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

const lib::color_t CLR_DEFAULT = RGB(255, 255, 255);

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
	m_bgd(CLR_DEFAULT) {
	
	viewport_logger = lib::logger::get_logger();
	
#if 0 // VS8
	IDirectDrawFactory *pDDF = NULL;
    HRESULT hr = CoCreateInstance(CLSID_DirectDrawFactory,
                              NULL, CLSCTX_INPROC_SERVER,
                              IID_IDirectDrawFactory,
                              (void **)&pDDF);
	if (FAILED(hr)){
		seterror("CoCreateInstance(CLSID_DirectDrawFactory, ...)", hr);
		return;
	}	
	IDirectDraw  *pDD1=NULL;
	hr = pDDF->CreateDirectDraw(NULL, m_hwnd, DDSCL_NORMAL , 0, NULL, &pDD1);
	pDDF->Release();
#else
	HRESULT hr;
	IDirectDraw  *pDD1=NULL;
	hr = DirectDrawCreate(NULL, &pDD1, NULL);
#endif

	if (FAILED(hr)){
		seterror("CreateDirectDraw()", hr);
		return;
	}
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	// XXXJACK: Guessing here that we don't have DD1 on WM5 and get a DD2 surface directly.
	m_direct_draw = pDD1;
#else
	hr = pDD1->QueryInterface(IID_IDirectDraw2, (void**)&m_direct_draw);
	if (FAILED(hr)){
		seterror("QueryInterface(IID_IDirectDraw2,...)", hr);
		pDD1->Release();
		return;
	}
	pDD1->Release();
#endif // AMBULANT_PLATFORM_WIN32_WCE
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
		clipper->SetHWnd(0, m_hwnd);
		hr = m_primary_surface->SetClipper(clipper);
		if (FAILED(hr))
			seterror("DirectDrawSurface::SetClipper()", hr);
		clipper->Release();
	}
	
	// create drawing surface
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
#ifdef DDSCAPS_OFFSCREENPLAIN
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
#else
	// XXXJACK: for WinCE. Maybe we need DDSCAPS_BACKBUFFER or something?
	sd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
#endif
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
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
#ifdef DDSCAPS_OFFSCREENPLAIN
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
#else
	// XXXJACK: for WinCE. Maybe we need DDSCAPS_BACKBUFFER or something?
	sd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
#endif
	sd.dwWidth = m_width;
	sd.dwHeight = m_height;
	hr = m_direct_draw->CreateSurface(&sd, &surf, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return;
	}
	m_surfaces.push_back(surf);
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
#ifdef DDSCAPS_OFFSCREENPLAIN
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
#else
	// XXXJACK: for WinCE. Maybe we need DDSCAPS_BACKBUFFER or something?
	sd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
#endif
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
void gui::dx::viewport::set_background(lib::color_t color) {
	m_bgd = (color == CLR_INVALID)?CLR_DEFAULT:color;
	m_ddbgd = convert(m_bgd);
}

// Creates a DD surface with the provided size.
// The surface is cleared using the specified color
IDirectDrawSurface* 
gui::dx::viewport::create_surface(DWORD w, DWORD h) {
	IDirectDrawSurface* surface = 0;
	DDSURFACEDESC sd;
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
#ifdef DDSCAPS_OFFSCREENPLAIN
	sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
#else
	// XXXJACK: for WinCE. Maybe we need DDSCAPS_BACKBUFFER or something?
	sd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
#endif
	sd.dwWidth = w;
	sd.dwHeight = h;
	HRESULT hr = m_direct_draw->CreateSurface(&sd, &surface, NULL);
	if (FAILED(hr)){
		seterror("DirectDraw::CreateSurface()", hr);
		return 0;
	}	
	return surface;
}

IDirectDrawSurface* gui::dx::viewport::create_surface() {
	IDirectDrawSurface* surf = 0;
	if(m_surfaces.empty()) {
		return create_surface(m_width, m_height);
	}
	std::list<IDirectDrawSurface*>::iterator it = m_surfaces.begin();
	surf = *it;
	m_surfaces.erase(it);
	return surf;
}

void gui::dx::viewport::release_surface(IDirectDrawSurface* surf) {
	m_surfaces.push_back(surf);
}

// Blt back buffer to primary surface
void gui::dx::viewport::redraw() {
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
			if ( ! blt_blend(tmps, s2, ourrect, m_fstransition->get_progress(), 0, 0xFFFFFFFF))
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
		primary_Blt(m_primary_surface, to_screen_rc_ptr(dst_rc), tmps, &src_rc, flags, NULL);
		release_surface(tmps);
	} else {
		// Copy to screen
		primary_Blt(m_primary_surface, to_screen_rc_ptr(dst_rc), m_surface, &src_rc, flags, NULL);
		// Copy to backing store for posible fs transition later
		HRESULT hr = m_fstr_surface->Blt(&src_rc, m_surface, &src_rc, flags, NULL);
		if (FAILED(hr)) {
			seterror("viewport::redraw()/DirectDrawSurface::Blt() m_fstr_surface", hr);
		}
	}
}

void gui::dx::viewport::redraw(const lib::rect& rc) {
	if(!m_primary_surface || !m_surface)
		return;
	RECT src_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	RECT dst_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	
	// Convert dst to screen coordinates
	to_screen_rc_ptr(dst_rc);
	
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
			if ( ! blt_blend(tmps, s2, ourrect, m_fstransition->get_progress(), 0, 0xFFFFFFFF))
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
		primary_Blt(m_primary_surface, &dst_rc, tmps, &src_rc, flags, NULL);
	} else {
		// Copy to screen
		primary_Blt(m_primary_surface, &dst_rc, m_surface, &src_rc, flags, NULL);
		// Copy to backing store, for later use with transition
		// XXX Or should we copy the whole surface?
		HRESULT hr = m_fstr_surface->Blt(&src_rc, m_surface, &src_rc, flags, NULL);
		if (FAILED(hr)) {
			seterror("viewport::redraw()/DirectDrawSurface::Blt()", hr);
		}
	}
}

void gui::dx::viewport::schedule_redraw() {
#ifdef DO_REDRAW_WITH_EVENTS
	::InvalidateRect(m_hwnd, NULL, 0);
#else
	redraw();
#endif // DO_REDRAW_WITHOUT_EVENTS
}
void gui::dx::viewport::schedule_redraw(const lib::rect& rc) {
#ifdef DO_REDRAW_WITH_EVENTS
	RECT src_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	::InvalidateRect(m_hwnd, &src_rc, 0);
#else
	redraw(rc);
#endif // DO_REDRAW_WITHOUT_EVENTS
}

// Clears the back buffer using this viewport bgd color
void gui::dx::viewport::clear() {
	if(!m_surface) return;
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = m_ddbgd; 
	RECT dst_rc = {0, 0, m_width, m_height};
	HRESULT hr = m_surface->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | AM_DDBLT_WAIT, &bltfx);
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

bool gui::dx::viewport::blt_blend (IDirectDrawSurface* to, IDirectDrawSurface* from, const lib::rect& rc, double opacity, lib::color_t low_chroma, lib::color_t high_chroma) {
	bool rv = true;
	uint32 low_ddclr = low_chroma,  high_ddclr = high_chroma;
	HRESULT hr = S_OK;
	if (bits_size == 32) {
		hr = blt_blend32(rc, opacity, from, to, low_ddclr, high_ddclr);
	} else if( bits_size == 24) {
		hr = blt_blend24(rc, opacity, from, to, low_ddclr, high_ddclr);
	} else if (bits_size == 16) {
		hr = blt_blend16(rc, opacity, from, to, low_ddclr, high_ddclr);
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
void gui::dx::viewport::clear(const lib::rect& rc, lib::color_t clr, double opacity, dx_transition *tr) {
	if(!m_surface) return;
	
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
		if (blt_blend(s2, s1, rc, tr->get_progress()*opacity, 0, 0xFFFFFFFF))
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
#ifdef XXXX
		// this code should just work, but since alpha blending is currently
		// ignored by DirectX, we just do it pixel by pixel in blt_blend()
 		dwFlags |= DDBLT_ALPHASRCCONSTOVERRIDE;
		bltfx.dwAlphaSrcConstBitDepth = 8;
		bltfx.dwAlphaSrcConst = opacity*255.0;
#endif//XXXX
		IDirectDrawSurface* colorsurf = create_surface();
		hr = colorsurf->Blt(&dstRC, 0, 0, dwFlags, &bltfx);
		if ( ! FAILED(hr)) {
			bltfx.dwFillColor = convert(0); // black
			blt_blend(dstview, colorsurf, rc, opacity, 0, 0xFFFFFFFF);
			release_surface(colorsurf);
		}
	} else {
		hr = dstview->Blt(&dstRC, 0, 0, dwFlags, &bltfx);
	if (FAILED(hr))
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Clears a DD surface with the provided color.
void gui::dx::viewport::clear_surface(IDirectDrawSurface* p, lib::color_t clr, double opacity) {
	DDSURFACEDESC sd;
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = (clr == CLR_INVALID)?m_ddbgd:convert(clr);
	RECT dst_rc;
	set_rect(p, &dst_rc);	
	HRESULT hr = p->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | AM_DDBLT_WAIT, &bltfx);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::Blt()", hr);
	}
}

// Draw the whole DD surface to the back buffer and destination rectangle
void gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& dst_rc, bool keysrc) {
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
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}


// Draw the src_rc of the DD surface to the back buffer and destination rectangle
void gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& src_rc,
	const lib::rect& dst_rc, bool keysrc, dx_transition *tr) {
	if(!m_surface || !src) return;
	
	if(!tr) {
		draw(src, src_rc, dst_rc, keysrc, m_surface);
		return;
	}
	
	smil2::blitter_type bt = tr->get_blitter_type();
	
	if(bt == smil2::bt_r1r2r3r4) {
#ifdef XXXX 
// r.1.40 leads to #1619481
		smil2::transition_blitclass_r1r2r3r4 *p = tr->get_as_r1r2r3r4_blitter();
		r1r2r3r4_adapter *r1r2r3r4 = (r1r2r3r4_adapter*)p;
		assert(r1r2r3r4);
		// copy rectangle of old pixels away were the new ones go
		lib::rect old_src = r1r2r3r4->get_old_src_rect();
		old_src.translate(r1r2r3r4->get_dst()->get_global_topleft());
		lib::rect old_dst = r1r2r3r4->get_old_dst_rect();
		old_dst.translate(r1r2r3r4->get_dst()->get_global_topleft());
		draw(m_surface, old_src, old_dst, keysrc, m_surface);
		// copy new pixels in place of the old pixels
		lib::rect new_src = r1r2r3r4->get_src_rect();
		lib::rect new_dst = r1r2r3r4->get_dst_rect();
		lib::rect src_rc_v = src_rc;
		lib::rect dst_rc_v = dst_rc;
		src_rc_v &= new_src;
		dst_rc_v &= new_dst;
		dst_rc_v.w = src_rc_v.w; //XXXX
		draw(src, src_rc_v, dst_rc_v, keysrc, m_surface);		
#else /*XXXX*/
// r.1.39 doesn't have the problem
		lib::rect src_rc_v = src_rc;
		lib::rect dst_rc_v = dst_rc;
		clipto_r1r2r3r4(tr, src_rc_v, dst_rc_v);
		draw(src, src_rc_v, dst_rc_v, keysrc, m_surface);
#endif/*XXXX*/
		return;
	} else if(bt == smil2::bt_fade) {
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
//		if (blt_blend(s2, s1, dst_rc, tr->get_progress()/*opacity*/, 0x88888888, 0xFFFFFFFF))
		if (blt_blend(s2, s1, dst_rc, tr->get_progress()/*opacity*/, 0x0, 0xFFFFFFFF))
			draw_to_bgd(s2, dst_rc, 0);
		else
			draw_to_bgd(s1, dst_rc, 0);
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

void gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::rect& src_rc,
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
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
		viewport_logger->trace("Blt %s --> %s failed", repr(src_rc).c_str(), repr(dst_rc).c_str());
	}
}

// Paints the provided string
void gui::dx::viewport::draw(const std::basic_string<text_char>& text, const lib::rect& rc, lib::color_t clr) {
	if(!m_surface || text.empty()) return;	
	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = (clr == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):clr;
	::SetTextColor(hdc, crTextColor);	
	RECT dstRC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	int res = ::DrawText(hdc, text.c_str(), int(text.length()), &dstRC, uFormat); 
	if(res == 0)
		win_report_last_error("DrawText()");
	m_surface->ReleaseDC(hdc);
}

// Frames the provided rect
void gui::dx::viewport::frame_rect(const lib::rect& rc, lib::color_t clr) {
	if(!m_surface) return;	
	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	HBRUSH hbr = CreateSolidBrush(clr);
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	lib::logger::get_logger()->trace("frame_rect: not implemented on Windows Mobile");
#else
	if(FrameRect(hdc, &RC, hbr) == 0)
		win_report_last_error("FrameRect()");
#endif
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
void gui::dx::viewport::blit(IDirectDrawSurface* src, const lib::rect& src_rc,
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
	if (FAILED(hr)) {
		seterror("viewport::blit/DirectDrawSurface::Blt()", hr);
		viewport_logger->trace("Blt %s --> %s failed", repr(src_rc).c_str(), repr(dst_rc).c_str());
	}
}

// Copies to the DD surface the back buffer within the from rect
void gui::dx::viewport::rdraw(IDirectDrawSurface* dst, const lib::rect& from_rc) {
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
	if (FAILED(hr)) {
		seterror("viewport::rdraw/DirectDrawSurface::Blt()", hr);
	}
}

void gui::dx::viewport::copy_bgd_to(IDirectDrawSurface* surf, const lib::rect& rc) { 
	if(!m_surface || !surf) return;
	DWORD flags = AM_DDBLT_WAIT;
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	RECT vrc = {0, 0, m_width, m_height};
	if(!IntersectRect(&RC, &RC, &vrc) || IsRectEmpty(&RC)) return;
	HRESULT hr = surf->Blt(&RC, m_surface, &RC, flags, NULL);
	if (FAILED(hr)) {
		seterror("viewport::copy/DirectDrawSurface::Blt()", hr);
	}
}

void gui::dx::viewport::draw_to_bgd(IDirectDrawSurface* surf, const lib::rect& rc, HRGN hrgn) {
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

uint16 gui::dx::viewport::low_bit_pos(uint32 dword) {
	uint32 test = 1;
	for(uint16 i=0;i<32;i++){
		if(dword & test)
			return i;
		test <<= 1;
	}
	return 0;
}
	
uint16 gui::dx::viewport::high_bit_pos(uint32 dword) {
	uint32 test = 1;
	test <<= 31;
	for(uint16 i=0;i<32;i++){
		if ( dword & test )
			return (uint16)(31-i);
		test >>= 1;
	}
	return 0;
}
	
void gui::dx::viewport::get_pixel_format() {
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

	lo_blue_bit  = low_bit_pos( format.dwBBitMask );
	uint16 hi_blue_bit  = high_bit_pos(format.dwBBitMask);
	blue_bits=(uint16)(hi_blue_bit-lo_blue_bit+1);
}

// Converts a lib::color_t to a DD color
uint32 gui::dx::viewport::convert(lib::color_t color) {
	return convert(lib::redc(color), lib::greenc(color), lib::bluec(color));
}

// Converts the RGB tuple to a DD color
uint32 gui::dx::viewport::convert(BYTE r, BYTE g, BYTE b) {
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
RECT* gui::dx::viewport::to_screen_rc_ptr(RECT& r) {
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
gui::dx::viewport::blt_blend32(const lib::rect& rc, double progress,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2, uint32 low_ddclr, uint32 high_ddclr) {
	
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

	int weight = int(progress*256);	
	for(int row = begin_row-1;row>=end_row;row--) {
		RGBQUAD* px1 = (RGBQUAD*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		RGBQUAD* px2 = (RGBQUAD*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=  begin_col;
		px2 +=  begin_col;
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			if (px1->rgbRed >= r_l && px1->rgbRed <= r_h)
				px2->rgbRed = (BYTE)blend(weight, px2->rgbRed, px1->rgbRed);
			else px2->rgbRed = 0;
			if (px1->rgbGreen >= g_l && px1->rgbGreen <= g_h)
				px2->rgbGreen = (BYTE)blend(weight, px2->rgbGreen, px1->rgbGreen);
			else px2->rgbGreen = 0;
			if (px1->rgbBlue >= b_l && px1->rgbBlue <= b_h)
				px2->rgbBlue = (BYTE)blend(weight, px2->rgbBlue, px1->rgbBlue);
			else px2->rgbBlue = 0;
		}
	}
	surf1->Unlock(0);
	surf2->Unlock(0);
	return hr;
}


HRESULT gui::dx::viewport::blt_blend24(const lib::rect& rc, double progress,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2, uint32 low_ddclr, uint32 high_ddclr) {
	
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
	
	int weight = int(progress*256);	
	for(int row = begin_row-1;row>=end_row;row--) {
		RGBTRIPLE* px1 = (RGBTRIPLE*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		RGBTRIPLE* px2 = (RGBTRIPLE*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=  begin_col;
		px2 +=  begin_col;
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			if (px1->rgbtRed >= r_l && px1->rgbtRed <= r_h)
				px2->rgbtRed = (BYTE)blend(weight, px2->rgbtRed, px1->rgbtRed);
			else px2->rgbtRed = 0;
			if (px1->rgbtGreen >= g_l && px1->rgbtGreen <= g_h)
				px2->rgbtGreen = (BYTE)blend(weight, px2->rgbtGreen, px1->rgbtGreen);
			else px2->rgbtGreen = 0;
			if (px1->rgbtBlue >= b_l && px1->rgbtBlue <= b_h)
				px2->rgbtBlue = (BYTE)blend(weight, px2->rgbtBlue, px1->rgbtBlue);
			else px2->rgbtBlue = 0;
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

HRESULT gui::dx::viewport::blt_blend16(const lib::rect& rc, double progress,
	IDirectDrawSurface *surf1, IDirectDrawSurface *surf2, uint32 low_ddclr, uint32 high_ddclr) {
	
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

	int weight = int(progress*256);	
	for(int row = begin_row-1;row>=end_row;row--) {
		trible565* px1 = (trible565*)((BYTE*)desc1.lpSurface+row*desc1.lPitch);
		trible565* px2 = (trible565*)((BYTE*)desc2.lpSurface+row*desc2.lPitch);
		px1 +=  begin_col;
		px2 +=  begin_col;
		for(int col=begin_col;col<end_col;col++, px1++, px2++) {
			BYTE r = px1->red(), g = px1->green(), b = px1->blue();
			if (r >= r_l && r <= r_h)
				r = (BYTE)blend(weight, px2->red(), r);
			else r = 0;
			if (g >= g_l && g <= g_h)
				g = (BYTE)blend(weight, px2->green(), g);
			else g = 0;
			if (b >= b_l && b <= b_h)
				b = (BYTE)blend(weight, px2->blue(), b);
			else b = 0;
			*px2 = trible565(r, g, b);
		}
	}
	surf1->Unlock(0);
	surf2->Unlock(0);
	return hr;
}

