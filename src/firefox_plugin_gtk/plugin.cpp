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
#ifdef WITH_GTK
#include "gtk_mainloop.h"
#endif
#ifdef WITH_CG
#include "cg_mainloop.h"
#endif

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

extern "C" {

//////////////////////////////////////
//
// general identification of this plugin and what it does
//

#define PLUGIN_NAME "ambulant plugin"
#define PLUGIN_DESCRIPTION "W3C Smil 3.0 multimedia player"
char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";

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

NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
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
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
  AM_DBG fprintf(stderr, "NS_NewPluginInstance(0x%x)\n", aCreateDataStruct);
  if(!aCreateDataStruct)
    return NULL;

  nsPluginInstance * plugin = new nsPluginInstance(aCreateDataStruct->instance);
  if (plugin)
	 plugin->mCreateData = *aCreateDataStruct;
  AM_DBG fprintf(stderr, "NS_NewPluginInstance: created %s=0x%x.\n",plugin);
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
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
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
	AM_DBG fprintf(stderr, "nsPluginInstance::init(0x%x)\n", aWindow);
    mNPWindow = aWindow;
    NPError nperr = NPN_GetValue(mInstance, NPNVWindowNPObject, &mNPWindow);
#ifdef	XP_UNIX
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
#endif/*XP_UNIX*/
	mInitialized = true;
    return true;
}

void nsPluginInstance::shut()
{
	AM_DBG fprintf(stderr, "nsPluginInstance::shut()\n");
    if (m_mainloop)
        delete m_mainloop;
    m_mainloop = NULL;
    m_ambulant_player = NULL; // deleted by mainloop
}

NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}

/// Get the location of the html document.
/// If the html document contains a javascript function GetDocumentLocation(), 
/// that one is used; otherwise dynamically a script is executed to retrieve
/// the information.
/// A third method could be to use NPN_GetProperty first to retrieve the 'document'
/// property from the 'window' object. Next use NPN_GetProperty  on the 'location'
/// property from the 'document' object. Finally, the string value could then be
/// retrieved using NPN_Invoke on the 'property' object with the 'toString' function.
/// The advantage of this approach would be that no javascript code is needed.
char* nsPluginInstance::get_document_location()
{
    char *id = "ambulant::nsPluginInstance::getLocation";
	AM_DBG fprintf(stderr, "nsPluginInstance::get_document_location)\n");
    char *rv = NULL;
    NPVariant npvarResult;
    NPIdentifier npidJSfun = NPN_GetStringIdentifier("GetDocumentLocation");
    bool ok = NPN_HasMethod(mInstance, (NPObject*) mNPWindow, npidJSfun);
    AM_DBG fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"ok",ok);
    if ( ! ok) {
        // dynamically evaluate javascript code to get the desired information.
        // by returning it first in a function, it is also returned by NPN_Evaluate.
        // prependeding the empty string forces return type NPVariantType_String.
        static const char js_script[] = "function GetDocumentLocation() { return ''+document.location; } GetDocumentLocation();";
        NPString npstrJSscript;
        npstrJSscript.utf8characters = js_script;
        npstrJSscript.utf8length = sizeof(js_script) - 1;
        ok = NPN_Evaluate(mInstance, (NPObject*) mNPWindow, &npstrJSscript, &npvarResult);
        if ( ! ok) {
            lib::logger::get_logger()->warn("%s: %s failed",id,"calling NPN_Invoke(\"eval\",(\"document.location\") failed.");
            return rv;
        }
    } else {
        ok = NPN_Invoke(mInstance, (NPObject*) mNPWindow, npidJSfun, NULL, 0, &npvarResult);
    }
    if (ok && NPVARIANT_IS_STRING(npvarResult)) {
        NPString nps = NPVARIANT_TO_STRING(npvarResult);
        size_t str_len = nps.utf8length;
        rv = (char*) malloc(str_len+1);
        strncpy(rv, nps.utf8characters, str_len);
        rv[str_len] = '\0';
    }
    NPN_ReleaseVariantValue(&npvarResult);
    return rv;
}

/* glue code */
NS_IMPL_ISUPPORTS1(nsPluginInstance, nsIAmbulantPlugin)

// this will start AmbulantPLayer
NS_IMETHODIMP nsPluginInstance::StartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->start();
	return NS_OK;
}

// this will stop AmbulantPLayer
NS_IMETHODIMP nsPluginInstance::StopPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::StopPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->stop();
	return NS_OK;
}

// this will restart AmbulantPLayer
NS_IMETHODIMP nsPluginInstance::RestartPlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::RestartPlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->stop();
	m_ambulant_player->start();
	return NS_OK;
}

// this will resume AmbulantPLayer
NS_IMETHODIMP nsPluginInstance::ResumePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::ResumePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->resume();
	return NS_OK;
}

// this will pause AmbulantPLayer
NS_IMETHODIMP nsPluginInstance::PausePlayer()
{
	AM_DBG lib::logger::get_logger()->debug("nsPluginInstance::PausePlayer()\n");
	if (m_ambulant_player == NULL) return NS_ERROR_NOT_AVAILABLE;
	m_ambulant_player->pause();
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


  if (aVariable == NPPVpluginScriptableInstance
#if 0
		// Jack added this one: it's what Safari seems to use. No idea
		// whether that's really correct, though...
		// ... And it seems it isn't: Safari wants another type of object.
		|| aVariable == NPPVpluginScriptableNPObject
#endif
		) {
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
  } else  {
	*(void**) aValue = NULL;
	rv = NPERR_INVALID_PARAM;
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
  AM_DBG fprintf(stderr, "nsPluginInstance::getScriptablePeer()\n");
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

#if 0
// XXXJACK: I think (but am not sure) this is cruft leftover from the sample
// code we started with. If things work this can be ripped out

#ifndef XP_UNIX
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    default:
      break;
  }

  return NULL;
}
#endif // XP_UNIX

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


const char* ambulant::get_version() { return "0";}
#endif



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
