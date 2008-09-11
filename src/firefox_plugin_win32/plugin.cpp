/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <cstddef> //XXXX Hack for ptrdiff_t  
#include <windows.h>
#include <windowsx.h>
#define ptrdiff_t long int //XXXX Hack for ptrdiff_t in xulrunner-sdk (GeckoSDK 1.9 and Vc7)
#include "plugin.h"
//#undef ptrdiff_t
#include "nsScriptablePeer.h"
#include "nsIServiceManager.h"
#include "nsISupportsUtils.h" // some usefule macros are defined here

#define AMBULANT_FIREFOX_PLUGIN
#ifdef AMBULANT_FIREFOX_PLUGIN
#endif // AMBULANT_FIREFOX_PLUGIN

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;


NPP nsPluginInstance::s_lastInstance = NULL;

//////////////////////////////////////
//
// general identification of this plugin and what it does
//

#define PLUGIN_NAME "ambulant plugin"
#define PLUGIN_DESCRIPTION "W3C Smil 3.0 multimedia player"
char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";
extern "C" {
//////////////////////////////////////
//
// general initialization and shutdown
//
NPError
NS_PluginInitialize()
{
  return NPERR_NO_ERROR;
}

void
NS_PluginShutdown()
{
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase *
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
  if (plugin)
	 plugin->mCreateData = *aCreateDataStruct;
  return plugin;
}

void
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}

} // extern "C"
////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
  mInstance(aInstance),
  mInitialized(FALSE),
#ifdef AMBULANT_FIREFOX_PLUGIN
  m_ambulant_player(NULL),
  m_cursor_id(0),
#endif // AMBULANT_FIREFOX_PLUGIN
  mScriptablePeer(NULL)
{
  s_lastInstance = mInstance;
  m_hwnd = NULL;
  mString[0] = '\0';
}

nsPluginInstance::~nsPluginInstance()
{
  // mScriptablePeer may be also held by the browser 
  // so releasing it here does not guarantee that it is over
  // we should take precaution in case it will be called later
  // and zero its mPlugin member
	if (mScriptablePeer) {
		mScriptablePeer->SetInstance(NULL);
		NS_IF_RELEASE(mScriptablePeer);
	}
#ifdef AMBULANT_FIREFOX_PLUGIN
	if (m_ambulant_player) {
		m_ambulant_player->stop();
		delete m_ambulant_player;
		m_ambulant_player = NULL;
	}
#endif // AMBULANT_FIREFOX_PLUGIN
}
int
strcasecmp(const char* s1, const char* s2) {
	if (s1 == NULL && s2 == NULL)
		return 0;
	else if (s1 == NULL)
		return -1;
	else if (s2 == NULL)
		return 1;

	while (*s1 != 0 && *s2 != 0) {
		if (toupper(*s1) != toupper(*s2))
			return (toupper(*s1) < toupper(*s2)) ? -1 : 1;
		s1++;
		s2++;
	}
	if (*s1 == 0 && *s2 == 0)
		return 0;
	else if (*s1 == 0)
		return -1;
	else
		return 1;
}

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;

NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
    if(aWindow == NULL)
		return FALSE;
	mNPWindow = aWindow;
	m_hwnd = (HWND)aWindow->window;
	if(m_hwnd == NULL)
		return FALSE;
    NPError nperr = NPN_GetValue(mInstance, NPNVWindowNPObject, &mNPWindow);
	if (nperr != NPERR_NO_ERROR)
		return FALSE;
	// subclass window so we can intercept window messages and
	// do our drawing to it
	lpOldProc = SubclassWindow(m_hwnd, (WNDPROC)PluginWinProc);

	// associate window with our nsPluginInstance object so we can access 
	// it in the window procedure
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);

	assert ( ! m_ambulant_player);
	ambulant::lib::logger::get_logger()->set_show_message(nsPluginInstance::display_message);
	ambulant::lib::logger::get_logger()->show("Ambulant plugin loaded");
	const char* arg_str = NULL;
    if (mCreateData.argc > 1)
		for (int i =0; i < mCreateData.argc; i++) {
// Uncomment next line to see the <EMBED/> attr values	
//          fprintf(stderr, "arg[%i]:%s=%s\n",i,mCreateData.argn[i],mCreateData.argv[i]);
            if (strcasecmp(mCreateData.argn[i],"data") == 0)
                if (arg_str == NULL)
                    arg_str = mCreateData.argv[i];
            if (strcasecmp(mCreateData.argn[i],"src") == 0)
                if (arg_str == NULL)
                    arg_str = mCreateData.argv[i];
		}	
	if (arg_str == NULL)
        return false;
    net::url file_url;
    net::url arg_url = net::url::from_url (arg_str);
    char* file_str = NULL;
    if (arg_url.is_absolute()) {
		file_url = arg_url;
        file_str = strdup(file_url.get_file().c_str());
    } else {
        char* loc_str = get_document_location();
        if (loc_str != NULL) {
            net::url loc_url = net::url::from_url (loc_str);
            file_url = arg_url.join_to_base(loc_url);
            free((void*)loc_str);
        } else {
            file_url = arg_url;
        }
        file_str = strdup(file_url.get_file().c_str());
    }
	m_url = file_url;
	m_player_callbacks.set_os_window(m_hwnd);
	m_ambulant_player = new ambulant::gui::dx::dx_player(m_player_callbacks, NULL, m_url);
	if (m_ambulant_player)
		m_ambulant_player->play();
//	plugin->m_ambulant_player->redraw(hWnd, hdc);
	mInitialized = TRUE;
	return TRUE;
}

void
nsPluginInstance::shut()
{
  // subclass it back
  SubclassWindow(m_hwnd, lpOldProc);
  m_hwnd = NULL;
  mInitialized = FALSE;
}

NPBool
nsPluginInstance::isInitialized()
{
  return mInitialized;
}

#ifdef AMBULANT_FIREFOX_PLUGIN

/// Get the location of the html document.
/// In javascript this is simply document.location.href. In C it's the
/// same, but slightly more convoluted:-)
char*
nsPluginInstance::get_document_location()
{
    char *id = "ambulant::nsPluginInstance::getLocation";
	AM_DBG fprintf(stderr, "nsPluginInstance::get_document_location()\n");
    char *rv = NULL;

	// Get document
	NPIdentifier npidocument = NPN_GetStringIdentifier("document");
	NPVariant npvDocument;
	bool ok = NPN_GetProperty(mInstance, (NPObject*)mNPWindow, npidocument, &npvDocument);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., document, ...) -> %d, 0x%d\n", ok, npvDocument);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvDocument));
	NPObject *document = NPVARIANT_TO_OBJECT(npvDocument);
	assert(document);
	
	// Get document.location
	NPVariant npvLocation;
	ok = NPN_GetProperty(mInstance, document, NPN_GetStringIdentifier("location"), &npvLocation);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., location, ...) -> %d, 0x%d\n", ok, npvLocation);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvLocation));
	NPObject *location = NPVARIANT_TO_OBJECT(npvLocation);
	assert(location);
	
	// Get document.location.href
	NPVariant npvHref;
	ok = NPN_GetProperty(mInstance, location, NPN_GetStringIdentifier("href"), &npvHref);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., href, ...) -> %d, 0x%d\n", ok, npvHref);
	if (!ok) return NULL;
	if (!NPVARIANT_IS_STRING(npvHref)) {
		AM_DBG fprintf(stderr, "get_document_location: document.location.href is not a string\n");
		return NULL;
	}

	// Turn it into a C string.
	// XXXJACK: the memory for the string isn't released...
	NPString href = NPVARIANT_TO_STRING(npvHref);
	rv = (char*) malloc(href.utf8length+1);
	strncpy(rv, href.utf8characters, href.utf8length);
	rv[href.utf8length] = '\0';
	AM_DBG fprintf(stderr, "get_document_location: returning \"%s\"\n", rv);
	
    NPN_ReleaseVariantValue(&npvLocation);
    NPN_ReleaseVariantValue(&npvDocument);
    NPN_ReleaseVariantValue(&npvHref);
    return rv;
}

const char *
nsPluginInstance::getVersion()
{
	return ambulant::get_version();
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
	return NULL; // not implementrd, is it needed?
}

/* glue code */
NS_IMPL_ISUPPORTS1(nsPluginInstance, AmbulantFFplugin)

// this will start AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::StartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->get_player()->start();
	return NS_OK;
}

// this will stop AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::StopPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StopPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->get_player()->stop();
	return NS_OK;
}

// this will restart AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::RestartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::RestartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->get_player()->stop();
	m_ambulant_player->get_player()->start();
	return NS_OK;
}

// this will resume AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::ResumePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::ResumePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->get_player()->resume();
	return NS_OK;
}

// this will pause AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::PausePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::PausePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->get_player()->pause();
	return NS_OK;
}

// this will query the 'done' flag of AmbulantPlayer
NS_IMETHODIMP
nsPluginInstance::IsDone(PRBool *isdone)
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance:IsDone()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	*isdone = m_ambulant_player->get_player()->is_done();
	return NS_OK;
}
#endif // AMBULANT_FIREFOX_PLUGIN

// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to 
// nsScriptablePeer interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError
nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  NPError rv = NPERR_NO_ERROR;

  if (aVariable == NPPVpluginScriptableInstance
	) {
    // addref happens in getter, so we don't addref here
    AmbulantFFplugin * scriptablePeer = getScriptablePeer();
    if (scriptablePeer) {
      *(nsISupports **)aValue = scriptablePeer;
    } else
      rv = NPERR_OUT_OF_MEMORY_ERROR;
  }
#ifdef  XP_WIN
  else if (aVariable  == NPPVpluginScriptableNPObject) {
	  *(void**)aValue = NULL;
  }
#endif//XP_WIN
  else if (aVariable == NPPVpluginScriptableIID) {
    static nsIID scriptableIID = AMBULANTFFPLUGIN_IID;
    nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
    if (ptr) {
        *ptr = scriptableIID;
        *(nsIID **)aValue = ptr;
    } else
      rv = NPERR_OUT_OF_MEMORY_ERROR;
  }

  return rv;
}

// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer*
nsPluginInstance::getScriptablePeer()
{
  if (!mScriptablePeer) {
    mScriptablePeer = new nsScriptablePeer(this);
    if(!mScriptablePeer)
      return NULL;

    NS_ADDREF(mScriptablePeer);
  }

  // add reference for the caller requesting the object
  NS_ADDREF(mScriptablePeer);
  return mScriptablePeer;
}

static ambulant_player_callbacks s_ambulant_player_callbacks;

static LRESULT CALLBACK
PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	nsPluginInstance *plugin = (nsPluginInstance *)GetWindowLong(hWnd, GWL_USERDATA);
	if (plugin)
		switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rc;
				GetClientRect(hWnd, &rc);
				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				EndPaint(hWnd, &ps);
				if (plugin->m_ambulant_player)
					plugin->m_ambulant_player->redraw(hWnd, hdc);
				NPRegion invalid_region = CreateRectRgn(rc.left,rc.top,rc.right,rc.bottom);
				NPN_InvalidateRegion(plugin->getNPP(), invalid_region);
				break;
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
			{
				POINT point;
				point.x=GET_X_LPARAM(lParam);
				point.y=GET_Y_LPARAM(lParam);

				if (plugin->m_ambulant_player) {
					if (msg == WM_MOUSEMOVE) {
						// code copied from MmView.cpp
						int new_cursor_id = plugin->m_ambulant_player->get_cursor(point.x, point.y, hWnd);
//XX					if (new_cursor_id>0) EnableToolTips(TRUE);
//XX					else CancelToolTips();
						if(new_cursor_id != plugin->m_cursor_id) {
							HINSTANCE hIns = 0;
							HCURSOR new_cursor = 0;
							if(new_cursor_id == 0) {
								new_cursor = LoadCursor(hIns, IDC_ARROW); 
							} else {
								new_cursor = LoadCursor(hIns, IDC_HAND); 
							}
							SetClassLongPtr(hWnd, GCLP_HCURSOR, HandleToLong(new_cursor));
							plugin->m_cursor_id = new_cursor_id;
						}
					} else {
						plugin->m_ambulant_player->on_click(point.x, point.y, hWnd);
				}
			}
			break;

		default:
			break;
		}
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

const char* 
nsPluginInstance::getValue(const char* name)
{
	std::string wanted(name);
	int i;
	char** np = mCreateData.argn; 
	char** vp = mCreateData.argv;
	for (i = 0; i < mCreateData.argc; i++) {
		const std::string current((const char*)np[i]);
		if (current == wanted)
			return (const char*) vp[i];
	}
	return NULL;
}

NPP
nsPluginInstance::getNPP()
{
	return mCreateData.instance;
}


void
nsPluginInstance::display_message(int level, const char *message) {
	if (s_lastInstance)
		NPN_Status(s_lastInstance, message);
	// NPN_Status (mInstance, message);
}
