// ieambulant.cpp : Implementation of Cieambulant
#include "stdafx.h"
#include "comutil.h"
#include "AmbulantActiveX.h"
#include "ieambulant.h"
#include <fstream>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// Cieambulant

// convert BSTR to LPSTR, allocates new char[] 
LPSTR
ConvertBSTRToLPSTR (BSTR bstrIn)
{
  	LPSTR pszOut = NULL;

	if (bstrIn != NULL)
  	{
   		int nInputStrLen = SysStringLen (bstrIn);
   		// Double NULL Termination
   		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, NULL, 0, 0, 0) + 2;	
   
   		pszOut = new char [nOutputStrLen];
   		if (pszOut)
   		{
   		    memset (pszOut, 0x00, sizeof (char)*nOutputStrLen);
 			WideCharToMultiByte (CP_ACP, 0, bstrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
   		}
   	}
	return pszOut;
}



HRESULT
Cieambulant::get_document_url() 
{
	HRESULT hr = E_FAIL;
	if ( ! m_site)
		return hr;

	IServiceProvider* pISP = NULL;
	hr = m_site->QueryInterface(IID_IServiceProvider, (void **)&pISP);
	if ( ! SUCCEEDED(hr))
		return hr;

	IWebBrowser2* pIWebBrowser2 = NULL;
	hr = pISP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2,
					   (void **)&pIWebBrowser2);
	if ( ! SUCCEEDED(hr))
		return hr;

	BSTR BSTR_base_url;
	hr = pIWebBrowser2->get_LocationURL(&BSTR_base_url);
	if ( ! SUCCEEDED(hr))
		return hr;

	LPSTR LPSTR_url = ConvertBSTRToLPSTR (BSTR_base_url);
	SysFreeString(BSTR_base_url);
	BSTR_base_url = NULL;
	std::string std_string_base_url (LPSTR_url);
	m_base_url = ambulant::net::url::from_url(std_string_base_url);
	delete [] LPSTR_url;
	IDispatch *dispatch = NULL;
	LPPROPERTYBAG pPropBag = (IPropertyBag*) malloc (sizeof IPropertyBag);
	hr = this->IPersistPropertyBag_Load(pPropBag, NULL, NULL);
	hr = pIWebBrowser2->get_Document(&dispatch);
	if ( ! SUCCEEDED(hr) || dispatch == NULL)
		return hr;

	IHTMLDocument3* document3;
	hr = dispatch->QueryInterface(IID_IHTMLDocument3,
		                          (void **)&document3);
	dispatch->Release();
	if ( ! SUCCEEDED(hr) || document3 == NULL)
		return hr;

	BSTR BSTR_object = SysAllocString(L"object");
	IHTMLElementCollection* pelColl;
	hr = document3->getElementsByTagName(BSTR_object, &pelColl);
	document3->Release();
	SysFreeString(BSTR_object);
	if ( ! SUCCEEDED(hr))
		return hr;

	VARIANT itemIndex;
	itemIndex.vt = VT_I4;
	itemIndex.lVal = 0;
	IDispatch *pElemDisp;
	hr = pelColl->item(itemIndex, itemIndex, &pElemDisp);
	pelColl->Release();
	if ( ! SUCCEEDED(hr))
		return hr;

	IHTMLElement* pHTMLElement;
	hr = pElemDisp->QueryInterface(IID_IHTMLElement, (void**) &pHTMLElement);
	pElemDisp->Release();

	if ( ! SUCCEEDED(hr))
		return hr;

	BSTR BSTR_src = SysAllocString(L"src");
	VARIANT var;
	hr = pHTMLElement->getAttribute(BSTR_src, 0, &var);
	pHTMLElement->Release();
	if ( ! SUCCEEDED(hr))
		return hr;

	if (var.vt != VT_NULL) {
		LPSTR_url = ConvertBSTRToLPSTR (_bstr_t(var));	
		std::string std_string_url (LPSTR_url);
		m_url = ambulant::net::url::from_url(std_string_url);
		delete [] LPSTR_url;
	}
}

/* MSG spy
static HHOOK s_hook;
LRESULT  CALLBACK
GetMsgProc(int   nCode,WPARAM   wparam,LPARAM   lparam)   
{  
    MSG * msg = (MSG *)lparam;
	ambulant::lib::logger::get_logger()->debug("MSG=0x%x",msg->message);
	if( msg->message == WM_COMMAND) {
		//analyse the command message 
		ambulant::lib::logger::get_logger()->debug("nCode=0x%x, wparam=0x%x",nCode,wparam);
    }
    return   CallNextHookEx(s_hook,nCode,wparam,lparam);   
}
*/
LPOLECLIENTSITE s_site;

STDMETHODIMP 
Cieambulant::SetClientSite(LPOLECLIENTSITE pSite)
{
    HRESULT hr = CComControlBase::IOleObject_SetClientSite(pSite);
	if (hr != S_OK)
		return hr;
	if (s_site == NULL) {
		m_site = pSite;
		s_site = pSite;
	}
//    if(pSite && !m_pFont)
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
DrawString( HDC hdc, RECT* rc, BSTR caption )
	{
		USES_CONVERSION;		
		TCHAR* pCaption = OLE2T(caption);
		DrawText( hdc,
				 pCaption,
				 lstrlen( pCaption ),
				 rc,
				 DT_WORDBREAK );
	}

// STDMETHODIMP HRESULT 
HRESULT
Cieambulant::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
}

HWND s_hwnd = NULL;

HRESULT
Cieambulant::updatePlayer()
{
	// added for ambulant
	// here the <url> is obtained from the <PARAM name="src" value="<url>"/> element
	// and joined with  the base url (see get_document_location())
	if (m_url.get_url() == "")
		get_document_url();
	if ( ! m_url.is_absolute()) {
		ambulant::net::url tmp_url;    
        if (m_base_url.get_url() != "")
            m_url = m_url.join_to_base(m_base_url);
    }
	if (m_hwnd == NULL) {
		m_hwnd = this->m_hWnd; //::GetWindow(this->m_hWnd, GW_HWNDFIRST);
		m_player_callbacks.set_os_window(m_hwnd);
		s_hwnd = m_hwnd;
	}
	if (m_ambulant_player == NULL) {
		m_player_callbacks.set_os_window(m_hwnd);
		m_ambulant_player = new ambulant::gui::dx::dx_player(m_player_callbacks, NULL, m_url);
//X		m_ambulant_player->set_state_component_factory(NULL); // XXXJACK DEBUG!!!!
		if (m_ambulant_player) {
			if ( ! get_player()) {
				delete m_ambulant_player;
				m_ambulant_player = NULL;
			} else {
				ambulant::lib::logger::get_logger()->set_show_message(ieambulant_display_message);
				ambulant::lib::logger::get_logger()->show("ieambulant plugin loaded");
				m_ambulant_player ->play();
			}
		} 
	}
	return S_OK;
}

// some platform/toolkit specific hacks and functions
static ambulant_player_callbacks s_ambulant_player_callbacks;

LRESULT CALLBACK
Cieambulant::PluginWinProc(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	typedef HRGN NPRegion;
	if (updatePlayer() != S_OK)
		return S_FALSE;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(&ps);
//				RECT rc;
//				GetClientRect(&rc);
//				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				if (m_hwnd && m_ambulant_player) {
					HDC hdc = ::GetDC(m_hwnd);
					m_ambulant_player->redraw(m_hwnd, hdc);
					::ShowWindow(m_hwnd, SW_SHOW);
					::ReleaseDC(m_hwnd, hdc);
				}
				EndPaint(&ps);
				break;
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
			{
				POINT point;
				point.x=GET_X_LPARAM(lParam);
				point.y=GET_Y_LPARAM(lParam);
				if (m_ambulant_player) {
					if (msg == WM_MOUSEMOVE) {
						// code copied from MmView.cpp
						int new_cursor_id = m_ambulant_player->get_cursor(point.x, point.y, m_hwnd);
//XX					if (new_cursor_id>0) EnableToolTips(TRUE);
//XX					else CancelToolTips();
						if(new_cursor_id != m_cursor_id) {
							HINSTANCE hIns = 0;
							HCURSOR new_cursor = 0;
							if(new_cursor_id == 0) {
								new_cursor = LoadCursor(hIns, IDC_ARROW); 
							} else {
								new_cursor = LoadCursor(hIns, IDC_HAND); 
							}
//XX						SetClassLongPtr(GCLP_HCURSOR, HandleToLong(new_cursor));
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
ambulant_player_callbacks::set_os_window(HWND hwnd)
{
	m_hwnd = hwnd;
}

HWND 
ambulant_player_callbacks::new_os_window()
{
	return m_hwnd;
}

SIZE
ambulant_player_callbacks::get_default_size()
{
	SIZE size;
	size.cx = ambulant::common::default_layout_width;
	size.cy = ambulant::common::default_layout_height;
	return size;
}

void
ambulant_player_callbacks::destroy_os_window(HWND hwnd)
{
	m_hwnd = NULL;
}

html_browser*
ambulant_player_callbacks::new_html_browser(int left, int top, int width, int height)
{
	return NULL; // not implemented, but needs to be declared
}

STDMETHODIMP
Cieambulant::get_mimeType(BSTR *pVal)
{
	if (!pVal)
    {
        return E_INVALIDARG;
    }
    *pVal = m_bstrType.Copy();
	return S_OK;
}

STDMETHODIMP
Cieambulant::put_mimeType(BSTR newVal)
{
    m_bstrType.Empty();
    m_bstrType.Attach(SysAllocString(newVal));
	return S_OK;
}

STDMETHODIMP
Cieambulant::get_src(BSTR *pVal)
{
	if (!pVal)
    {
        return E_INVALIDARG;
    }
    *pVal = m_bstrSrc.Copy();
	return S_OK;
}

STDMETHODIMP
Cieambulant::put_src(BSTR newVal)
{
    m_bstrSrc.Empty();
    m_bstrSrc.Attach(SysAllocString(newVal));
	return S_OK;
}

// following 3 lines for CString (conversion char* to BSTR)
#define _AFXDLL
#undef _WINDOWS_
#include <afxwin.h>

void
ieambulant_display_message(int level, const char* message) {
	USES_CONVERSION;
/*JNK: code does not work
	HWND top = ::GetTopWindow(s_hwnd);
	if (top == NULL)
		return;
	HWND statusBar = GetStatusBar(top);
	if (statusBar) {
		PAINTSTRUCT ps;
		HDC hdc = ::GetDC(statusBar);
		RECT rc;
		GetClientRect(statusBar, &rc);
		_bstr_t bstrt(message);
		DrawString (hdc, &rc,  bstrt);
//		FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
		::ShowWindow(statusBar, SW_SHOW);
		::ReleaseDC(statusBar, hdc);
	}
*/
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
	BSTR BSTR_message = SysAllocString(Cstr_message);
	pIHTMLWindow2->put_status(BSTR_message);
	SysFreeString(BSTR_message);
	pIHTMLWindow2->Release();
}

STDMETHODIMP
Cieambulant::startPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->play();
		return S_OK;
	} else
		return E_FAIL;
}

STDMETHODIMP
Cieambulant::stopPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->stop();
		return S_OK;
	} else
		return E_FAIL;
}

STDMETHODIMP
Cieambulant::restartPlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->stop();
		m_ambulant_player->play();
		return S_OK;
	} else
		return E_FAIL;
}

STDMETHODIMP
Cieambulant::pausePlayer( void) {
	if (m_ambulant_player != NULL) {
		m_ambulant_player->pause();
		return S_OK;
	} else
		return E_FAIL;
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
	} else
		return E_FAIL;
}
