// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef	XP_WIN32
#include <cstddef>	 // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,

#include <windows.h>
#include <windowsx.h>
#include "ambulant\lib\textptr.h"
#endif//XP_WIN32
#include "ScriptablePluginObject.h"
#include "npambulant.h"

//#ifdef	WITH_HTML_WIDGET
#include "ambulant/gui/d2/html_bridge.h"
//#endif
/* ambulant player includes */

#ifdef	XP_WIN32
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
#else//!XP_WIN32: Linux, Mac
#include <dlfcn.h>	// for dladdr()
#include <libgen.h> // for dirname()
#endif//XP_WIN32: Linux, Mac

#ifdef WITH_GTK
#include <gtk/gtk.h>
#if GTK_MAJOR_VERSION >= 3
#error "npambulant cannot yet use gtk+-3.0"
#include <gtk/gtkx.h>
#endif // GTK_MAJOR_VERSION
#include "gtk_mainloop.h"
#endif
#ifdef WITH_CG
#include "cg_mainloop.h"
#endif

#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/preferences.h"
// #define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

NPIdentifier sStartPlayer_id;
NPIdentifier sStopPlayer_id;
NPIdentifier sPausePlayer_id;
NPIdentifier sResumePlayer_id;
NPIdentifier sRestartPlayer_id;
NPIdentifier sIsDone_id;

NPIdentifier sDocument_id;
NPIdentifier sBody_id;
NPIdentifier sCreateElement_id;
NPIdentifier sCreateTextNode_id;
NPIdentifier sAppendChild_id;
NPIdentifier sPluginType_id;

const char* mimetypes =
"application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;\
application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;\
application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";

#ifndef NDEBUG
class stderr_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) { return 0;}
	int write(const char *cstr);
	void flush() {}
};

int stderr_ostream::write(const char *cstr)
{
#ifdef XP_WIN32
	OutputDebugStringA(cstr);
#endif
	fprintf(stderr, "%s", cstr);
	return strlen(cstr);
}
#endif//!NDEBUG

npambulant::npambulant(
	NPMIMEType mimetype,
	NPP pNPInstance,
	PRUint16 mode,
	int argc,
	char* argn[],
	char* argv[],
	NPSavedData* data)
:
	m_mimetype(mimetype),
	m_pNPInstance(pNPInstance ),
	m_mode(mode),
	m_argc(argc),
	m_argn(argn),
	m_argv(argv),
	m_data(data),
	m_pNPStream(NULL),
	m_bInitialized(FALSE),
	m_pScriptableObject(NULL),
	m_autostart(true),
	m_ambulant_player(NULL),
	m_Window(NULL)
{
m_url = net::url();
#ifdef XP_WIN
	m_hWnd = NULL;
	m_lpOldProc = NULL;
	m_OldWindow = NULL;
#elif WITH_CG
	m_size = CGSizeMake(0,0);
	m_doc_size = CGSizeMake(0,0);
	m_zoom = 1.0;
	m_view = NULL;
	m_cgcontext = NULL;
	m_ctm = CGAffineTransformIdentity;
	NPRect r = {0,0,0,0};
	m_nprect = r;
	m_nprect_window = r;
	m_mainloop = NULL;
#endif
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &m_window_obj);
	
	sStartPlayer_id = NPN_GetStringIdentifier("startPlayer");
	sStopPlayer_id = NPN_GetStringIdentifier("stopPlayer");
	sRestartPlayer_id = NPN_GetStringIdentifier("restartPlayer");
	sPausePlayer_id = NPN_GetStringIdentifier("pausePlayer");
	sResumePlayer_id = NPN_GetStringIdentifier("resumePlayer");
	sIsDone_id = NPN_GetStringIdentifier("isDone");

	sDocument_id = NPN_GetStringIdentifier("document");
	sBody_id = NPN_GetStringIdentifier("body");
	sCreateElement_id = NPN_GetStringIdentifier("createElement");
	sCreateTextNode_id = NPN_GetStringIdentifier("createTextNode");
	sAppendChild_id = NPN_GetStringIdentifier("appendChild");
	sPluginType_id = NPN_GetStringIdentifier("PluginType");

	DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject, AllocateScriptablePluginObject);//KB

	const char *ua = NPN_UserAgent(m_pNPInstance);
	strncpy(m_String, ua, sizeof m_String);
	NPP s_npambulant_last_instance;
	/* copy argument names and values, as some browers (Chrome) re-use their space */
	m_argn = (char**) malloc (sizeof(char*)*argc);
	m_argv = (char**) malloc (sizeof(char*)*argc);
	for (int i=0; i<m_argc; i++) {
		LOG("argn[%i]=%s argv[%i]=%s",i,argn[i],i,argv[i]);
		m_argn[i] = argn[i]?strdup(argn[i]):NULL;
		m_argv[i] = argv[i]?strdup(argv[i]):NULL;
	}
	s_npambulant_last_instance = pNPInstance;
}

npambulant::~npambulant()
{
#ifndef NDEBUG
	ambulant::lib::logger::get_logger()->set_ostream(NULL);
#endif
	if (m_window_obj)
		NPN_ReleaseObject(m_window_obj);
	if (m_pScriptableObject)
		NPN_ReleaseObject(m_pScriptableObject);
	m_window_obj = 0;
	for (int i=0; i<m_argc; i++) {
		free(m_argn[i]);
		free(m_argv[i]);
	}
	free(m_argn);
	free(m_argv);
}

bool
npambulant::init_ambulant(NPP npp)
{
	// Check if browser already called NPP_SetWindow() to inform us where to draw
	if (m_Window == NULL)
		return TRUE;
	//
	// Step 1 - Initialize the logger and such
	//
	const char* version = ambulant::get_version();
	LOG("m_window=%p ambulant version %s", m_Window, version);
#ifndef NDEBUG
	if (getenv("AMBULANT_DEBUG") != 0) {
		ambulant::lib::logger::get_logger()->set_ostream(new stderr_ostream);
		ambulant::lib::logger::get_logger()->set_level(ambulant::lib::logger::LEVEL_DEBUG);
		ambulant::lib::logger::get_logger()->debug("npambulant: DEBUG enabled. Ambulant version: %s\n", version);
	}
#else //NDEBUG
	ambulant::lib::logger::get_logger()->set_level(ambulant::lib::logger::LEVEL_SHOW);
#endif // NDEBUG
	//
	// Step 2 - Initialize preferences, including setting up for loading
	// Ambulant plugins (which are needed for SMIL State and (on Windows) ffmpeg).
	//
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = true;
	prefs->m_use_plugins = true;
	prefs->m_log_level = ambulant::lib::logger::LEVEL_SHOW;
	prefs->m_parser_id = "expat";

#ifdef XP_WIN32
	// for Windows, ffmpeg is only available as plugin
	prefs->m_plugin_path = lib::win32::get_module_dir()+"\\plugins\\";
	ambulant::lib::textptr pn_conv(prefs->m_plugin_path.c_str());
	SetDllDirectory (pn_conv);
#else //!XP_WIN3: Linux, Mac
	Dl_info p;
	if (dladdr("main", &p) < 0) {
		fprintf(stderr, "npambulant::init_ambulant:	 dladdr(\"main\") failed, cannot use ambulant plugins\n");
		prefs->m_use_plugins = false;
	} else {
#ifdef WITH_LTDL_PLUGINS
		char* path = strdup(p.dli_fname); // full path of this npapi plugin
		char* ffplugindir = dirname(path);
#ifdef MOZ_X11
		const char* npambulant_plugins = "/npambulant/plugins";
#elif WITH_CG
		const char* npambulant_plugins = "/../PlugIns"; // expected in app. bundle
#endif
		char* amplugin_path = (char*) malloc(strlen(ffplugindir)+strlen(npambulant_plugins)+1);
		sprintf(amplugin_path, "%s%s", ffplugindir, npambulant_plugins);
		prefs->m_plugin_path = amplugin_path;
		free (path);
#else // WITH_LTDL_PLUGINS
		prefs->m_use_plugins = false;
#endif // WITH_LTDL_PLUGINS
	}

#endif // !XP_WIN3: Linux, Mac
	//
	// Step 3 - save the NPWindow for any Ambulant plugins (such as SMIL State)
	//
	ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
	void *edptr = pe->get_extra_data("npapi_extra_data");
	if (edptr) {
		*(NPWindow**)edptr = m_Window;
		LOG("npambulant::init_ambulant: setting npapi_extra_data(%p) to NPWindow %p", edptr, m_Window);
	} else {
		LOG("npambulant::init_ambulant: Cannot find npapi_extra_data, cannot communicate NPWindow");
	}
	
	//
	// Step 4 - Platform-specific window mumbo-jumbo
	//
#ifdef WITH_GTK
	long long ll_winid = reinterpret_cast<long long>(m_Window->window);
#if GTK_MAJOR_VERSION >= 3
	int i_winid = static_cast<int>(ll_winid);
	GtkWidget* gtkwidget = gtk_plug_new((Window) i_winid);
#else
	int i_winid = static_cast<int>(ll_winid);
	GtkWidget* gtkwidget = GTK_WIDGET(gtk_plug_new((GdkNativeWindow) i_winid));
	if (gtkwidget->window == NULL) {
		// a.o. google-chrome
		ambulant::lib::logger::get_logger()->error( "ambulant not yet supported as plugin for this browser");
		return TRUE;
	}
#endif // GTK_MAJOR_VERSION
#endif // WITH_GTK
#ifdef	XP_WIN32
	m_hwnd = (HWND)m_Window->window;
	if(m_hwnd == NULL)
		return FALSE;
#endif//XP_WIN32
//	assert ( ! m_ambulant_player);
//	ambulant::lib::logger::get_logger()->set_show_message(npambulant_display_message);
	ambulant::lib::logger::get_logger()->show(gettext("Ambulant plugin loaded"));
	//
	// Step 5 - Argument processing, and obtaining the document URL.
	//
	const char* arg_str = NULL;
    int width = 0, height = 0;
	if (m_argc > 1) {
		for (int i =0; i < m_argc; i++) {
			// Uncomment next line to see the <EMBED/> attr values
			// fprintf(stderr, "arg[%i]:%s=%s\n",i,m_argn[i],m_argv[i]);
			const char* name = m_argn[i];
			const char* value = m_argv[i];
			if (strcasecmp(name, "data") == 0 && arg_str == NULL)
				arg_str = value;
            if (strcasecmp(name,"src") == 0 && arg_str == NULL)
                arg_str = value;
            if (strcasecmp(name,"width") == 0)
                width = atoi(value);
            if (strcasecmp(name,"height") == 0)
                height = atoi(value);
			if (strcasecmp(name,"autostart") == 0 && strcasecmp(value , "false") == 0)
				m_autostart = false;
		}
	}
	if (arg_str == NULL)
		return false;
	net::url file_url;
	net::url arg_url = net::url::from_url(arg_str);
	char* url_str = NULL;
	LOG("arg_url=%s", repr(arg_url).c_str());
	if (arg_url.is_absolute()) {
		LOG("absolute");
		url_str = strdup(arg_url.get_url().c_str());
		file_url = arg_url;
	} else {
		char* loc_str = get_document_location();
		if (loc_str != NULL) {
			net::url loc_url = net::url::from_url(loc_str);
			file_url = arg_url.join_to_base(loc_url);
			free((void*)loc_str);
		} else {
			file_url = arg_url;
		}
		url_str = strdup(file_url.get_url().c_str());
	}
	LOG("file_url=%s", repr(file_url).c_str());
	m_url = file_url;
	LOG("m_url=%s m_Window->window=%p", repr(m_url).c_str(), m_Window->window);
	//
	// Step 6 - Initialize the platform-specific widget infrastructure, create
	// the ambulant player and optionally start it
	//
#ifdef WITH_GTK
	gtk_gui* m_gui = new gtk_gui((char*) gtkwidget, url_str);
	m_mainloop = new gtk_mainloop(m_gui);
	if (url_str)
		free((void*)url_str);
	m_logger = lib::logger::get_logger();
	m_ambulant_player = m_mainloop->get_player();
	if (m_ambulant_player == NULL) return false;
	if (m_autostart) m_ambulant_player->start();
	gtk_widget_show_all(gtkwidget);
	gtk_widget_realize(gtkwidget);
#endif // WITH_GTK
#ifdef WITH_CG
	if (url_str != NULL)
 		free(url_str);
    m_nprect.right = width;
    m_nprect.bottom = height;
	if (m_view == NULL) {
		m_nprect_window = m_nprect; // Save the window rectangle for NPN_InvalidateRect 
		NPN_InvalidateRect (npp, &m_nprect);	// Ask for draw event 
		LOG("NPN_InvalidateRect(%p,{l=%d,t=%d,b=%d,r=%d}",npp,m_nprect.top,m_nprect.left,m_nprect.bottom,m_nprect.right);
    }
#endif // WITH_CG

#ifdef	XP_WIN32
	m_player_callbacks.set_os_window(m_hwnd);
	m_ambulant_player = new ambulant_gui_player(m_player_callbacks, NULL, m_url);
	if (m_ambulant_player) {
		if ( ! get_player()) {
			delete m_ambulant_player;
			m_ambulant_player = NULL;
		} else if (m_autostart)
			m_ambulant_player->play();
	}
	m_bInitialized = TRUE;
	return TRUE;
#endif // ! XP_WIN32
	m_bInitialized = true;
	return true;
}


/// Get the location of the html document.
/// In javascript this is simply document.location.href. In C it's the
/// same, but slightly more convoluted:-)
char* npambulant::get_document_location() {
	LOG("npambulant::get_document_location()");
	char *rv = NULL;

	// Get document
	NPVariant npvDocument;
	bool ok = NPN_GetProperty( m_pNPInstance, m_window_obj, NPN_GetStringIdentifier("document"), &npvDocument);
	LOG("NPN_GetProperty(..., document, ...) -> %d, %p",  ok, npvDocument.value.objectValue);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvDocument));
	NPObject *document = NPVARIANT_TO_OBJECT(npvDocument);
	assert(document);

	// Get document.location
	NPVariant npvLocation;
	ok = NPN_GetProperty(m_pNPInstance, document, NPN_GetStringIdentifier("location"), &npvLocation);
	LOG("NPN_GetProperty(..., location, ...) -> %d, %p", ok, npvLocation.value.objectValue);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvLocation));
	NPObject *location = NPVARIANT_TO_OBJECT(npvLocation);
	assert(location);

	// Get document.location.href
	NPVariant npvHref;
	ok = NPN_GetProperty(m_pNPInstance, location, NPN_GetStringIdentifier("href"), &npvHref);
	LOG("NPN_GetProperty(..., href, ...) -> %d, %p", ok, npvHref.value.objectValue);
	if (!ok) return NULL;
	if (!NPVARIANT_IS_STRING(npvHref)) {
		LOG("get_document_location: document.location.href is not a string");
		return NULL;
	}

	// Turn it into a C string.
	// XXXJACK: the memory for the string isn't released...
	NPString href = NPVARIANT_TO_STRING(npvHref);
	rv = (char*) malloc(href.UTF8Length+1);
	strncpy(rv, href.UTF8Characters, href.UTF8Length);
	rv[href.UTF8Length] = '\0';
	LOG("get_document_location: returning \"%s\"", rv);

	NPN_ReleaseVariantValue(&npvLocation);
	NPN_ReleaseVariantValue(&npvDocument);
	NPN_ReleaseVariantValue(&npvHref);
	return rv;
}

double
recompute_zoom (double old_width, double old_height, double new_width, double new_height) {
	double rv = 1.0;
	if (old_width > 0.0 && new_width > 0.0) {
		double scale_x = new_width / old_width;
		double scale_y = new_height / old_height;
		rv = scale_x > 1.0 && scale_x < scale_y ? scale_x : scale_y;
		AM_DBG fprintf(stderr, "recompute_zoom:scale_x=%f, scale_y=%f, scale=%f\n", scale_x, scale_y, rv);
	}
	return rv;
}

NPBool
npambulant::setWindow(NPWindow* pNPWindow) {
	if(pNPWindow == NULL)
		return FALSE;
	bool first_call = m_Window == NULL;
	if (m_Window && m_Window != pNPWindow)
		ambulant::lib::logger::get_logger()->trace("npambulant: NPWindow changed from %p to %p", m_Window, pNPWindow);
#ifdef XP_WIN
	if (m_hWnd && m_hwnd != (HWND)pNPWindow->window)
		ambulant::lib::logger::get_logger()->trace("npambulant: HWND changed from %p to %p", m_hWnd, (HWND)pNPWindow->window);
	m_hWnd = (HWND)pNPWindow->window;
	if(m_hWnd == NULL)
		return FALSE;
	// subclass window so we can intercept window messages and
	// do our drawing to it
	m_lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);

	// associate window with our npambulant object so we can access
	// it in the window procedure
	m_OldWindow = SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
#endif

	m_Window = pNPWindow;
	LOG("m_Window=%p .windox=%p .x=%d .y=%d .width=%d .height=%d .clipRect=(t=%d,l=%d,b=%d,r=%d) .type=%d", m_Window, m_Window->window, m_Window->x, m_Window->y, m_Window->width, m_Window->height, m_Window->clipRect.top, m_Window->clipRect.left, m_Window->clipRect.bottom, m_Window->clipRect.right, m_Window->type);
#ifdef WITH_CG
	if (m_size.width == 0.0 && m_size.height == 0.0) {
		// initialize 
		m_size.width = m_Window->width;
		m_size.height = m_Window->height;
		m_ctm = CGAffineTransformIdentity;
	} else if (m_size.width != m_Window->width || m_size.height != m_Window->height) {
		m_zoom = recompute_zoom(m_doc_size.width, m_doc_size.height, m_Window->width, m_Window->height);
		m_size.width = m_Window->width;
		m_size.height = m_Window->height;
		LOG("m_doc_size=(%f,%f) m_Window->width=%d, m_Window->height=%d m_zoom=%f)",m_doc_size.width, m_doc_size.height, m_Window->width, m_Window->height, m_zoom);
		m_ctm = CGAffineTransformMake(m_zoom,0.0,0.0,m_zoom, 0.0,0.0);
		if (m_cgcontext != NULL) {
//			CGContextScaleCTM(m_cgcontext, m_zoom, m_zoom);
		}
	}				  
#endif//WITH_CG
	if (first_call) {
		init_ambulant(get_NPP());
	}
	return TRUE;
}

NPBool
npambulant::init() {
	if (!m_pNPInstance) {
		ambulant::lib::logger::get_logger()->trace("npambulant: init called without NPInstance");
		return FALSE;
	}
	if (!m_Window) {
		ambulant::lib::logger::get_logger()->trace("npambulant: init called without NPWindow");
		return FALSE;
	}
	if (m_bInitialized) {
		ambulant::lib::logger::get_logger()->trace("npambulant: init called twice");
	}
	m_bInitialized = TRUE;
	return init_ambulant(m_pNPInstance);
}

void
npambulant::shut() {
#ifdef XP_WIN
	if (m_hWnd) {
		// reset the userdata association
		if (m_OldWindow)
			SetWindowLong(m_hWnd, GWL_USERDATA, m_OldWindow);
		// subclass it back
		if (m_lpOldProc)
			SubclassWindow(m_hWnd, m_lpOldProc);
	}
	m_OldWindow = NULL;
	m_hWnd = NULL;
	m_lpOldProc = NULL;
#endif//XP_WIN

	if (m_ambulant_player) {
#ifdef XP_WIN
		m_ambulant_player->get_player()->stop();
		delete m_ambulant_player;
	}
#else
	if (m_ambulant_player->is_playing()
		|| m_ambulant_player->is_pausing() )
	{
		m_ambulant_player->stop();
		while ( ! m_ambulant_player->is_done())
			sleep(3);
	}
#ifdef WITH_CG
	if (m_view != NULL) {
 //X mainloop takes care //delete_AmbulantView(m_view);
		m_view = NULL;
	}
#endif//WITH_CG
	delete m_mainloop;
	}
#endif
	m_ambulant_player = NULL; // deleted by mainloop
	m_bInitialized = FALSE;
	//XXXX SDL_Quit() forgets to do clear the environment variable ESD_NO_SPAWN
	// the variable was included in the environment by calling	putenv(char* string),
	// using the data section from the plugin as storage for 'string'.man putenv says:
	// "The string pointed to by 'string' becomes part of the environment"
	// this caused firefox to crash after npambulant was shut and removed, because
	// when using getenv() it hit a pointer to a string that was no longer there.
	// unsetenv() is not available on Windows.
#ifndef XP_WIN
	unsetenv("ESD_NO_SPAWN");
#endif//XP_WIN
}

NPBool
npambulant::isInitialized() {
	return m_bInitialized;
}

NPP
npambulant::getNPP() {
	return m_pNPInstance;
}

const char*
npambulant::getValue(const char *name) {
	return NULL; //TBD
}

int16
npambulant::handleEvent(void* event) {
#if GTK_MAJOR_VERSION >= 3
	LOG("handleEvent: type = %d", ((XEvent*) event)->type);
#endif // GTK_MAJOR_VERSION
#ifdef WITH_CG
	NPCocoaEvent cocoaEvent = *(NPCocoaEvent*)event;
	LOG("event=%p, type=%d", event, cocoaEvent.type);
	AM_DBG {
		// print everything in the event data
		// From: https://wiki.mozilla.org/NPAPI:CocoaEventModel
		switch (cocoaEvent.type) {
			case NPCocoaEventDrawRect:			//1
				LOG("cocoaEvent.data.draw.context=%p .x=%lf .y=%lf .width=%lf .height=%lf", cocoaEvent.data.draw.context, cocoaEvent.data.draw.x, cocoaEvent.data.draw.y, cocoaEvent.data.draw.width, cocoaEvent.data.draw.height);
				break;
			case NPCocoaEventMouseDown: 		//2
			case NPCocoaEventMouseUp:			//3
			case NPCocoaEventMouseMoved: 		//4
			case NPCocoaEventMouseEntered: 		//5
			case NPCocoaEventMouseExited: 		//6
			case NPCocoaEventMouseDragged: 		//7
				LOG("cocoaEvent.data.mouse.modifierFlags=%x .pluginX=%lf .pluginY=%lf .buttonNumber=%d .clickCount=%d .deltaX=%lf .deltaY=%lf .deltaZ=%lf", cocoaEvent.data.mouse.modifierFlags, cocoaEvent.data.mouse.pluginX, cocoaEvent.data.mouse.pluginY, cocoaEvent.data.mouse.buttonNumber, cocoaEvent.data.mouse.clickCount, cocoaEvent.data.mouse.deltaX, cocoaEvent.data.mouse.deltaY, cocoaEvent.data.mouse.deltaZ);
				break;
			case NPCocoaEventKeyDown:			//8
			case NPCocoaEventKeyUp:				//9
				LOG("cocoaEvent.data.key.modifierFlags=%x .characters=%p .charactersIgnoringModifiers=%p .isARepeat=%d keyCode=%d", cocoaEvent.data.key.modifierFlags, cocoaEvent.data.key.characters, cocoaEvent.data.key.charactersIgnoringModifiers, cocoaEvent.data.key.isARepeat, cocoaEvent.data.key.keyCode);
				break;
			case NPCocoaEventFlagsChanged: 		//10
			case NPCocoaEventFocusChanged: 		//11
			case NPCocoaEventWindowFocusChanged://12
				LOG("cocoaEvent.data.focus.hasFocus=%d",cocoaEvent.data.focus.hasFocus); 
				break;
			case NPCocoaEventScrollWheel:		//13
			case NPCocoaEventTextInput:			//14
				LOG("cocoaEvent.data.text=%p", cocoaEvent.data.text.text); // NPNString* is a NSString*
				break;
			default:
				LOG("unknown cocoaEvent");
				break;
		};
	}
	if (cocoaEvent.type == NPCocoaEventDrawRect) {
		CGRect cgrect = CGRectMake(cocoaEvent.data.draw.x, cocoaEvent.data.draw.y, cocoaEvent.data.draw.width, cocoaEvent.data.draw.height);
		NPRect nprect = {cocoaEvent.data.draw.y, cocoaEvent.data.draw.x, cocoaEvent.data.draw.y+ cocoaEvent.data.draw.height,cocoaEvent.data.draw.x+cocoaEvent.data.draw.width};
		m_nprect = nprect;
		LOG("New m_nprect=(tlbr)(%d,%d,%d,%d)",m_nprect.top,m_nprect.left,m_nprect.bottom,m_nprect.right);
		CGContextRef ctx =  ((NPCocoaEvent*) event)->data.draw.context;
		if (ctx == NULL) {
			return 1;
		}
		CGAffineTransform ctm = CGContextGetCTM(ctx);
		LOG("CGContext=%p CTM(a=%f,b=%f,c=%f,d=%f,tr=%f,ty=%f)",ctx,ctm.a,ctm.b,ctm.c,ctm.d,ctm.tx,ctm.ty);
		m_cgcliprect = cgrect;
		if (m_cgcontext != ctx) {
			m_cgcontext = ctx;
			CGAffineTransform t = CGContextGetCTM(ctx);
			LOG("New m_cgcontext: ctx=%p (t=abcdxy)(%f,%f,%f,%f,%f,%f)", ctx,t.a,t.b,t.c,t.d,t.tx,t.ty);
		}
		if (m_view == NULL && repr(m_url) != "") {
			init_cg_view(m_cgcontext);
			return 1;
		}
		if (m_view != NULL && m_mainloop != NULL) {
			LOG("m_view=%p m_mainloop=%p m_cgcliprect=(ltwh)(%f,%f,%f,%f)",m_view, m_mainloop,m_cgcliprect.origin.x,m_cgcliprect.origin.y,m_cgcliprect.size.width,m_cgcliprect.size.height);
			CGContextScaleCTM(m_cgcontext, m_zoom, m_zoom);
			// remember last CTM used fro drawing for mouse location and NPInvalidateRect
			m_ctm = CGContextGetCTM(ctx);
			draw_rect_AmbulantView(m_view, m_cgcontext, &m_cgcliprect); // do redraw
		}
 	} else  if (cocoaEvent.type == NPCocoaEventMouseMoved || cocoaEvent.type == NPCocoaEventMouseDown || cocoaEvent.type == NPCocoaEventMouseEntered || cocoaEvent.type == NPCocoaEventMouseExited) {
 		if (m_view != NULL && m_mainloop != NULL) {
		    event_data e_data;
			unsigned long int NSLeftMouseDown = 1, NSMouseMoved = 5, NSMouseEntered = 8, NSMouseExited = 9; //XXX needs #include <NSEvent.h >
			CGPoint p = CGPointMake(cocoaEvent.data.mouse.pluginX, cocoaEvent.data.mouse.pluginY);
			p = CGPointApplyAffineTransform(p, CGAffineTransformInvert(CGAffineTransformScale(CGAffineTransformIdentity, m_zoom, m_zoom)));
 			e_data.x = p.x; //convert_x;
			e_data.y = p.y; //convert_y;
			unsigned long int e_type
			  = cocoaEvent.type == NPCocoaEventMouseMoved ? NSMouseMoved
			  : cocoaEvent.type == NPCocoaEventMouseEntered ? NSMouseEntered
			  : cocoaEvent.type == NPCocoaEventMouseExited ? NSMouseExited
			  : NSLeftMouseDown;
			handle_event_AmbulantView((void*) m_view,  m_cgcontext, &e_type, (void*) &e_data, m_mainloop);
		}
	} else if (cocoaEvent.type == NPCocoaEventKeyDown) {
 		if (m_view != NULL && m_mainloop != NULL) {
			const char* s = to_char_AmbulantView((void*) m_view, cocoaEvent.data.key.characters);
			if (s != NULL) {
				LOG("key.characters=%s",s);
				m_mainloop->on_char((int) *s);
			}
		}
	} else if (m_nprect.top < m_nprect.bottom && m_nprect.left < m_nprect.right) {
		NPRect npr = m_nprect_window;
		NPN_InvalidateRect (m_pNPInstance, &npr);	// Ask for draw event
		LOG("NPN_InvalidateRect(%p,{l=%d,t=%d,b=%d,r=%d}",m_pNPInstance,npr.top,npr.left,npr.bottom,npr.right);
	}
#endif//WITH_CG
	return 0;
}

// this will start AmbulantPlayer
void
npambulant::startPlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::startPlayer()\n");
	if (m_ambulant_player != NULL) {
		get_player()->start();
	}
}

// this will stop AmbulantPlayer
void
npambulant::stopPlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::stopPlayer()\n");
	if (m_ambulant_player != NULL) {
		get_player()->stop();
	}
}

// this will restart AmbulantPlayer
void
npambulant::restartPlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::restartPlayer()\n");
	if (m_ambulant_player != NULL) {
		get_player()->stop();
		get_player()->start();
	}
}

// this will pause AmbulantPlayer
void
npambulant::pausePlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::pausePlayer()\n");
	if (m_ambulant_player != NULL) {
		get_player()->pause();
	}
}

// this will resume AmbulantPlayer
void
npambulant::resumePlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::resumePlayer()\n");
	if (m_ambulant_player != NULL) {
		get_player()->resume();
	}
}

// this will restart AmbulantPlayer
bool
npambulant::isDone() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::isDone()\n");
	if (m_ambulant_player != NULL) {
		return get_player()->is_done();
	}
	return false;
}

// this will force to draw a version string in the plugin window
void
npambulant::showVersion() {
	const char *ua = NPN_UserAgent(m_pNPInstance);
	strcpy(m_String, ua);

#ifdef XP_WIN
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);
#endif

	if (m_Window) {
		NPRect r =
#ifdef JNK
			{
				(uint16)m_Window->y, (uint16)m_Window->x,
				(uint16)(m_Window->y + m_Window->height),
				(uint16)(m_Window->x + m_Window->width)
			};
#else //JNK
			{ 0,0,200,200 };
#endif//JNK
			NPN_InvalidateRect(m_pNPInstance, &r);
	}
}

// this will clean the plugin window
void
npambulant::clear() {
	strcpy(m_String, "");

#ifdef XP_WIN
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);
#endif
}

void
npambulant::getVersion(char* *aVersion) {
	const char *ua = NPN_UserAgent(m_pNPInstance);
	char*& version = *aVersion;
	version = (char*)NPN_MemAlloc(1 + strlen(ua));
	if (version) {
		strcpy(version, ua);
	}
}

NPObject *
npambulant::GetScriptableObject() {
	DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject, AllocateScriptablePluginObject);//KB
	if ( ! m_pScriptableObject) {
		m_pScriptableObject = NPN_CreateObject(m_pNPInstance, GET_NPOBJECT_CLASS(ScriptablePluginObject));
	}
	if (m_pScriptableObject) {
		NPN_RetainObject(m_pScriptableObject);
	}
	return m_pScriptableObject;
}

/* status line */
#ifndef WIN32
extern "C" {
#endif//WIN32
NPP s_npambulant_last_instance = NULL;

void
npambulant_display_message(int level, const char *message) {
	if (s_npambulant_last_instance) {
		NPN_Status(s_npambulant_last_instance, message);
	}
}
#ifndef WIN32
} // extern "C"
#endif//WIN32

// some platform/toolkit specific hacks and functions

#ifdef XP_WIN32

static ambulant_player_callbacks s_ambulant_player_callbacks;

static LRESULT CALLBACK
PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	npambulant *plugin = (npambulant *)GetWindowLong(hWnd, GWL_USERDATA);
	if (plugin) {
		switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rc;
				GetClientRect(hWnd, &rc);
#ifndef	NDEBUG
				// Draw rectangle around drawing area
				FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
				EndPaint(hWnd, &ps);
#endif//NDEBUG
				if (plugin->m_ambulant_player)
					plugin->m_ambulant_player->redraw(hWnd, hdc, NULL); // XXX Should pass dirty rect
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
//XX						if (new_cursor_id>0) EnableToolTips(TRUE);
//XX						else CancelToolTips();
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
			}
		default:
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
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

#ifdef WITH_GTK
// some fake gtk_gui functions needed by gtk_mainloop
void gtk_gui::internal_message(int, char*) {}
GtkWidget* gtk_gui::get_document_container() { return m_documentcontainer; }
//XXXX FIXME fake gtk_gui constructor 1st arg is used as GtkWindow, 2nd arg as smilfile
gtk_gui::gtk_gui(const char* s, const char* s2) {
	memset (this, 0, sizeof(gtk_gui));
	m_toplevelcontainer = (GtkWindow*) s;
	m_documentcontainer = gtk_drawing_area_new();
//	gtk_widget_hide(m_documentcontainer);
//XXXX FIXME vbox only needed to give	m_documentcontainer a parent widget at *draw() callback time
#if GTK_MAJOR_VERSION >= 3
//	gtk_widget_set_parent (m_documentcontainer, (GtkWidget*) m_toplevelcontainer);
	gtk_container_add((GtkContainer*) m_toplevelcontainer, m_documentcontainer);
#else
	m_guicontainer = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));
	gtk_box_pack_start (GTK_BOX(m_guicontainer), m_documentcontainer, TRUE, TRUE, 0);
#endif // GTK_MAJOR_VERSION
//XXXX not used:  m_guicontainer = menubar = NULL;
//XXXX FIXME <EMBED src="xxx" ../> attr value is 2nd contructor arg.
	m_smilfilename = s2;
	main_loop = g_main_loop_new(NULL, FALSE);
}

gtk_gui::~gtk_gui() {
	g_object_unref (G_OBJECT (main_loop));
}
#endif // WITH_GTK

#ifdef  WITH_CG

void
plugin_callback(void* ptr, void* arg)
{
	npambulant* npa = (npambulant*) ptr;
	CGRect r = *(CGRect*) arg;
	// Note: NPRect is top-left-bottom-right (https://developer.mozilla.org/en/NPRect)
	// typedef struct _NPRect{ uint16 top; uint16 left; uint16 bottom; uint16 right; } NPRect;
	r = CGRectApplyAffineTransform(r, CGAffineTransformScale(CGAffineTransformIdentity, npa->m_zoom, npa->m_zoom));
	NPRect npr = {r.origin.y, r.origin.x, r.origin.y+r.size.height, r.origin.x+r.size.width};
	npr = npa->m_nprect_window; // always redraw whole window
	AM_DBG ambulant::lib::logger::get_logger()->debug("plugin_callback(%p,%p): calling NPN_InvalidateRect r=(tlbr)(%d,%d,%d,%d)\n", ptr, arg, npr.top, npr.left, npr.bottom, npr.right);
	NPN_InvalidateRect (npa->get_NPP(), &npr);
}

void
npambulant::init_cg_view(CGContextRef cg_ctx)
{
	LOG("getting a view... m_view=%p, cg_ctx=%p m_url=%s", m_view, cg_ctx, repr(m_url).c_str());
	if (cg_ctx == NULL || m_view != NULL || repr(m_url) == "")
		return;
	CGRect cgcliprect =  CGContextGetClipBoundingBox (cg_ctx);
	LOG("CGContext=%p bounding box (%f, %f, %f, %f)",cg_ctx,cgcliprect.origin.x,cgcliprect.origin.y,cgcliprect.size.width,cgcliprect.size.height);
	m_view = new_AmbulantView(cg_ctx, cgcliprect, (void*) plugin_callback, this);
	if (m_view == NULL) {
		return;
	}
	m_mainloop = new cg_mainloop(repr(m_url).c_str(), m_view, false, NULL);
	m_logger = lib::logger::get_logger();
	m_ambulant_player = m_mainloop->get_player();
	if (m_ambulant_player == NULL) {
		delete m_mainloop;
		m_mainloop = NULL;
		m_view = NULL;
		LOG("m_ambulant_player == NULL");
		return;
	}
	m_doc_size = get_bounds_AmbulantView((void*) m_view);
	LOG("m_doc_size=%f,%f",m_doc_size.width, m_doc_size.height);
	CGRect r = CGRectMake(0,0, m_doc_size.width, m_doc_size.width);
	if (m_view == NULL) {
		return;
	}
	m_size.width = 1; m_size.height = 1;
	setWindow(m_Window); // recompute_zoom
	if (m_autostart)
		m_ambulant_player->start();

}
#endif//WITH_CG
/* Attic
	GtkWidget* gtkwidget = GTK_WIDGET(gtk_plug_new((Window) i_winid));
	if (gtk_widget_get_window(gtkwidget) == NULL) {

	gtk_gui* m_gui = new gtk_gui((char*) gtkwidget, url_str);
	m_mainloop = new gtk_mainloop(m_gui);
	if (url_str)
		free((void*)url_str);
	m_logger = lib::logger::get_logger();
	m_ambulant_player = m_mainloop->get_player();
	if (m_ambulant_player == NULL) return false;
	if (m_autostart) m_ambulant_player->start();
	gtk_widget_show_all(gtkwidget);
	gtk_widget_realize(gtkwidget);
z


//	m_guicontainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
//	gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));
//	gtk_box_pack_start (GTK_BOX(m_guicontainer), m_documentcontainer, TRUE, TRUE, 0);

 */

