// ieambulant.cpp : Implementation of Cieambulant
#include "stdafx.h"
#include "comutil.h"
#include "AmbulantActiveX.h"
#include "ieambulant.h"
#include <fstream>
#include <string>
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

/////////////////////////////////////////////////////////////////////////////
// Cieambulant

// convert BSTR to std::string
std::string
BSTR_to_std_string (BSTR bstrIn) {
	if (bstrIn == NULL)
		return "";

	int nInputStrLen = SysStringLen (bstrIn);
	// Double NULL Termination
	int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, NULL, 0, 0, 0) + 2;

	LPSTR lpstr = new char [nOutputStrLen];
	if (lpstr) {
		memset (lpstr, 0x00, sizeof (char)*nOutputStrLen);
		WideCharToMultiByte (CP_ACP, 0, bstrIn, nInputStrLen, lpstr, nOutputStrLen, 0, 0);
	}
	std::string result(lpstr);
	delete [] lpstr;
	return result;
}

//	convert CComBSTR to std::string
std::string
CComBSTR_to_std_string (CComBSTR BSTR_value) {
	std::string result = BSTR_to_std_string((BSTR)BSTR_value);
	return result;
}


HRESULT
Cieambulant::get_document_url() {
	// here the <url> is obtained from the <PARAM name="src" value="<url>"/> element
	// and joined with	the base url.
	// also the the value of <PARAM name="autostart" value="<true|false>"/> is retrieved.

	HRESULT hr = E_FAIL;
	if ( ! m_site)
		return hr;

	IServiceProvider* pISP = NULL;
	hr = m_site->QueryInterface(IID_IServiceProvider, (void **)&pISP);
	if ( ! SUCCEEDED(hr))
		return hr;

	IWebBrowser2* pIWebBrowser2 = NULL;
	hr = pISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void **)&pIWebBrowser2);
	if ( ! SUCCEEDED(hr))
		return hr;

	BSTR BSTR_base_url;
	hr = pIWebBrowser2->get_LocationURL(&BSTR_base_url);
	if ( ! SUCCEEDED(hr))
		return hr;
	std::string std_string_base_url = CComBSTR_to_std_string (BSTR_base_url);
	SysFreeString(BSTR_base_url);
	m_base_url = ambulant::net::url::from_url(std_string_base_url);
	m_url = ambulant::net::url::from_url(CComBSTR_to_std_string(m_bstrSrc));

	std::string std_string_autostart = CComBSTR_to_std_string(m_bstrAutostart);
	if (std_string_autostart == "false"
		|| std_string_autostart == "FALSE")
		m_autostart = false;

	return S_OK;

}

LPOLECLIENTSITE s_site;
int s_ref_count;

STDMETHODIMP
Cieambulant::SetClientSite(LPOLECLIENTSITE pSite) {
	HRESULT hr = CComControlBase::IOleObject_SetClientSite(pSite);
	if (hr != S_OK)
		return hr;
	if (pSite) {
		s_ref_count++;
		if (s_ref_count == 1)
			s_site = pSite;
	} else {
		if (s_ref_count > 0)
			s_ref_count--;
		if (s_ref_count == 0) {
			// clear all statics
			s_site = NULL;
		}
	}
	m_site = pSite;
//	if(pSite && !m_pFont)
//		hr = GetAmbientFontDisp(&m_pFont);
	GetAmbientBackColor(m_clrBackColor);
	GetAmbientForeColor(m_clrForeColor);
	//XXX std::ostream sos = new std::ostream(std::cout);
	//XXX ambulant::lib::logger::get_logger()->set_std_ostream(sos);
//	static std::ofstream log_os("C:\\Documents and Settings\\kees.AMBULANT-DEV\\My Documents\\Ambulant\\ambulant\\src\\ieambulant\\amlog.txt");
//	ambulant::lib::logger::get_logger()->set_std_ostream(log_os);
//	s_hook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId());
	return hr;
}

void
DrawString( HDC hdc, RECT* rc, BSTR caption ) {
	USES_CONVERSION;
	TCHAR* pCaption = OLE2T(caption);
	DrawText(
		hdc,
		pCaption,
		lstrlen( pCaption ),
		rc,
		DT_WORDBREAK );
}

// STDMETHODIMP HRESULT
HRESULT
Cieambulant::OnDraw(ATL_DRAWINFO& di) {
	return S_OK;
}

HWND s_hwnd = NULL;

HRESULT
Cieambulant::updatePlayer() {
	// added for ambulant

	if (m_url.get_url() == "")
		get_document_url();
	if ( !m_url.is_absolute()) {
		ambulant::net::url tmp_url;
		if (m_base_url.get_url() != "")
			m_url = m_url.join_to_base(m_base_url);
	}
	if (m_hwnd == NULL) {
		m_hwnd = this->m_hWnd;//::GetWindow(this->m_hWnd, GW_HWNDFIRST);
		m_player_callbacks.set_os_window(m_hwnd);
		s_hwnd = m_hwnd;
	}
	if (m_ambulant_player == NULL) {
		// prepare for dynamic linking ffmpeg
#if 1
		ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
		prefs->m_prefer_ffmpeg = true;
		prefs->m_use_plugins = true;
#endif
		// save the HWND for any Ambulant plugins (such as SMIL State)
		ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
		void *edptr = pe->get_extra_data("npapi_extra_data");
		if (edptr) {
			*(HWND*)edptr = m_hWnd;;
			AM_DBG fprintf(stderr, "ieambulant::updatePlayer(): setting npapi_extra_data(0x%x) to HWND 0x%x\n", edptr, m_hWnd);
		} else {
			AM_DBG fprintf(stderr, "ieambulant::updatePlayer(): Cannot find npapi_extra_data, cannot communicate HWND\n");
		}

		m_player_callbacks.set_os_window(m_hwnd);
		m_ambulant_player = new ambulant_gui_player(m_player_callbacks, NULL, m_url);
//X		m_ambulant_player->set_state_component_factory(NULL); // XXXJACK DEBUG!!!!
		if (m_ambulant_player) {
			if ( !get_player()) {
				abort();
				delete m_ambulant_player;
				m_ambulant_player = NULL;
			} else {
				ambulant::lib::logger::get_logger()->set_show_message(ieambulant_display_message);
				ambulant::lib::logger::get_logger()->show("ieambulant plugin loaded");
				if (m_autostart)
					m_ambulant_player->play();
			}
		}
	}
	return S_OK;
}

// some platform/toolkit specific hacks and functions
static ambulant_player_callbacks s_ambulant_player_callbacks;

LRESULT CALLBACK
Cieambulant::PluginWinProc(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	typedef HRGN NPRegion;
	if (updatePlayer() != S_OK)
		return S_FALSE;

	switch (msg) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(&ps);
			if (m_hwnd && m_ambulant_player) {
				HDC hdc = ::GetDC(m_hwnd);
				m_ambulant_player->redraw(m_hwnd, hdc, NULL); // XXX Should pass dirty rect
				::ShowWindow(m_hwnd, SW_SHOW);
				::ReleaseDC(m_hwnd, hdc);
			}
			EndPaint(&ps);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		POINT point;
		point.x=GET_X_LPARAM(lParam);
		point.y=GET_Y_LPARAM(lParam);
		if (m_ambulant_player) {
			if (msg == WM_MOUSEMOVE) {
				// code copied from MmView.cpp
				int new_cursor_id = m_ambulant_player->get_cursor(point.x, point.y, m_hwnd);
				if(new_cursor_id != m_cursor_id) {
					HINSTANCE hIns = 0;
					HCURSOR new_cursor = 0;
					if(new_cursor_id == 0) {
						new_cursor = LoadCursor(hIns, IDC_ARROW);
					} else {
						new_cursor = LoadCursor(hIns, IDC_HAND);
					}
					SetClassLongPtr(m_hwnd, GCLP_HCURSOR, HandleToLong(new_cursor));
					m_cursor_id = new_cursor_id;
				}
			} else {
				m_ambulant_player->on_click(point.x, point.y, m_hwnd);
			}
		}
		break;

	default:
		break;
	}
	BOOL rv = DefWindowProc(msg, wParam, lParam);
	bHandled = rv;
	return rv;
}

ambulant_player_callbacks::ambulant_player_callbacks()
:	m_hwnd(NULL)
{
}

void
ambulant_player_callbacks::set_os_window(HWND hwnd) {
	m_hwnd = hwnd;
}

HWND
ambulant_player_callbacks::new_os_window() {
	return m_hwnd;
}

SIZE
ambulant_player_callbacks::get_default_size() {
	SIZE size;
	size.cx = ambulant::common::default_layout_width;
	size.cy = ambulant::common::default_layout_height;
	return size;
}

void
ambulant_player_callbacks::destroy_os_window(HWND hwnd) {
	m_hwnd = NULL;
}

html_browser*
ambulant_player_callbacks::new_html_browser(int left, int top, int width, int height) {
	return NULL; // not implemented, but needs to be declared
}

STDMETHODIMP
Cieambulant::get_mimeType(BSTR *pVal) {
	if (!pVal) {
		return E_INVALIDARG;
	}
	*pVal = m_bstrType.Copy();
	return S_OK;
}

STDMETHODIMP
Cieambulant::put_mimeType(BSTR newVal) {
	m_bstrType.Empty();
	m_bstrType.Attach(SysAllocString(newVal));
	return S_OK;
}

STDMETHODIMP
Cieambulant::get_src(BSTR *pVal) {
	if (!pVal) {
		return E_INVALIDARG;
	}
	*pVal = m_bstrSrc.Copy();
	return S_OK;
}

STDMETHODIMP
Cieambulant::put_src(BSTR newVal) {
	m_bstrSrc.Empty();
	m_bstrSrc.Attach(SysAllocString(newVal));
	return S_OK;
}

STDMETHODIMP
Cieambulant::put_autostart(BSTR newVal) {
	m_bstrAutostart.Empty();
	m_bstrAutostart.Attach(SysAllocString(newVal));
	return S_OK;
}

STDMETHODIMP
Cieambulant::get_autostart(BSTR *pVal) {
	if (!pVal) {
		return E_INVALIDARG;
	}
	*pVal = m_bstrAutostart.Copy();
	return S_OK;
}

// following 3 lines for CString (conversion char* to BSTR)
#define _AFXDLL
#undef _WINDOWS_
#include <afxwin.h>

void
ieambulant_display_message(int level, const char* message) {
	USES_CONVERSION;

	HRESULT hr = E_FAIL;
	if ( ! s_site)
		return;

	IServiceProvider* pISP = NULL;
	hr = s_site->QueryInterface(IID_IServiceProvider, (void **)&pISP);
	if ( ! SUCCEEDED(hr))
		return;
	// get IWebBrowser2
	IWebBrowser2* pIWebBrowser2 = NULL;
	hr = pISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void **)&pIWebBrowser2);
	pISP->Release();
	if ( ! SUCCEEDED(hr) || pIWebBrowser2 == NULL)
		return;
	IDispatch *pIDispatchDocument = NULL;
	hr = pIWebBrowser2->get_Document(&pIDispatchDocument);
	pIWebBrowser2->Release();
	if ( ! SUCCEEDED(hr) || pIDispatchDocument == NULL)
		return;
	// get IHTMLDocument2
	IHTMLDocument2* pIHTMLDocument2 = NULL;
	hr = pIDispatchDocument->QueryInterface(IID_IHTMLDocument2, (void **)&pIHTMLDocument2);
	pIDispatchDocument->Release();
	if ( ! SUCCEEDED(hr) || pIHTMLDocument2 == NULL)
		return;
	// get IHTMLWindow2
	IHTMLWindow2* pIHTMLWindow2 = NULL;
	hr = pIHTMLDocument2->get_parentWindow(&pIHTMLWindow2);
	pIHTMLDocument2->Release();
	if ( ! SUCCEEDED(hr) || pIHTMLWindow2 == NULL)
		return;
	CString Cstr_message(message);
	BSTR BSTR_message = Cstr_message.AllocSysString();
	pIHTMLWindow2->put_status(BSTR_message);
	SysFreeString(BSTR_message);
	pIHTMLWindow2->Release();
}

STDMETHODIMP
Cieambulant::startPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->play();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

STDMETHODIMP
Cieambulant::stopPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->stop();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

STDMETHODIMP
Cieambulant::restartPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->stop();
		m_ambulant_player->play();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

STDMETHODIMP
Cieambulant::pausePlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->pause();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

STDMETHODIMP
Cieambulant::isDone( BOOL* pVal) {
	if (m_ambulant_player != NULL) {
		*pVal = m_ambulant_player->get_player()->is_done();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

STDMETHODIMP
Cieambulant::resumePlayer( void) {
	if (m_ambulant_player != NULL) {
		if (m_ambulant_player->get_player()->is_pausing())
			m_ambulant_player->play();
		return S_OK;
	} else {
		return E_FAIL;
	}
}

LRESULT
Cieambulant::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return PluginWinProc(uMsg, wParam, lParam, bHandled);
}

LRESULT
Cieambulant::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return PluginWinProc(uMsg, wParam, lParam, bHandled);
}

//LRESULT Cieambulant::Plugin(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	// TODO: Add your message handler code here and/or call default
//
//	return 0;
//}
