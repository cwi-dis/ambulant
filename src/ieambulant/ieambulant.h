// ieambulant.h : Declaration of the Cieambulant
#ifndef __ieambulant_H_
#define __ieambulant_H_
#include "resource.h"	   // main symbols
#include <atlctl.h>
// ambulant player includes
#include "ambulant/version.h"
#include "ambulant/common/player.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/logger.h"
#include <windowsx.h>

#include "ambulant/gui/d2/d2_player.h"
typedef ambulant::gui::d2::d2_player ambulant_gui_player;
typedef ambulant::gui::d2::d2_player_callbacks gui_callbacks; //XX from MmView.cpp
typedef ambulant::gui::d2::d2_player_callbacks ambulant_baseclass_player_callbacks;

#include <ambulant/net/url.h>
#include "AmbulantActiveX.h"
#include "_IieambulantEvents_CP.H"

class ambulant_player_callbacks : public ambulant_baseclass_player_callbacks
{
public:
	ambulant_player_callbacks();
	void set_os_window(HWND hwnd);
	HWND new_os_window();
	SIZE get_default_size();
	void destroy_os_window(HWND);
	html_browser *new_html_browser(int left, int top, int width, int height);
	HWND m_hwnd;
};
//XXX end copied from npambulant.h


/////////////////////////////////////////////////////////////////////////////
// Cieambulant
class ATL_NO_VTABLE Cieambulant :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<Cieambulant, Iieambulant, &IID_Iieambulant, &LIBID_AmbulantACTIVEXLib>,
	public CComControl<Cieambulant>,
	public IPersistStreamInitImpl<Cieambulant>,
	public IPersistPropertyBagImpl<Cieambulant>,
	public IObjectSafetyImpl<Cieambulant, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
	public IOleControlImpl<Cieambulant>,
	public IOleObjectImpl<Cieambulant>,
	public IOleInPlaceActiveObjectImpl<Cieambulant>,
	public IViewObjectExImpl<Cieambulant>,
	public IOleInPlaceObjectWindowlessImpl<Cieambulant>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<Cieambulant>,
	public IPersistStorageImpl<Cieambulant>,
	public ISpecifyPropertyPagesImpl<Cieambulant>,
	public IQuickActivateImpl<Cieambulant>,
	public IDataObjectImpl<Cieambulant>,
	public IProvideClassInfo2Impl<&CLSID_ieambulant, &DIID__IieambulantEvents, &LIBID_AmbulantACTIVEXLib>,
	public IPropertyNotifySinkCP<Cieambulant>,
	public CComCoClass<Cieambulant, &CLSID_ieambulant>
{
public:
	CContainedWindow m_ctlStatic;


	Cieambulant() :
		m_ctlStatic(_T("Static"), this, 1),
		m_autostart(true)
	{
		m_OldWindow = NULL;
		m_hwnd = NULL;
		m_site = NULL;
		m_lpOldProc = NULL;
		m_logger = NULL;
		m_ambulant_player = NULL;
		m_bWindowOnly = TRUE;
		m_bstrCaption = "playing";
		//m_url = ambulant::net::url::from_filename("file://C:\\Documents and Settings\\kees.AMBULANT-DEV\\My Documents\\Ambulant\\ambulant\\Extras\\Welcome\\Welcome.smil");
	}
	~Cieambulant() {
		if (m_ambulant_player) {
			m_ambulant_player->stop();
			delete m_ambulant_player;
			m_ambulant_player = NULL;
			ambulant_gui_player::cleanup();//causes crash
		}
	}
DECLARE_REGISTRY_RESOURCEID(IDR_ieambulant)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(Cieambulant)
	COM_INTERFACE_ENTRY(Iieambulant)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistPropertyBag)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(Cieambulant)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// AmbulantPlayer plugin control properties
	PROP_ENTRY("type", 1, CLSID_NULL)
	PROP_ENTRY("src", 2, CLSID_NULL)
	PROP_ENTRY("autostart", 3, CLSID_NULL)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(Cieambulant)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(Cieambulant)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_PAINT, PluginWinProc)
	MESSAGE_HANDLER(WM_MOUSEMOVE, PluginWinProc)
//	MESSAGE_HANDLER(WM_MOUSEWHEEL, Plugin)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, PluginWinProc)
CHAIN_MSG_MAP(CComControl<Cieambulant>)
ALT_MSG_MAP(1)
	// Replace this with message map entries for superclassed Static
END_MSG_MAP()
// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] =
		{
			&IID_Iieambulant,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}


	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = CComControl<Cieambulant>::OnSetFocus(uMsg, wParam, lParam, bHandled);
		if (m_bInPlaceActive)
		{
			DoVerbUIActivate(&m_rcPos,	NULL);
			if(!IsChild(::GetFocus()))
				m_ctlStatic.SetFocus();
		}
		return lRes;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rc;
		GetWindowRect(&rc);
		rc.right -= rc.left;
		rc.bottom -= rc.top;
		rc.top = rc.left = 0;
		//m_ctlStatic.Create(m_hWnd, rc);
		m_ctlStatic.Create( m_hWnd,
			rc,
			_T("Static"),
			WS_CHILD|WM_PAINT);
		return 0;
	}
	LRESULT CALLBACK PluginWinProc(UINT, WPARAM, LPARAM, BOOL& );
	STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip)
	{
		IOleInPlaceObjectWindowlessImpl<Cieambulant>::SetObjectRects(prcPos, prcClip);
		int cx, cy;
		cx = prcPos->right - prcPos->left;
		cy = prcPos->bottom - prcPos->top;
		::SetWindowPos(m_ctlStatic.m_hWnd, NULL, 0,
			0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE /*XXX | SWP_SHOWWINDOW */);
		return S_OK;
	}

	STDMETHOD(SetClientSite)(LPOLECLIENTSITE pSite);
	HRESULT OnDraw(ATL_DRAWINFO& di);
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

public:
	OLE_COLOR m_clrBackColor;
	CComBSTR m_bstrCaption;
//	CComPtr<IFontDisp> m_pFont;
	OLE_COLOR m_clrForeColor;

// added for Ambulant player
	HRESULT updatePlayer();
public:
	CComBSTR m_bstrType;
	CComBSTR m_bstrSrc;
	CComBSTR m_bstrAutostart;

	ambulant_player_callbacks m_player_callbacks;
	LPOLECLIENTSITE m_site;
	HRESULT get_document_url();
	HWND m_hwnd;
	WNDPROC m_lpOldProc;
	LONG m_OldWindow;
	ambulant_gui_player* m_ambulant_player;
	ambulant::lib::logger* m_logger;
	ambulant::net::url m_url;
	ambulant::net::url m_base_url;
	bool m_autostart;
	int m_cursor_id;
	ambulant::common::player* get_player() { return m_ambulant_player->get_player();}
/*
from http://www.ozhiker.com/electronics/vcpp/ActiveXControl.htm
To make ATL think your CWnd derived class is a CWindow derived class, add the following method:
*/
static LPCTSTR GetWndClassName()
{
	return NULL;
}
// added for Ambulant
//XX IPersistPropertyBag override
//XX	STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
public:
	STDMETHOD(get_autostart)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_autostart)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_src)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_src)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_mimeType)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_mimeType)(/*[in]*/ BSTR newVal);
	STDMETHOD(startPlayer)(void);
	STDMETHOD(stopPlayer)(void);
	STDMETHOD(restartPlayer)(void);
	STDMETHOD(pausePlayer)(void);
	STDMETHOD(resumePlayer)(void);
	STDMETHOD(isDone)(/*[out, retval]*/ BOOL *pVal);
public:
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
//	LRESULT Plugin(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

extern "C" {
void ieambulant_display_message(int level, const char *message);
const char* get_last_log_message();
extern LPOLECLIENTSITE s_site;
extern int s_ref_count;
};

#endif //__ieambulant_H_
