/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */


#define INITGUID
#include <objbase.h>
#include <ddrawex.h>
#include <uuids.h>
#include <windows.h>
#include <mmsystem.h>

#include "ambulant/gui/dx/dx_viewport.h"


#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"ddraw.lib")
#pragma comment (lib,"dxguid.lib")

#include "ambulant/lib/logger.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/gui/dx/dx_video_player.h"

#include <algorithm>
#include <cassert>

using namespace ambulant;

using lib::uint16;
using lib::uint32;
using lib::uchar;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;

struct error {
	HRESULT hr;
	char *name;
} errorlist [] = {
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

const lib::color_t CLR_DEFAULT = RGB(255, 255, 255);

gui::dx::viewport::viewport(int width, int height, HWND hwnd) 
:	m_width(width), m_height(height),
	m_direct_draw(NULL),
	m_primary_surface(NULL),
	m_surface(NULL),
	m_hwnd(hwnd?hwnd:GetDesktopWindow()),
	bits_size(24),
	red_bits(8), green_bits(8), blue_bits(8),
	lo_red_bit(16), lo_green_bit(8), lo_blue_bit(0),
	palette_entries(0),
	m_bgd(CLR_DEFAULT) {
	
	viewport_logger = lib::logger::get_logger();
	
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
}

gui::dx::viewport::~viewport() {
	RELEASE(m_surface);
	RELEASE(m_primary_surface);
	RELEASE(m_direct_draw);
	if(palette_entries != 0)
		delete[] palette_entries;
	if(!m_hwnd)
		RedrawWindow(GetDesktopWindow(), NULL, NULL, RDW_INVALIDATE |RDW_ERASE | RDW_ALLCHILDREN);
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


// Blt back buffer to primary surface
void gui::dx::viewport::redraw() {
	if(!m_primary_surface || !m_surface)
		return;
	RECT src_rc = {0, 0, m_width, m_height};
	RECT dst_rc = {0, 0, m_width, m_height};
	DWORD flags = DDBLT_WAIT;
	HRESULT hr = m_primary_surface->Blt(to_screen_rc_ptr(dst_rc), m_surface, &src_rc, flags, NULL);
	if (FAILED(hr)) {
		seterror("viewport::redraw/DirectDrawSurface::Blt()", hr);
	}
}

// Clears the back buffer using this viewport bgd color
void gui::dx::viewport::clear() {
	if(!m_surface) return;
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = m_ddbgd; 
	RECT dst_rc = {0, 0, m_width, m_height};
	HRESULT hr = m_surface->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Clears the specified back buffer rectangle using the provided color 
void gui::dx::viewport::clear(const lib::screen_rect<int>& rc, lib::color_t clr) {
	if(!m_surface) return;
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = convert(clr); 
	RECT dst_rc = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	HRESULT hr = m_surface->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Clears a DD surface with the provided color.
void gui::dx::viewport::clear_surface(IDirectDrawSurface* p, lib::color_t clr) {
	DDSURFACEDESC sd;
	memset(&sd, 0, sizeof(DDSURFACEDESC));
	sd.dwSize = sizeof(DDSURFACEDESC);
	DDBLTFX bltfx;
	memset(&bltfx, 0, sizeof(DDBLTFX));
	bltfx.dwSize = sizeof(bltfx);
	bltfx.dwFillColor = (clr == CLR_INVALID)?m_ddbgd:convert(clr);
	RECT dst_rc;
	set_rect(p, &dst_rc);	
	HRESULT hr = p->Blt(&dst_rc, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::Blt()", hr);
	}
}

// Draw the whole DD surface to the back buffer and destination rectangle
void gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::screen_rect<int>& dst_rc, bool keysrc) {
	if(!m_surface || !src) return;
	DWORD flags = DDBLT_WAIT;
	if(keysrc) flags |= DDBLT_KEYSRC;
	RECT srcRC;
	set_rect(src, &srcRC);
	RECT dstRC;
	set_rect(dst_rc, &dstRC);
	HRESULT hr = m_surface->Blt(&dstRC, src, &srcRC, flags, NULL);
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Draw the src_rc of the DD surface to the back buffer and destination rectangle
void gui::dx::viewport::draw(IDirectDrawSurface* src, const lib::screen_rect<int>& src_rc,
	const lib::screen_rect<int>& dst_rc, bool keysrc) {
	if(!m_surface || !src) return;
	DWORD flags = DDBLT_WAIT;
	if(keysrc) flags |= DDBLT_KEYSRC;
	
	RECT srcRC;
	set_rect(src_rc, &srcRC);
	
	RECT dstRC;
	set_rect(dst_rc, &dstRC);
	
	HRESULT hr = m_surface->Blt(&dstRC, src, &srcRC, flags, NULL);
	if (FAILED(hr)) {
		seterror(":viewport::clear/DirectDrawSurface::Blt()", hr);
	}
}

// Paints the provided string
void gui::dx::viewport::draw(const std::string text, const lib::screen_rect<int>& dst_rc) {
	if(!m_surface || text.empty()) return;	
	HDC hdc;
	HRESULT hr = m_surface->GetDC(&hdc);
	if (FAILED(hr)) {
		seterror("DirectDrawSurface::GetDC()", hr);
		return;
	}
	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
	::SetTextColor(hdc, crTextColor);	
	RECT dstRC;
	set_rect(dst_rc, &dstRC);
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	int res = ::DrawText(hdc, text.c_str(), int(text.length()), &dstRC, uFormat); 
	if(res == 0)
		win_report_last_error("DrawText()");
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
	POINT pt = {0, 0}; // margins
	::ClientToScreen(m_hwnd, &pt);
	r.left += pt.x;
	r.right += pt.x;
	r.top += pt.y;
	r.bottom += pt.y;
	return &r;
}



