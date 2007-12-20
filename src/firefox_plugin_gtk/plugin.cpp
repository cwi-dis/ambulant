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


#include "plugin.h"
#include "nsIServiceManager.h"
#include "nsISupportsUtils.h" // some usefule macros are defined here
#include "gtk_mainloop.h"
//#define AMBULANT_DATADIR "./share/ambulant"
extern "C" {
char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;";
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

#define PLUGIN_NAME "ambulant plugin"
#define PLUGIN_DESCRIPTION "W3C Smil 3.0 multimedia player"

NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;
#ifdef DEBUG
    char *id = "NS_GetValue";
    fprintf(stderr, "%s: %s=%d (0x%x).\n",id,"aVariable",aVariable,aVariable);
#endif

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
            err = NPERR_GENERIC_ERROR;
    }
    return err;
}


/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
#ifdef DEBUG
    char *id = "NS_NewPluginInstance";
    fprintf(stderr, "%s: %s=0x%x.\n",id,"aCreateDataStruct",aCreateDataStruct);
#endif
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
#ifdef AMBULANT_FIREFOX_PLUGIN
  if (plugin)
	 plugin->mCreateData = *aCreateDataStruct;
#endif // AMBULANT_FIREFOX_PLUGIN
#ifdef DEBUG
    fprintf(stderr, "%s: created %s=0x%x.\n",id,"plugin",plugin);
#endif
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
#ifdef DEBUG
    char *id = "NS_DestroyPluginInstance";
    fprintf(stderr, "%s: %s=0x%x.\n",id,"aPlugin",aPlugin);
#endif
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
}
} /* extern "C" */

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
#ifdef  MOZ_X11
  display(NULL),
#endif//MOZ_X11
  mScriptablePeer(NULL)
{
#ifdef	XP_WIN
  mhWnd = NULL;
#endif/*XP_WIN*/
    strcpy(mString,"Ambulant 1.9 Firefox plugin");
#ifdef DEBUG
    char *id = "nsPluginInstance::nsPluginInstance";
    fprintf(stderr, "%s(%x): %s=%s.\n",id,this,"mString",mString);
#endif
}

nsPluginInstance::~nsPluginInstance()
{
  // mScriptablePeer may be also held by the browser 
  // so releasing it here does not guarantee that it is over
  // we should take precaution in case it will be called later
  // and zero its mPlugin member
#ifdef DEBUG
    char *id = "nsPluginInstance::~nsPluginInstance";
    fprintf(stderr, "%s(%x): %s=%0x%x.\n",id,this,"mScriptablePeer",mScriptablePeer);
#endif
	if (mScriptablePeer) {
		mScriptablePeer->SetInstance(NULL);
		NS_IF_RELEASE(mScriptablePeer);
	}
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
#ifdef DEBUG
    char *id = "nsPluginInstance::init";
    NPSetWindowCallbackStruct *ws_info;
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"aWindow",aWindow);
#endif
#ifdef	XP_UNIX
#ifdef	MOZ_X11
    this->window = (Window) aWindow->window;
    ws_info = (NPSetWindowCallbackStruct *)aWindow->ws_info;
    this->display = ws_info->display;
    width = aWindow->width;
    height = aWindow->height;
#endif/*MOZ_X11*/
#ifdef AMBULANT_FIREFOX_PLUGIN
    GtkWidget* gtkwidget = GTK_WIDGET(gtk_plug_new((GdkNativeWindow)aWindow->window));
//  gtk_widget_set_parent(gtkwidget, gtk_plug_new((GdkNativeWindow)aWindow->window));
//  gtk_window_set_resizable(gtkwidget, true); 	
  	gtk_widget_set_size_request(gtkwidget, width, height);
//  	gtk_widget_set_uposition(gtkwidget, 240, 320);	
    for (int i =0; i < mCreateData.argc; i++) {
        fprintf(stderr, "arg[%i]: =%s\n",i,mCreateData.argn[i],mCreateData.argv[i]);
    }
    char* filename = NULL;
    if (mCreateData.argc > 1)
    for (int i =0; i < mCreateData.argc; i++) {
// Uncomment next line to see the <EMBED/> attr values	
//            fprintf(stderr, "arg[%i]:%s=%s\n",i,mCreateData.argn[i],mCreateData.argv[i]);
            if (strcasecmp(mCreateData.argn[i],"src") == 0)
                filename = mCreateData.argv[i];
    }
    if (filename == NULL)
        return false;
    gtk_gui* m_gui = new gtk_gui((char*) gtkwidget, filename);
    m_mainloop = new gtk_mainloop(m_gui);
	m_logger = lib::logger::get_logger();
    m_ambulant_player = m_mainloop->get_player();
    if (m_ambulant_player == NULL)
        return false;
    m_ambulant_player->start();
    gtk_widget_show_all (gtkwidget);
	gtk_widget_realize(gtkwidget);
#endif // AMBULANT_FIREFOX_PLUGIN
#endif/*XP_UNIX*/
    return mInitialized = true;
}

void nsPluginInstance::shut()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::shut";
    fprintf(stderr, "%s(%x).\n",id,"this",this,"<empty>",0);
#endif
    if (m_mainloop)
        delete m_mainloop;
    m_mainloop = NULL;
    m_ambulant_player = NULL; // deleted by mainloop
}

NPBool nsPluginInstance::isInitialized()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::isInitialized";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"mInitialized",mInitialized);
#endif
  return mInitialized;
}

#ifdef AMBULANT_FIREFOX_PLUGIN

const char * nsPluginInstance::getVersion()
{
#ifdef DEBUG
    char *id = " nsPluginInstance::getVersion";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"ambulant::get_version",ambulant::get_version);
#endif
	return ambulant::get_version();
}

#ifdef	XP_WIN
ambulant_player_callbacks::ambulant_player_callbacks()
:	m_hwnd(NULL)
{
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::ambulant_player_callbacks";
    fprintf(stderr, "%s.\n",id);
#endif
}

void
ambulant_player_callbacks::set_os_window(HWND hwnd)
{
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::set_os_window";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"<empty>",0);
#endif
	m_hwnd = hwnd;
}


HWND 
ambulant_player_callbacks::new_os_window()
{
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::new_os_window";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"<empty>",0);
#endif
	return m_hwnd;
}

SIZE
ambulant_player_callbacks::get_default_size()
{
	SIZE size;
	size.cx = ambulant::common::default_layout_width;
	size.cy = ambulant::common::default_layout_height;
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::get_default_size";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"size.cx",size.cx);
#endif
	return size;
}

void
ambulant_player_callbacks::destroy_os_window(HWND hwnd)
{
	m_hwnd = NULL;
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::destroy_os_window";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"<empty>",0);
#endif
}

html_browser*
ambulant_player_callbacks::new_html_browser(int left, int top, int width, int height)
{
#ifdef DEBUG
    char *id = "ambulant_player_callbacks::new_html_browser";
    fprintf(stderr, "%s(%x): %s=%d: %s=%d.\n",id,this,"width",width,"height",height);
#endif
	return NULL; // not implementrd, is it needed?
}
#endif/*XP_WIN*/

// this will start AmbulantPLayer
void nsPluginInstance::startPlayer()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::startPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_ambulant_player",m_ambulant_player);
#endif
	if (m_ambulant_player)
		m_ambulant_player->start();
#ifdef	XP_WIN
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
#endif/*XP_WIN*/
}

// this will stop AmbulantPLayer
void nsPluginInstance::stopPlayer()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::stopPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_ambulant_player",m_ambulant_player);
#endif
	if (m_ambulant_player)
		m_ambulant_player->stop();
}

// this will restart AmbulantPLayer
void nsPluginInstance::restartPlayer()
{
//  InvalidateRect(mhWnd, NULL, TRUE);
//  UpdateWindow(mhWnd);
#ifdef DEBUG
    char *id = "nsPluginInstance::restartPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_ambulant_player",m_ambulant_player);
#endif
	if (m_ambulant_player) {
		m_ambulant_player->stop();
		m_ambulant_player->start();
	}
#ifdef	XP_WIN
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
#endif/*XP_WIN*/
}

// this will resume AmbulantPLayer
void nsPluginInstance::resumePlayer()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::resumePlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_ambulant_player",m_ambulant_player);
#endif
//  InvalidateRect(mhWnd, NULL, TRUE);
//  UpdateWindow(mhWnd);
	if (m_ambulant_player)
        m_ambulant_player->resume();
#ifdef	XP_WIN
	InvalidateRect(mhWnd, NULL, TRUE);
	UpdateWindow(mhWnd);
#endif/*XP_WIN*/
}

// this will pause AmbulantPLayer
void nsPluginInstance::pausePlayer()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::pausePlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_ambulant_player",m_ambulant_player);
#endif
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
#ifdef	XP_UNIX
    GC gc;
    unsigned int h,w;
    int x,y,l;
    const char *string;
#endif/*XP_UNIX*/
#ifdef DEBUG
    char *id = "nsPluginInstance::showVersion";
    fprintf(stderr, "%s(%x) %s=%s.\n",id,this,"mString",mString);
#endif
  const char *ua = NPN_UserAgent(mInstance);
  strcpy(mString, ua);
#ifdef	XP_WIN
  InvalidateRect(mhWnd, NULL, TRUE);
  UpdateWindow(mhWnd);
#endif/*XP_WIN*/
#ifdef	XP_UNIX
  if (this->display == NULL) {
#ifdef DEBUG
            fprintf(stderr, "%s(%x) called with display=0x%x, window=0x%x.\n",id,this,this->display,this->window);
#endif        
    	return;
    }      
    gc = XCreateGC(this->display, this->window, 0, NULL);

    /* draw a rectangle */
    h = this->height/2;
    w = 3 * this->width/4;
    x = 0; /* (this->width - w)/2;  center */
    y = h/2;
    XDrawRectangle(this->display, this->window, gc, x, y, w, h);

    /* draw a string */
    string = this->mString;
    if (string && *string)
    {
        l = strlen(string);
        x += this->width/10;
        XDrawString(this->display, this->window, gc, x, this->height/2, string, l);
    }
    XFreeGC(this->display, gc);
#endif/*XP_UNIX*/
}

// this will clean the plugin window
void nsPluginInstance::clear()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::clear";
    fprintf(stderr, "%s(%x): %s=%0x%x.\n",id,this,"<empty>",0);
#endif
  strcpy(mString, "");
#ifdef	XP_WIN
  InvalidateRect(mhWnd, NULL, TRUE);
  UpdateWindow(mhWnd);
#endif/*XP_WIN*/
#ifdef	XP_UNIX

#endif/*XP_UNIX*/
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
#ifdef DEBUG
    char *id = "nsPluginInstance::GetValue";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"aVariable",aVariable);
#endif
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
    static nsIID scriptableIID = NS_IAMBULANTPLUGIN_IID;
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
  } else  *(void**) aValue = NULL;
  return rv;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
{
#ifdef DEBUG
    char *id = "nsPluginInstance::getScriptablePeer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mScriptablePeer",mScriptablePeer);
#endif
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
#ifdef DEBUG
    char *id = "PluginWinProc";
    fprintf(stderr, "%s(static): %s=%d.\n",id,"<empty>",0);
#endif
  switch (msg) {
#ifdef	XP_WIN
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
#endif/*XP_WIN*/
    default:
      break;
  }

#ifdef	XP_WIN
  return DefWindowProc(hWnd, msg, wParam, lParam);
#else /* ! XP_WIN*/
  return NULL;
#endif/*XP_WIN*/
}
#else // AMBULANT_FIREFOX_PLUGIN

#ifdef	XP_WIN
static ambulant_player_callbacks s_ambulant_player_callbacks;


static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    char *id = "PluginWinProc";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"<empty>",0);
#endif
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
#endif/*XP_WIN*/

const char* 
nsPluginInstance::getValue(const char* name)
{
#ifdef DEBUG
    char *id = "nsPluginInstance::getValue";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"name",name);
#endif
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

/* glue code */
NS_IMPL_ISUPPORTS1(nsPluginInstance, nsIAmbulantPlugin)

NS_IMETHODIMP nsPluginInstance::StartPlayer() { 
#ifdef DEBUG
    char *id = "nsPluginInstance::StartPlayer";
    fprintf(stderr, "%s(%x.\n",id,this);
#endif
startPlayer(); return NS_OK; }
NS_IMETHODIMP nsPluginInstance::StopPlayer() {
#ifdef DEBUG
    char *id = "nsPluginInstance::StopPlayer";
    fprintf(stderr, "%s(%x.\n",id,this);
#endif
stopPlayer(); return NS_OK; }
NS_IMETHODIMP nsPluginInstance::RestartPlayer() {
#ifdef DEBUG
    char *id = "nsPluginInstance::RestartPlayer";
    fprintf(stderr, "%s(%x.\n",id,this);
#endif
restartPlayer(); return NS_OK; }
NS_IMETHODIMP nsPluginInstance::ResumePlayer() {
#ifdef DEBUG
    char *id = "nsPluginInstance::ResumePlayer";
    fprintf(stderr, "%s(%x.\n",id,this);
#endif
resumePlayer(); return NS_OK; }
NS_IMETHODIMP nsPluginInstance::PausePlayer() {
#ifdef DEBUG
    char *id = "nsPluginInstance::PausePlayer";
    fprintf(stderr, "%s(%x.\n",id,this);
#endif
pausePlayer(); return NS_OK; }
/* end glue */

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
#ifdef DEBUG
    char *id = "gtk_gui::gtk_gui (FAKE)";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"m_documentcontainer",m_documentcontainer);
#endif
//XXXX FIXME <EMBED src="xxx" ../> attr value is 2nd contructor arg.
    m_smilfilename = s2;
	main_loop = g_main_loop_new(NULL, FALSE);
}
gtk_gui::~gtk_gui() {
    g_object_unref (G_OBJECT (main_loop));
}

const char* ambulant::get_version() { return "0";}

#endif // AMBULANT_FIREFOX_PLUGIN
