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

#ifndef __CPLUGIN_H__
#define __CPLUGIN_H__

#ifdef __LP64__
// Hack: 1.9.2 npai.h on MacOS 64 bit does not include next file (for good reasons) so we do it...
#include <Carbon/Carbon.h>
#endif
#include "npapi.h"
#include "npruntime.h"
#include "prtypes.h"

//
// Some header files and typedefs have changed between Firefox 3.0 and 3.5.
// The code currently uses the old names, and typedefs them to the new ones
// if required.
//
#if (NP_VERSION_MAJOR >= 0) && (NP_VERSION_MINOR >= 22)
#include "npfunctions.h"
#ifndef XP_WIN
typedef int32_t int32;
typedef int16_t int16;
#endif
typedef void JRIEnv;
typedef void *jref;
#define NewNPP_NewProc(x) (x)
#define NewNPP_DestroyProc(x) (x)
#define NewNPP_SetWindowProc(x) (x)
#define NewNPP_NewStreamProc(x) (x)
#define NewNPP_DestroyStreamProc(x) (x)
#define NewNPP_StreamAsFileProc(x) (x)
#define NewNPP_WriteReadyProc(x) (x)
#define NewNPP_WriteProc(x) (x)
#define NewNPP_PrintProc(x) (x)
#define NewNPP_URLNotifyProc(x) (x)
#define NewNPP_GetValueProc(x) (x)
#else
#include "npupp.h"
#endif // (NP_VERSION_MAJOR >= 0) && (NP_VERSION_MINOR >= 22)


// ambulant player includes
#include "ambulant/version.h"
#include "ambulant/common/player.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/logger.h"
// mozilla includes
#ifdef	XP_WIN32
#include "npapi.h"
#endif//XP_WIN32

// ambulant player includes
#include "ambulant/version.h"
#include "ambulant/common/player.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/logger.h"

// graphic toolkit includes
#ifdef	MOZ_X11
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#endif // MOZ_X11
#ifdef	WITH_GTK
class gtk_mainloop;
#elif WITH_CG
class cg_mainloop;
#elif XP_WIN32
#ifdef WITH_D2D
#include "ambulant/gui/d2/d2_player.h"
typedef ambulant::gui::d2::d2_player ambulant_gui_player;
typedef ambulant::gui::d2::d2_player_callbacks gui_callbacks; //XX from MmView.cpp
typedef ambulant::gui::d2::d2_player_callbacks ambulant_baseclass_player_callbacks;
#else
#include "ambulant/gui/dx/dx_player.h"
typedef ambulant::gui::dx::dx_player ambulant_gui_player;
typedef ambulant::gui::dx::dx_player_callbacks gui_callbacks;
typedef ambulant::gui::dx::dx_player_callbacks ambulant_baseclass_player_callbacks;
#endif // WITH_D2D
#include <ambulant/net/url.h>
class ambulant_player_callbacks : public ambulant_baseclass_player_callbacks {

  public:
	ambulant_player_callbacks();
	void set_os_window(HWND hwnd);
	HWND new_os_window();
	SIZE get_default_size();
	void destroy_os_window(HWND);
	html_browser *new_html_browser(int left, int top, int width, int height);
	HWND m_hwnd;
};
#else
#error None of WITH_GTK/WITH_CG/XP_WIN32 defined: no graphic toolkit available
#endif//WITH_GTK/WITH_CG/XP_WIN32

extern NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass); //KB

class npambulant
{
  private:
	NPMIMEType m_mimetype;
	NPP m_pNPInstance;
	uint16 m_mode;
	int m_argc;
	char** m_argn;
	char** m_argv;
	NPSavedData* m_data;
	bool m_autostart;
	NPObject* m_window_obj;

#ifdef XP_WIN
	HWND m_hWnd;
	WNDPROC m_lpOldProc;
	LONG m_OldWindow;
#endif

	NPWindow * m_Window;

	NPStream * m_pNPStream;
	NPBool m_bInitialized;

	NPObject *m_pScriptableObject;

  public:
	char m_String[128];

  public:
	npambulant(
		NPMIMEType mimetype,
		NPP pNPInstance,
		uint16 mode,
		int argc,
		char* argn[],
		char* argv[],
		NPSavedData* data);
	~npambulant();

	NPBool setWindow(NPWindow* pNPWindow);
	NPBool init();
	void shut();
	NPBool isInitialized();
	// next functions are overridden to start the plugin late
	// this solves problems with concurrent plugins like greasemonkey
	// a more elegant approach could be to do all initialization in the plugin's
	// init() function and to start the player when WriteReady() is called.
	NPError SetWindow(NPWindow* pNPWindow);
	NPError NewStream(NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
	NPP getNPP();
	const char* getValue(const char *name);

	int16 handleEvent(void* event);

	void showVersion();
	void clear();
	void getVersion(char* *aVersion);

	void startPlayer();
	void stopPlayer();
	void restartPlayer();
	void pausePlayer();
	void resumePlayer();
	bool isDone();

	NPObject *GetScriptableObject();

	/* for ambulant player */
	ambulant::lib::logger* m_logger;
	ambulant::net::url m_url;
	int m_cursor_id;

	static NPP s_last_instance;
	bool init_ambulant(NPP npp, NPWindow* aWindow);
	char* get_document_location();
#ifdef	MOZ_X11
	Window window;
	Display* display;
	int width, height;
#endif // MOZ_X11

#ifdef WITH_GTK
	gtk_mainloop* m_mainloop;
#elif WITH_CG
	cg_mainloop *m_mainloop;
#else
	void *m_mainloop;
#endif

#ifdef	XP_WIN32
#define strcasecmp(s1,s2) _stricmp(s1,s2)
	ambulant_player_callbacks m_player_callbacks;
	HWND m_hwnd;
	ambulant_gui_player* m_ambulant_player;
	ambulant::common::player* get_player() {
		return m_ambulant_player->get_player();
	}
#else //!XP_WIN32
	ambulant::common::player* m_ambulant_player;
	ambulant::common::player* get_player() {
		return m_ambulant_player;
	}
#endif // XP_WIN32
};

extern "C" {
void npambulant_display_message(int level, const char *message);
};

#endif // __CPLUGIN_H__
