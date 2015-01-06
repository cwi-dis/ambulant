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

#ifndef AMBULANT_GUI_D2_DSHOWSINK_H
#define AMBULANT_GUI_D2_DSHOWSINK_H

#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"
#include <windows.h>
#include <dshow.h>
// NOTE: The next include file comes from sdkdir\Samples\multimedia\directshow\baseclasses.
// If you don't have it you should re-install the SDK and include the sample code.
#include "streams.h"

// To enable screenshots you must define WITH_SCREENSHOT. But there is a price
// to pay: each frame will be copied.
#define WITH_SCREENSHOT

interface ID2D1RenderTarget;
interface ID2D1Bitmap;
class CVideoD2DBitmapRenderer;

/// Data availability callback interface.
/// Renderers that use CVideoD2DBitmapRenderer must implement this interface, it
/// will be used to inform the client when a new video frame is available.
class AMBULANTAPI IVideoD2DBitmapRendererCallback
{
  public:
	virtual void BitmapAvailable(CVideoD2DBitmapRenderer *caller) = 0;
};

/// DirectX renderer that produces bitmaps.
/// Instances of this class can be used at the end of a DirectShow pipeline. There, bitmaps
/// will be collected which can then be processed further by the application.
class AMBULANTAPI CVideoD2DBitmapRenderer : public CBaseVideoRenderer
{
  public:
	CVideoD2DBitmapRenderer(LPUNKNOWN pUnk, HRESULT *phr);
	~CVideoD2DBitmapRenderer();

	// Methods required by DirectShow.
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT SetMediaType(const CMediaType *pmt);
	HRESULT DoRenderSample(IMediaSample * pSample);
	HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
            __inout REFERENCE_TIME *ptrStart,
            __inout REFERENCE_TIME *ptrEnd);

	/// Signals that bitmaps created are compatible with the given Direct2D RenderTarget.
	void SetRenderTarget(ID2D1RenderTarget *rt);

	/// Set the callback handler, called when new bitmaps are available.
	void SetCallback(IVideoD2DBitmapRendererCallback *callback);

	/// Should the bitmap renderer honour timestamps (delaying bitmaps) or not?
	void SetIgnoreTimestamps(bool ignore) { m_ignore_timestamps = ignore; }

	/// Obtain a reference to the most recently decoded bitmap.
	ID2D1Bitmap *LockBitmap();

	/// Release the reference to the bitmap obtained with LockBitmap previously.
	void UnlockBitmap(ID2D1Bitmap *bitmap);

	/// Free the current bitmap.
	void DestroyBitmap();
private:
	// Convert the DX m_sample to the D2D m_d2bitmap
	HRESULT _SampleToBitmap(BYTE *data);
	ID2D1RenderTarget *m_rt;
#ifdef WITH_SCREENSHOT
	BYTE *m_sample_data;
	int m_sample_data_size;
#endif
	ID2D1Bitmap *m_d2bitmap;
	IVideoD2DBitmapRendererCallback *m_callback;
	int m_width;   // Video width
	int m_height;  // Video Height
	int m_pitch;   // Video Pitch
	bool m_has_alpha;
	bool m_ignore_timestamps;
	ambulant::lib::critical_section m_d2bitmap_lock;

};

#endif // AMBULANT_GUI_D2_DSHOWSINK_H
