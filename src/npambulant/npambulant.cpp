#ifdef	XP_WIN32
#include <cstddef>	 // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
#ifdef _DEBUG
#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#endif//_DEBUG

#include <windows.h>
#include <windowsx.h>
#include "ambulant\lib\textptr.h"
#endif//XP_WIN32
#include "ScriptablePluginObject.h"
#include "npambulant.h"

//#ifdef	WITH_HTML_WIDGET
#include "ambulant/gui/dx/html_bridge.h"
//#endif
/* ambulant player includes */

#ifdef	XP_WIN32
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
#else//!XP_WIN32: Linux, Mac
#include <dlfcn.h>  // for dladdr()
#include <libgen.h> // for dirname()
#endif//XP_WIN32: Linux, Mac

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
#ifdef XP_WIN
	m_hWnd = NULL;
	m_lpOldProc = NULL;
	m_OldWindow = NULL;
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
	strcpy(m_String, ua);
	extern NPP s_npambulant_last_instance;
	/* copy argument names and values, as some browers (Chrome) re-use their space */
	m_argn = (char**) malloc (sizeof(char*)*argc);
	m_argv = (char**) malloc (sizeof(char*)*argc);
	for (int i=0; i<m_argc; i++) {
		m_argn[i] = strdup(argn[i]);
		m_argv[i] = strdup(argv[i]);
	}
	s_npambulant_last_instance = pNPInstance;
}

npambulant::~npambulant()
{
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
npambulant::init_ambulant(NPP npp, NPWindow* aWindow)
{
	//
	// Step 1 - Initialize the logger and such
	//
	const char* version = ambulant::get_version();
	AM_DBG fprintf(stderr, "npambulant::init(0x%x) ambulant version\n", aWindow, version);
#ifndef NDEBUG
	if (getenv("AMBULANT_DEBUG") != 0) {
		ambulant::lib::logger::get_logger()->set_ostream(new stderr_ostream);
		ambulant::lib::logger::get_logger()->set_level(ambulant::lib::logger::LEVEL_DEBUG);
		ambulant::lib::logger::get_logger()->debug("npambulant: DEBUG enabled. Ambulant version: %s\n", version);
	}
#else //NDEBUG
	ambulant::lib::logger::get_logger()->set_level(ambulant::lib::logger::LEVEL_SHOW);
#endif // NDEBUG
	if(aWindow == NULL)
		return FALSE;
	
	//
	// Step 2 - Initialize preferences, including setting up for loading
	// Ambulant plugins (which are needed for SMIL State and (on Windows) ffmpeg).
	//
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = true;
	prefs->m_use_plugins = true;
	prefs->m_log_level = ambulant::lib::logger::LEVEL_SHOW;
	
#ifdef XP_WIN32
	// for Windows, ffmpeg is only available as plugin
	prefs->m_plugin_dir = lib::win32::get_module_dir()+"\\plugins\\";
	ambulant::lib::textptr pn_conv(prefs->m_plugin_dir.c_str());
	SetDllDirectory (pn_conv);
#else //!XP_WIN3: Linux, Mac
	Dl_info p;
	if (dladdr("main", &p) < 0) {
	    fprintf(stderr, "npambulant::init_ambulant:  dladdr(\"main\") failed, cannot use ambulant plugins\n");
	    prefs->m_use_plugins = false;
	} else {
#ifdef WITH_LTDL_PLUGINS
	    char* path = strdup(p.dli_fname); // full path of this firefox plugin
	    char* ffplugindir = dirname(path);
		char* npambulant_plugins = "/npambulant/plugins";
	    char* amplugin_path = (char*) malloc(strlen(ffplugindir)+strlen(npambulant_plugins)+1);
	    sprintf(amplugin_path, "%s%s", ffplugindir, npambulant_plugins);
	    prefs->m_plugin_dir = amplugin_path;
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
		*(NPWindow**)edptr = aWindow;
		AM_DBG fprintf(stderr, "npambulant::init_ambulant: setting npapi_extra_data(0x%x) to NPWindow 0x%x\n", edptr, aWindow);
	} else {
		AM_DBG fprintf(stderr, "npambulant::init_ambulant: Cannot find npapi_extra_data, cannot communicate NPWindow\n");
    }
    
    //
    // Step 4 - Platform-specific window mumbo-jumbo
    //
#ifdef WITH_GTK
	long long ll_winid = reinterpret_cast<long long>(aWindow->window);
	int i_winid = static_cast<int>(ll_winid);
	GtkWidget* gtkwidget = GTK_WIDGET(gtk_plug_new((GdkNativeWindow) i_winid));
	if (gtkwidget->window == NULL) {
		// a.o. google-chrome
		ambulant::lib::logger::get_logger()->error( "ambulant not yet supported as plugin for this browser");
		return FALSE;
	}
#endif // WITH_GTK
#ifdef	XP_WIN32
	m_hwnd = (HWND)aWindow->window;
	if(m_hwnd == NULL)
		return FALSE;
#endif//XP_WIN32
	assert ( ! m_ambulant_player);
	ambulant::lib::logger::get_logger()->set_show_message(npambulant_display_message);
	ambulant::lib::logger::get_logger()->show(gettext("Ambulant plugin loaded"));

	//
	// Step 5 - Argument processing, and obtaining the document URL.
	//
	const char* arg_str = NULL;
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
			if (strcasecmp(name,"autostart") == 0 && strcasecmp(value , "false") == 0)
				m_autostart = false;
		}
	}
	if (arg_str == NULL)
        	return false;
    net::url file_url;
	net::url arg_url = net::url::from_url(arg_str);
	char* url_str = NULL;
	if (arg_url.is_absolute()) {
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
	m_url = file_url;
	
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
	NP_CGContext *cg_context = (NP_CGContext *)aWindow->window;
	AM_DBG fprintf(stderr, "npambulant::init_ambulant: context=0x%x, window=0x%x\n", cg_context->context, cg_context->window);
	AM_DBG {
		CGRect rect = CGContextGetClipBoundingBox(cg_context->context);
		fprintf(stderr, "npambulant::init_ambulant: bounding box (%f, %f, %f, %f)\n", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
	}
	void *view = NULL; // XXXJACK
	m_mainloop = new cg_mainloop(url_str, view, false, NULL);
	m_logger = lib::logger::get_logger();
	m_ambulant_player = m_mainloop->get_player();
	if (m_ambulant_player == NULL)
		return false;
	if (m_autostart)
	  m_ambulant_player->start();
#endif // WITH_CG

#ifdef	XP_WIN32
	m_player_callbacks.set_os_window(m_hwnd);
	m_ambulant_player = new ambulant::gui::dx::dx_player(m_player_callbacks, NULL, m_url);
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
	AM_DBG fprintf(stderr, "npambulant::get_document_location()\n");
	char *rv = NULL;

	// Get document
	NPVariant npvDocument;
	bool ok = NPN_GetProperty( m_pNPInstance, m_window_obj, NPN_GetStringIdentifier("document"), &npvDocument);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., document, ...) -> %d, 0x%d\n", ok, npvDocument);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvDocument));
	NPObject *document = NPVARIANT_TO_OBJECT(npvDocument);
	assert(document);

	// Get document.location
	NPVariant npvLocation;
	ok = NPN_GetProperty(m_pNPInstance, document, NPN_GetStringIdentifier("location"), &npvLocation);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., location, ...) -> %d, 0x%d\n", ok, npvLocation);
	if (!ok) return NULL;
	assert(NPVARIANT_IS_OBJECT(npvLocation));
	NPObject *location = NPVARIANT_TO_OBJECT(npvLocation);
	assert(location);

	// Get document.location.href
	NPVariant npvHref;
	ok = NPN_GetProperty(m_pNPInstance, location, NPN_GetStringIdentifier("href"), &npvHref);
	AM_DBG fprintf(stderr, "NPN_GetProperty(..., href, ...) -> %d, 0x%d\n", ok, npvHref);
	if (!ok) return NULL;
	if (!NPVARIANT_IS_STRING(npvHref)) {
		AM_DBG fprintf(stderr, "get_document_location: document.location.href is not a string\n");
		return NULL;
	}

	// Turn it into a C string.
	// XXXJACK: the memory for the string isn't released...
	NPString href = NPVARIANT_TO_STRING(npvHref);
	rv = (char*) malloc(href.UTF8Length+1);
	strncpy(rv, href.UTF8Characters, href.UTF8Length);
	rv[href.UTF8Length] = '\0';
	AM_DBG fprintf(stderr, "get_document_location: returning \"%s\"\n", rv);

	NPN_ReleaseVariantValue(&npvLocation);
	NPN_ReleaseVariantValue(&npvDocument);
	NPN_ReleaseVariantValue(&npvHref);
	return rv;
}

NPBool
npambulant::setWindow(NPWindow* pNPWindow) {
	if(pNPWindow == NULL)
		return FALSE;
	if (m_Window && m_Window != pNPWindow)
		ambulant::lib::logger::get_logger()->trace("npambulant: NPWindow changed from 0x%x to 0x%x", m_Window, pNPWindow);
#ifdef XP_WIN
	if (m_hWnd && m_hwnd != (HWND)pNPWindow->window)
		ambulant::lib::logger::get_logger()->trace("npambulant: HWND changed from 0x%x to 0x%x", m_hWnd, (HWND)pNPWindow->window);
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
	return init_ambulant(m_pNPInstance, m_Window);
}

void
npambulant::shut() {
#ifdef XP_WIN
	if (m_hWnd) {
		// reset the userdata association
		if (m_OldWindow)
			SetWindowLong(m_hWnd, GWL_USERDATA, m_OldWindow);
		// subclass it back
		if (m_lpOldProc);
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
		delete m_mainloop;
	}
#endif
	m_ambulant_player = NULL; // deleted by mainloop
	m_bInitialized = FALSE;
    //XXXX SDL_Quit() forgets to do clear the environment variable ESD_NO_SPAWN
    // the variable was included in the environment by calling  putenv(char* string),
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
#ifdef XP_MAC
	// XXXJACK thinks this is nonsense.... Old debug code??
	NPEvent* ev = (NPEvent*)event;
	if (m_Window) {
		Rect box = { 
			m_Window->y, m_Window->x,
			m_Window->y + m_Window->height, m_Window->x + m_Window->width };
		if (ev->what == updateEvt) {
			::TETextBox(m_String, strlen(m_String), &box, teJustCenter);
		}
	}
#endif
	return 0;
}

// this will start AmbulantPlayer
void
npambulant::startPlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::startPlayer()\n");
	if (m_ambulant_player != NULL)
		get_player()->start();
}

// this will stop AmbulantPlayer
void
npambulant::stopPlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::stopPlayer()\n");
	if (m_ambulant_player != NULL)
		get_player()->stop();
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
	if (m_ambulant_player != NULL)
		get_player()->pause();
}

// this will resume AmbulantPlayer
void
npambulant::resumePlayer() {
	AM_DBG lib::logger::get_logger()->debug("npambulant::resumePlayer()\n");
	if (m_ambulant_player != NULL)
		get_player()->resume();
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
	if (version)
		strcpy(version, ua);
}

NPObject *
npambulant::GetScriptableObject() {
	DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject, AllocateScriptablePluginObject);//KB
	if (!m_pScriptableObject) {
		m_pScriptableObject = NPN_CreateObject(
			m_pNPInstance,
			GET_NPOBJECT_CLASS(ScriptablePluginObject));
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
	if (s_npambulant_last_instance)
		NPN_Status(s_npambulant_last_instance, message);
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
