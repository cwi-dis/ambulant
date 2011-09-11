//needed for DirectShow. Do not change order of the first 3 includes!
//#include <streams.h>

#include "ambulant/gui/d2/d2_dshowsink.h"
#include "ambulant/lib/logger.h"

#include <d2d1.h>
#include <d2d1helper.h>

#include <cassert>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

struct __declspec(uuid("{AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}")) CLSID_TextureRenderer;

CVideoD2DBitmapRenderer::CVideoD2DBitmapRenderer(LPUNKNOWN pUnk, HRESULT *phr)
:	CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), NAME("Texture Renderer"), pUnk, phr),
	m_rt(NULL),
#ifdef WITH_SCREENSHOT
	m_sample_data(NULL),
	m_sample_data_size(0),
#endif
	m_d2bitmap(NULL),
	m_callback(NULL),
	m_width(0),
	m_height(0),
	m_pitch(0),
	m_has_alpha(false),
	m_ignore_timestamps(false)
{
	*phr = S_OK;
}


//-----------------------------------------------------------------------------
// CVideoTextureRenderer destructor
//-----------------------------------------------------------------------------
CVideoD2DBitmapRenderer::~CVideoD2DBitmapRenderer()
{
	m_d2bitmap_lock.enter();
	if (m_d2bitmap) {
		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
	m_d2bitmap_lock.leave();
	m_rt = NULL;
}

void
CVideoD2DBitmapRenderer::SetRenderTarget(ID2D1RenderTarget *rt)
{
	if (rt == m_rt) return;
	m_d2bitmap_lock.enter();
	if (rt) rt->AddRef();
	ID2D1Bitmap *old_bitmap = m_d2bitmap;
	ID2D1RenderTarget *old_rt = m_rt;
	m_d2bitmap = NULL;
	m_rt = rt;
	if (old_rt) old_rt->Release();
	if (old_bitmap) old_bitmap->Release();
	m_d2bitmap_lock.leave();
}

void
CVideoD2DBitmapRenderer::SetCallback(IVideoD2DBitmapRendererCallback *callback)
{
	m_callback = callback;
}

ID2D1Bitmap *
CVideoD2DBitmapRenderer::LockBitmap()
{
	// XXX Lock it.
	m_d2bitmap_lock.enter();
#ifdef WITH_SCREENSHOT
	if (m_d2bitmap == NULL && m_sample_data != NULL) {
		// Someone changed our RenderTarget, probably due to a screenshot
		// being taken. Re-create the bitmap.
		(void)_SampleToBitmap(m_sample_data);
	}
#endif
	if (m_d2bitmap) m_d2bitmap->AddRef();
	ID2D1Bitmap *rv = m_d2bitmap;
	m_d2bitmap_lock.leave();
	return rv;
}

void
CVideoD2DBitmapRenderer::UnlockBitmap(ID2D1Bitmap *bitmap)
{
	bitmap->Release();
	// XXX Unlock it.
}

void
CVideoD2DBitmapRenderer::DestroyBitmap()
{
	m_d2bitmap_lock.enter();
	if (m_d2bitmap) m_d2bitmap->Release();
	m_d2bitmap = NULL;
	m_d2bitmap_lock.leave();
}

//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT hr = E_FAIL;
	VIDEOINFO *pvi=0;

	CheckPointer(pmt,E_POINTER);

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo ) {
		return E_INVALIDARG;
	}

	// Only accept 24 or 32bit RGB[A] video
	pvi = (VIDEOINFO *)pmt->Format();

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video)) {
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB24)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_RGB24");
			hr = S_OK;
		}
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_RGB32");
			hr = S_OK;
		}
		if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_ARGB32)) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::CheckMediaType: MEDIASUBTYPE_ARGB32");
			hr = S_OK;
		}
	}
	return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::SetMediaType(const CMediaType *pmt)
{
	HRESULT hr;

	UINT uintWidth = 2;
	UINT uintHeight = 2;

	// Retrieve the size of this media type
	assert(IsEqualGUID(*pmt->Type(), MEDIATYPE_Video));
	if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB32)) {
		AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::SetMediaType: MEDIASUBTYPE_RGB32");
		hr = S_OK;
	} else if (IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_ARGB32)) {
		AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::SetMediaType: MEDIASUBTYPE_ARGB32");
		hr = S_OK;
	} else {
		assert(0);
	}
	VIDEOINFO *pviBmp;                      // Bitmap info header
	pviBmp = (VIDEOINFO *)pmt->Format();

	m_width = pviBmp->bmiHeader.biWidth;
	m_height = abs(pviBmp->bmiHeader.biHeight);
	m_pitch = m_width*4; // Only 32-bit formats supported.
	m_has_alpha = MEDIASUBTYPE_HASALPHA(*pmt);

	return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CVideoD2DBitmapRenderer::DoRenderSample( IMediaSample * pSample )
{
	AM_DBG {long long mTime0=0, mTime1=0; pSample->GetMediaTime(&mTime0, &mTime1); ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::DoRenderSample(%lld, %lld)", mTime0, mTime1); }
	CheckPointer(pSample,E_POINTER);
	BYTE *pBmpBuffer = NULL;
	pSample->GetPointer(&pBmpBuffer);
#ifdef WITH_SCREENSHOT
	// Release the old sample data
	if (m_sample_data) free(m_sample_data);
	m_sample_data = NULL;
	m_sample_data_size = 0;
	// Get the video bitmap buffer data
	m_sample_data_size = pSample->GetActualDataLength();
	m_sample_data = (BYTE *)malloc(m_sample_data_size);
	if (m_sample_data == NULL||pBmpBuffer == NULL)
		return E_OUTOFMEMORY;
	memcpy((void*)m_sample_data, pBmpBuffer, m_sample_data_size);
	HRESULT hr = _SampleToBitmap(m_sample_data);

#else
	HRESULT hr = _SampleToBitmap(pBmpBuffer);
#endif
	if (!SUCCEEDED(hr))
		return hr;
	if (m_callback) 
		m_callback->BitmapAvailable(this);
	return S_OK;
}

HRESULT CVideoD2DBitmapRenderer::_SampleToBitmap(BYTE *pBMPBytes)
{
	HRESULT hr;

	if (m_rt == NULL) 
		return S_OK;
	assert(pBMPBytes);
#ifdef WITH_SCREENSHOT
	if (m_width*m_height*4 != m_sample_data_size) {
		ambulant::lib::logger::get_logger()->trace("CVideoD2DBitmapRenderer::DoRenderSample: incorrect sized data: %d", m_sample_data_size);
		m_d2bitmap_lock.enter();
		if (m_d2bitmap) m_d2bitmap->Release();
		m_d2bitmap = NULL;
		m_d2bitmap_lock.leave();
		return E_FAIL;
	}
#endif
	ID2D1Bitmap *bitmap = NULL;
	D2D1_SIZE_U size = D2D1::SizeU(m_width, m_height);
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	props.pixelFormat = D2D1::PixelFormat(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		m_has_alpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
	hr = m_rt->CreateBitmap(size, pBMPBytes, m_pitch, props, &bitmap);
	if (!SUCCEEDED(hr)) {
		ambulant::lib::logger::get_logger()->trace("CVideoD2DBitmapRenderer::DoRenderSample: CreateBitmap: error 0x%x", hr);
	}
	// XXX Lock
	AM_DBG ambulant::lib::logger::get_logger()->debug("CVideoD2DBitmapRenderer::DoRenderSample: new bitmap is 0x%x", bitmap);
	m_d2bitmap_lock.enter();
	ID2D1Bitmap *old_bitmap = m_d2bitmap;
	m_d2bitmap = bitmap;
	// XXX Unlock
	if (old_bitmap) 
		old_bitmap->Release();
	m_d2bitmap_lock.leave();
	return hr;
}

HRESULT CVideoD2DBitmapRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                                __inout REFERENCE_TIME *ptrStart,
                                                __inout REFERENCE_TIME *ptrEnd)
{
	HRESULT rv;
	if (m_ignore_timestamps) {
		rv = S_OK;
	} else {
		rv = CBaseVideoRenderer::ShouldDrawSampleNow(pMediaSample, ptrStart, ptrEnd);
	}
	AM_DBG ambulant::lib::logger::get_logger()->debug("ShouldDrawSampleNow(..., %lld, %lld) [ignoretimestamp=%d] -> 0x%x", (long long)*ptrStart, (long long)*ptrEnd, m_ignore_timestamps, rv);
	return rv;
}
