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

#include <windows.h>
#include <windowsx.h>

#include "plugin.h"
#include "nsIServiceManager.h"
#include "nsISupportsUtils.h" // some usefule macros are defined here

#define AMBULANT_FIREFOX_PLUGIN
#ifdef AMBULANT_FIREFOX_PLUGIN
#endif // AMBULANT_FIREFOX_PLUGIN

//////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
  if (plugin)
	 plugin->mCreateData = *aCreateDataStruct;
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}

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
  mhWnd = NULL;
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

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
  if(aWindow == NULL)
    return FALSE;

  mhWnd = (HWND)aWindow->window;
  if(mhWnd == NULL)
    return FALSE;

  // subclass window so we can intercept window messages and
  // do our drawing to it
  lpOldProc = SubclassWindow(mhWnd, (WNDPROC)PluginWinProc);

  // associate window with our nsPluginInstance object so we can access 
  // it in the window procedure
  SetWindowLong(mhWnd, GWL_USERDATA, (LONG)this);

  mInitialized = TRUE;
  return TRUE;
}

void nsPluginInstance::shut()
{
  // subclass it back
  SubclassWindow(mhWnd, lpOldProc);
  mhWnd = NULL;
  mInitialized = FALSE;
}

NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}

#ifdef AMBULANT_FIREFOX_PLUGIN

const char * nsPluginInstance::getVersion()
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

// this will start AmbulantPLayer
void nsPluginInstance::startPlayer()
{
	if (m_ambulant_player)
		m_ambulant_player->play();
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
}

// this will stop AmbulantPLayer
void nsPluginInstance::stopPlayer()
{
	if (m_ambulant_player)
		m_ambulant_player->stop();
}

// this will restart AmbulantPLayer
void nsPluginInstance::restartPlayer()
{
//  InvalidateRect(mhWnd, NULL, TRUE);
//  UpdateWindow(mhWnd);
	if (m_ambulant_player) {
		m_ambulant_player->stop();
		m_ambulant_player->play();
	}
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
}

// this will resume AmbulantPLayer
void nsPluginInstance::resumePlayer()
{
//  InvalidateRect(mhWnd, NULL, TRUE);
//  UpdateWindow(mhWnd);
	if (m_ambulant_player)
		m_ambulant_player->play();
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
}

// this will pause AmbulantPLayer
void nsPluginInstance::pausePlayer()
{
//  InvalidateRect(mhWnd, NULL, TRUE);
//  UpdateWindow(mhWnd);
	if (m_ambulant_player)
		m_ambulant_player->pause();
}

#else // AMBULANT_FIREFOX_PLUGIN

//XXXX
// this will force to draw a version string in the plugin window
void nsPluginInstance::showVersion()
{
  const char *ua = NPN_UserAgent(mInstance);
  strcpy(mString, ua);
  InvalidateRect(mhWnd, NULL, TRUE);
  UpdateWindow(mhWnd);
}

// this will clean the plugin window
void nsPluginInstance::clear()
{
  strcpy(mString, "");
  InvalidateRect(mhWnd, NULL, TRUE);
  UpdateWindow(mhWnd);
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
NPError	nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  NPError rv = NPERR_NO_ERROR;

  if (aVariable == NPPVpluginScriptableInstance) {
    // addref happens in getter, so we don't addref here
    AmbulantFFplugin * scriptablePeer = getScriptablePeer();
    if (scriptablePeer) {
      *(nsISupports **)aValue = scriptablePeer;
    } else
      rv = NPERR_OUT_OF_MEMORY_ERROR;
  }
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

// ==============================
// ! Scriptability related code !
// ==============================
//
// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
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

#ifndef AMBULANT_FIREFOX_PLUGIN
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_PAINT:
      {
        // draw a frame and display the string
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));

        // get our plugin instance object and ask it for the version string
        nsPluginInstance *plugin = (nsPluginInstance *)GetWindowLong(hWnd, GWL_USERDATA);
        if (plugin)
          DrawText(hdc, plugin->mString, strlen(plugin->mString), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        else {
          char string[] = "Error occured";
          DrawText(hdc, string, strlen(string), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        EndPaint(hWnd, &ps);
      }
      break;
    default:
      break;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}
#else // AMBULANT_FIREFOX_PLUGIN

static ambulant_player_callbacks s_ambulant_player_callbacks;

static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
				if ( ! plugin->m_ambulant_player) {
					// get our plugin instance object and ask it for the value of the "src" string
					const char * s = plugin->getValue("src");
					if (s == NULL)
						break; //XXXX Error msg "no src attribute"
					std::string str(s);
					if (str.find("://") != std::string.npos)
						plugin->m_url = ambulant::net::url::from_url(str);
					else plugin->m_url = ambulant::net::url::from_filename(str);
					plugin->m_hwnd = hWnd;
					plugin->m_player_callbacks.set_os_window(hWnd);
					plugin->m_ambulant_player = new ambulant::gui::dx::dx_player(plugin->m_player_callbacks, NULL, plugin->m_url);
					if (plugin->m_ambulant_player)
						plugin->m_ambulant_player->play();
				} else {
					if (plugin->m_ambulant_player)
						plugin->m_ambulant_player->redraw(hWnd, hdc);
				}
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


#endif // AMBULANT_FIREFOX_PLUGIN
