/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_D2_DSHOWSINK_H
#define AMBULANT_GUI_D2_DSHOWSINK_H

#include "ambulant/config/config.h"
//Every windows application needs to include this
#include <windows.h>
#include <dshow.h>
// NOTE: The next include file comes from sdkdir\Samples\multimedia\directshow\baseclasses.
// If you don't have it you should re-install the SDK and include the sample code.
#include "streams.h"

interface ID2D1RenderTarget;
interface ID2D1Bitmap;

class CVideoD2DBitmapRenderer;

class AMBULANTAPI IVideoD2DBitmapRendererCallback
{
  public:
	virtual void BitmapAvailable(CVideoD2DBitmapRenderer *caller) = 0;
};

class AMBULANTAPI CVideoD2DBitmapRenderer : public CBaseVideoRenderer
{

public:

	//-----------------------------------------------------------------------------
	// Define GUID for Texture Renderer {AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}
	//-----------------------------------------------------------------------------
//	struct __declspec(uuid("{AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}")) CLSID_TextureRenderer;

	CVideoD2DBitmapRenderer(LPUNKNOWN pUnk, HRESULT *phr);

	~CVideoD2DBitmapRenderer();

	HRESULT CheckMediaType(const CMediaType *pmt);
	
	HRESULT SetMediaType(const CMediaType *pmt);

	HRESULT DoRenderSample(IMediaSample * pSample);

	HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
            __inout REFERENCE_TIME *ptrStart,
            __inout REFERENCE_TIME *ptrEnd);

	void SetRenderTarget(ID2D1RenderTarget *rt);

	void SetCallback(IVideoD2DBitmapRendererCallback *callback);
	void SetIgnoreTimestamps(bool ignore) { m_ignore_timestamps = ignore; }
	
	ID2D1Bitmap *LockBitmap();

	void UnlockBitmap(ID2D1Bitmap *bitmap);

	void DestroyBitmap();
private:
	ID2D1RenderTarget *m_rt;
	ID2D1Bitmap *m_d2bitmap;
	IVideoD2DBitmapRendererCallback *m_callback;
	int m_width;   // Video width
	int m_height;  // Video Height
	int m_pitch;   // Video Pitch
	bool m_has_alpha;
	bool m_ignore_timestamps;

};

#endif // AMBULANT_GUI_D2_DSHOWSINK_H
