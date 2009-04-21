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

#ifdef	XP_WIN32
#include <cstddef>		   // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#define _PTRDIFF_T_DEFINED
#include <windows.h>
#include <windowsx.h>
#endif//XP_WIN32

#include "plugin.h"
#include "nsScriptablePeer.h"
#include "nsIServiceManager.h"
#include "nsISupportsUtils.h" // some usefule macros are defined here
#ifdef WITH_GTK
#include "gtk_mainloop.h"
#endif
#ifdef WITH_CG
#include "cg_mainloop.h"
#endif
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/preferences.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;


//////////////////////////////////////
//
// general identification of this plugin and what it does
//

#define PLUGIN_NAME "ambulant plugin"
#define PLUGIN_DESCRIPTION "W3C Smil 3.0 multimedia player"
#ifndef	XP_WIN32
extern "C" {
#endif//XP_WIN32
char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";
#ifdef	XP_WIN32
extern "C" {
#endif//XP_WIN32

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

NPError
NS_PluginGetValue(NPPVariable aVariable, void *aValue)
// 	Retrieve values from the plugin for the Browser. 
{
    NPError err = NPERR_NO_ERROR;
	AM_DBG fprintf(stderr, "NS_PluginGetValue(%d)\n", (int)aVariable);
    switch (aVariable) {
        case NPPVpluginNameString:
            *((char **)aValue) = PLUGIN_NAME;
            break;
        case NPPVpluginDescriptionString:
            *((char **)aValue) = PLUGIN_DESCRIPTION;
            break;
    	case NPPVpluginNeedsXEmbed:
            *((PRBool *)aValue) = PR_TRUE;
            break;
        default:
            err = NPERR_INVALID_PARAM;
    }
    return err;
}


/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase*
NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
	AM_DBG fprintf(stderr, "NS_NewPluginInstance(0x%x)\n", aCreateDataStruct);
    if(!aCreateDataStruct)
        return NULL;

    nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
    if (plugin)
        plugin->mCreateData = *aCreateDataStruct;
    AM_DBG fprintf(stderr, "NS_NewPluginInstance: created 0x%x.\n",plugin);
    return plugin;
}

void 
NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
	AM_DBG fprintf(stderr, "NS_DestroyPluginInstance(0x%x)\n", aPlugin);
    if(aPlugin)
        delete (nsPluginInstance *)aPlugin;
}
} /* extern "C" */

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(NPP aInstance)
: nsPluginInstanceBase(),
  mInstance(aInstance),
  mInitialized(FALSE),
  m_ambulant_player(NULL),
  m_mainloop(NULL),
#ifdef  MOZ_X11
  display(NULL),
#endif//MOZ_X11
  mScriptablePeer(NULL)
{
	AM_DBG fprintf(stderr, "nsPluginInstance::nsPluginInstance(0x%x)\n", aInstance);
	s_last_instance = aInstance;
}

nsPluginInstance::~nsPluginInstance()
{
  // mScriptablePeer may be also held by the browser 
  // so releasing it here does not guarantee that it is over
  // we should take precaution in case it will be called later
  // and zero its mPlugin member
	AM_DBG fprintf(stderr, "nsPluginInstance::~nsPluginInstance(0x%x)\n", (void*)this);
	if (mScriptablePeer) {
		mScriptablePeer->SetInstance(NULL);
		NS_IF_RELEASE(mScriptablePeer);
	}
	if (m_ambulant_player) {
		m_ambulant_player->stop();
		delete m_ambulant_player;
		m_ambulant_player = NULL;
	}
}
#ifdef	XP_WIN32
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
static  LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static  WNDPROC lpOldProc = NULL;
#endif//XP_WIN32

NPBool
nsPluginInstance::init(NPWindow* aWindow)
{
	AM_DBG fprintf(stderr, "nsPluginInstance::init(0x%x)\n", aWindow);
    if(aWindow == NULL)
		return FALSE;
	    mNPWindow = aWindow;
    NPError nperr = NPN_GetValue(mInstance, NPNVWindowNPObject, &mNPWindow);
	if (nperr != NPERR_NO_ERROR)
		return FALSE;
    // Start by saving the NPWindow for any Ambulant plugins (such as SMIL State)
	ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
	void *edptr = pe->get_extra_data("npapi_extra_data");
	if (edptr) {
		*(NPWindow**)edptr = mNPWindow;
		AM_DBG fprintf(stderr, "nsPluginInstance::init: setting npapi_extra_data(0x%x) to NPWindow 0x%x\n", edptr, mNPWindow);
	} else {
		AM_DBG fprintf(stderr, "AmbulantWebKitPlugin: Cannot find npapi_extra_data, cannot communicate NPWindow\n");
	}
#ifdef	MOZ_X11
    this->window = (Window) aWindow->window;
    NPSetWindowCallbackStruct *ws_info =
    	(NPSetWindowCallbackStruct *)aWindow->ws_info;
    this->display = ws_info->display;
    width = aWindow->width;
    height = aWindow->height;
#endif/*MOZ_X11*/
    long long ll_winid = reinterpret_cast<long long>(aWindow->window);
    int i_winid = static_cast<int>(ll_winid);
#ifdef WITH_GTK
    GtkWidget* gtkwidget = GTK_WIDGET(gtk_plug_new((GdkNativeWindow) i_winid));
//  gtk_widget_set_parent(gtkwidget, gtk_plug_new((GdkNativeWindow)aWindow->window));
//  gtk_window_set_resizable(gtkwidget, true); 	
  	gtk_widget_set_size_request(gtkwidget, width, height);
//  	gtk_widget_set_uposition(gtkwidget, 240, 320);
#endif // WITH_GTK
#ifdef	XP_WIN32
	m_hwnd = (HWND)aWindow->window;
	if(m_hwnd == NULL)
		return FALSE;
	// subclass window so we can intercept window messages and
	// do our drawing to it
	lpOldProc = SubclassWindow(m_hwnd, (WNDPROC)PluginWinProc);

	// associate window with our nsPluginInstance object so we can access 
	// it in the window procedure
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);
#endif//XP_WIN32
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
        file_str = strdup(arg_url.get_file().c_str());
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
#ifdef WITH_GTK
    gtk_gui* m_gui = new gtk_gui((char*) gtkwidget, file_str);
    m_mainloop = new gtk_mainloop(m_gui);
    if (file_str) 
        free((void*)file_str);
	m_logger = lib::logger::get_logger();
    m_ambulant_player = m_mainloop->get_player();
    if (m_ambulant_player == NULL)
        return false;
    m_ambulant_player->start();
    gtk_widget_show_all (gtkwidget);
	gtk_widget_realize(gtkwidget);
#endif // WITH_GTK
#ifdef WITH_CG
	void *view = NULL;
	m_mainloop = new cg_mainloop(file_str, view, false, NULL);
	m_logger = lib::logger::get_logger();
    m_ambulant_player = m_mainloop->get_player();
    if (m_ambulant_player == NULL)
        return false;
    m_ambulant_player->start();
#endif // WITH_CG
#ifdef	XP_WIN32
	m_player_callbacks.set_os_window(m_hwnd);
	m_ambulant_player = new ambulant::gui::dx::dx_player(m_player_callbacks, NULL, m_url);
//X	m_ambulant_player->set_state_component_factory(NULL); // XXXJACK DEBUG!!!!
	if (m_ambulant_player) {
		if ( ! get_player()) {
			delete m_ambulant_player;
			m_ambulant_player = NULL;
		} else 
			m_ambulant_player->play();
	}
	mInitialized = TRUE;
    return TRUE;
#else //XP_WIN32
	mInitialized = true;
    return true;
#endif//XP_WIN32
}

void nsPluginInstance::shut()
{
	AM_DBG fprintf(stderr, "nsPluginInstance::shut()\n");
#ifdef	XP_WIN32
    // subclass it back
    SubclassWindow(m_hwnd, lpOldProc);
    m_hwnd = NULL;
    mInitialized = FALSE;
#else // ! XP_WIN32
    if (m_mainloop)
        delete m_mainloop;
    m_mainloop = NULL;
    m_ambulant_player = NULL; // deleted by mainloop
#endif// ! XP_WIN32
    mInitialized = false;
}

NPBool nsPluginInstance::isInitialized()
{
	return mInitialized;
}

/// Get the location of the html document.
/// In javascript this is simply document.location.href. In C it's the
/// same, but slightly more convoluted:-)
char* nsPluginInstance::get_document_location()
{
    char *id = "ambulant::nsPluginInstance::getLocation";
	AM_DBG fprintf(stderr, "nsPluginInstance::get_document_location()\n");
    char *rv = NULL;

	// Get document
	NPVariant npvDocument;
	bool ok = NPN_GetProperty(mInstance, (NPObject*)mNPWindow, NPN_GetStringIdentifier("document"), &npvDocument);
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

/* glue code */
// this macro implements AddRef(), Release() and QueryInterface() members
NS_IMPL_ISUPPORTS1(nsPluginInstance, npambulant)

// this will start AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::StartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	get_player()->start();
	return NS_OK;
}

// this will stop AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::StopPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StopPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	get_player()->stop();
	return NS_OK;
}

// this will restart AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::RestartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::RestartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	get_player()->stop();
	get_player()->start();
	return NS_OK;
}

// this will resume AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::ResumePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::ResumePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	get_player()->resume();
	return NS_OK;
}

// this will pause AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::PausePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::PausePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	get_player()->pause();
	return NS_OK;
}

// this will query the 'done' flag of AmbulantPlayer
NS_IMETHODIMP nsPluginInstance::IsDone(PRBool *isdone)
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance:IsDone()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	*isdone = get_player()->is_done();
	return NS_OK;
}

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
	AM_DBG fprintf(stderr, "nsPluginInstance::GetValue(%d)\n", (int)aVariable);
    NPError rv = NPERR_NO_ERROR;


    if (aVariable == NPPVpluginScriptableInstance) {
        // addref happens in getter, so we don't addref here
        nsScriptablePeer * scriptablePeer = getScriptablePeer();
        if (scriptablePeer) {
            *(void**)aValue = (void*)scriptablePeer;
        } else
            rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    else if (aVariable == NPPVpluginScriptableIID
             || aVariable ==  NPPVpluginScriptableInstance) {
        static nsIID scriptableIID = NPAMBULANT_IID;
        nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
        if (ptr) {
            *ptr = scriptableIID;
        *(nsIID **)aValue = ptr;
        } else
            rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    else if (aVariable ==  NPPVpluginNeedsXEmbed) {
	    *(PRBool *) aValue = PR_TRUE;
	    rv = NPERR_NO_ERROR;
    } else  {
        *(void**) aValue = NULL;
//        rv = NPERR_INVALID_PARAM;
    }
    return rv;
}

// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
{
    AM_DBG fprintf(stderr, "nsPluginInstance::getScriptablePeer()\n");
    if (!mScriptablePeer) {
        mScriptablePeer = new nsScriptablePeer(this);
        if(!mScriptablePeer)
            return NULL;
    }
    // add reference for the caller requesting the object
	// and for ourself
    NS_ADDREF(mScriptablePeer);
    NS_ADDREF(mScriptablePeer);
    return mScriptablePeer;
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


NPP nsPluginInstance::s_last_instance = NULL;

void
nsPluginInstance::display_message(int level, const char *message) {
	if (s_last_instance)
		NPN_Status(s_last_instance, message);
}

// platform/toolkit specifixc functions

#ifdef XP_WIN32

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
	return NULL; // not implemented, but needs to be declared
}
#endif//XP_WIN32

//const char* ambulant::get_version() { return "0";}

#ifdef WITH_GTK
// some fake gtk_gui functions needed by gtk_mainloop
void gtk_gui::internal_message(int, char*) {}
GtkWidget* gtk_gui::get_document_container() { return m_documentcontainer; }
//XXXX FIXME fake gtk_gui constructor 1st arg is used as GtkWindow, 2nd arg as smilfile
gtk_gui::gtk_gui(const char* s, const char* s2) {
    memset (this, 0, sizeof(gtk_gui));
    m_toplevelcontainer = (GtkWindow*) s;
    m_documentcontainer = gtk_drawing_area_new();
    gtk_widget_hide(m_documentcontainer);
//XXXX FIXME vbox only needed to give	m_documentcontainer a parent widget at *draw() callback time
    m_guicontainer = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));
    gtk_box_pack_start (GTK_BOX(m_guicontainer), m_documentcontainer, TRUE, TRUE, 0);
//XXXX not used:  m_guicontainer = menubar = NULL;
//XXXX FIXME <EMBED src="xxx" ../> attr value is 2nd contructor arg.
    m_smilfilename = s2;
    main_loop = g_main_loop_new(NULL, FALSE);
}

gtk_gui::~gtk_gui() {
    g_object_unref (G_OBJECT (main_loop));
}
#endif // WITH_GTK
