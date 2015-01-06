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

/* 
 * sdl_gui.cpp - SDL GUI for Ambulant
 *               intially keyboard based
 *               could look like iAmbulant
 */
// TBD: visual gui, file/url selection, preferences, error logger

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include "sdl_gui.h"
#include "sdl_gui_player.h"
#include "unix_preferences.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "SDL/Ambulant", __VA_ARGS__))
#endif//__ANDROID__
#include "sdl_logger.h"
#include "ambulant/config/config.h"

#if  TBD
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#endif//TBD

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#undef WITH_SDL_LOGGER

#ifndef AMBULANT_DATADIR
#define AMBULANT_DATADIR "/usr/local/share/ambulant"
#endif

const char *about_text =
	"Ambulant SMIL 3.0 player.\n"
	"Version: %s\n"
	"Copyright Stichting CWI, 2003-2012.\n\n"
	"License: LGPL";


// Places where to look for the Welcome document
const char *welcome_locations[] = {
	"Welcome/Welcome.smil",
	"../Welcome/Welcome.smil",
	"Extras/Welcome/Welcome.smil",
	"../Extras/Welcome/Welcome.smil",
	AMBULANT_DATADIR "/Welcome/Welcome.smil",
	NULL
};

// Places where to look for the helpfile
const char *helpfile_locations[] = {
	"Documentation/user/index.html",
	"../Documentation/user/index.html",
	AMBULANT_DATADIR "/AmbulantPlayerHelp/index.html",
	NULL
};

sdl_gui::sdl_gui(const char* title, const char* initfile)
:
	m_programfilename(NULL),
	m_smilfilename(initfile),
	m_toplevelcontainer(NULL),
	m_guicontainer(NULL),
	m_documentcontainer(NULL),
	m_window(NULL),
	m_arrow_cursor(NULL),
	m_hand_cursor(NULL)
{
	// create the player
	m_gui_player = new sdl_gui_player(this);
	m_toplevelcontainer = m_documentcontainer = m_window = m_gui_player->get_window();
	SDL_SetWindowTitle(m_toplevelcontainer, title);
#ifndef __ANDROID__
	m_arrow_cursor = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	m_hand_cursor = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_HAND);
	SDL_SetCursor (m_arrow_cursor);
#endif//#ifndef __ANDROID__
}

sdl_gui::~sdl_gui() {

	// remove all dynamic data in reverse order as they are constructed
	// m_programfilename - not dynamic
	if (m_arrow_cursor != NULL) {
		SDL_FreeCursor (m_arrow_cursor);
	}
	if (m_hand_cursor != NULL) {
		SDL_FreeCursor (m_hand_cursor);
	}
	if (m_smilfilename != NULL) {
		free((void*)m_smilfilename);
		m_smilfilename = NULL;
	}
	if (m_window != NULL) {
//XXX		SDL_DestroyWindow(m_window);

}
	if (m_gui_player) {
		delete m_gui_player;
		m_gui_player = NULL;
	}
}

SDL_Window*
sdl_gui::get_document_container()
{
	return this->m_documentcontainer;
}

#ifdef WITH_SDL2


bool
sdl_gui::openSMILfile(const char *smilfilename, int mode, bool dupFlag) {

	if (smilfilename==NULL)
		return false;

	SDL_SetWindowTitle(m_toplevelcontainer, smilfilename);
	if (dupFlag) {
		if (m_smilfilename) free ((void*)m_smilfilename);
		m_smilfilename = strdup(smilfilename);
	}
	if (m_gui_player)
		delete m_gui_player;

	m_gui_player = new sdl_gui_player(this);
	assert (m_gui_player);
	return m_gui_player->is_open();
}

void
sdl_gui::quit() {
	if (m_gui_player) {
		m_gui_player->stop();
		delete m_gui_player;
		m_gui_player = NULL;
	}
	SDL_Quit ();
}

// sdl_gather_events collects  all window and redraw events into a single redraw() call
// using SDL_PeepEvents()
void
sdl_gather_events () {

}

void
sdl_gui::sdl_loop() {
	bool busy = true;
	SDL_Event event;

	while (busy) {
		if (SDL_WaitEvent(&event) == 0) {
			lib::logger::get_logger()->fatal("ambulant::sdl_gui::sdl_loop(0x%x): SDL error %s", this, SDL_GetError());
			busy = false;
		}
		switch (event.type) {
		case SDL_QUIT:
			lib::logger::get_logger()->trace("ambulant::sdl_gui::sdl_loop(0x%x): SDL_QUIT", this);
			busy = false;
			break;
		case SDL_USEREVENT:
			AM_DBG lib::logger::get_logger()->debug("%s SDL_USEREVENT: code=%d data1=0x%x data2=0x%x",__PRETTY_FUNCTION__, event.user.code,event.user.data1,event.user.data2);
			if (event.user.code == 317107) {
				if (m_gui_player != NULL) {
					m_gui_player->redraw(event.user.data1, event.user.data2);
					free (event.user.data2); // malloc'd by need_redraw()
				}
 			}
			break;
		case SDL_WINDOWEVENT:
			ambulant::gui::sdl::ambulant_sdl_window* asw; //XX no refs to 'ambulant' 
			ambulant::gui::sdl::sdl_ambulant_window* saw;
			saw = ambulant::gui::sdl::sdl_ambulant_window::get_sdl_ambulant_window (event.window.windowID);
			AM_DBG lib::logger::get_logger()->debug("%s SDL_WINDOWEVENT: type=%d windowID=%d code=%d data1=0x%x data2=0x%x saw=0x%x",__PRETTY_FUNCTION__, event.window.type, event.window.windowID, event.window.event,event.window.data1,event.window.data2, saw);
			if (saw != NULL && (asw = saw->get_ambulant_sdl_window()) != NULL) {
				common::player* player = m_gui_player->get_player();
				if (player != NULL && saw->get_evp() == NULL) {
					// First window event after creation, complete initialization
					saw->set_evp(player->get_evp()); // for timestamps
					saw->get_ambulant_sdl_window()->set_gui_player (m_gui_player);
				}
				ambulant::lib::rect r; //XX no refs to 'ambulant' 
				switch ( event.window.event ) {
				case  SDL_WINDOWEVENT_SHOWN:
				case  SDL_WINDOWEVENT_EXPOSED:
				case  SDL_WINDOWEVENT_FOCUS_GAINED:
					r = asw->get_bounds();
					asw->redraw(r);
					break;
				case  SDL_WINDOWEVENT_RESIZED:
					m_gui_player->resize_window (event.window.data1,event.window.data2);
					break;
				case  SDL_WINDOWEVENT_MINIMIZED:
					busy = false; // tmp. for android
				default:  
					break;
				}
			}
			break;
		case SDL_MOUSEMOTION: // mouse moved
			AM_DBG lib::logger::get_logger()->debug("%s SDL_MOUSEMOTION: type=%d windowID=%d which=%d state=%d x=%d y=%d relx=%d rely=%d",__PRETTY_FUNCTION__, event.motion.type,  event.motion.windowID, event.motion.which, event.motion.state,event.motion.x,event.motion.y,event.motion.xrel,event.motion.yrel);
			if (m_gui_player != NULL) {
				SDL_Point p;
				p.x = event.motion.x;
				p.y = event.motion.y;
				m_gui_player->before_mousemove(0);
				m_gui_player->user_event(p, 1);
				if (m_gui_player->after_mousemove()) {
					SDL_SetCursor (m_hand_cursor);
				} else {
					SDL_SetCursor (m_arrow_cursor);
				}	
			}
			break;
		case SDL_MOUSEBUTTONDOWN: // mouse button pressed
		case SDL_MOUSEBUTTONUP: // mouse button released
			AM_DBG lib::logger::get_logger()->debug("%s %s: type=%d windowID=%d which=%d button=%d, state=%d, x=%d y=%d",__PRETTY_FUNCTION__, event.button.state ? "SDL_MOUSEBUTTONDOWN":"SDL_MOUSEBUTTONUP", event.button.type,  event.button.windowID, event.button.which, event.button.button, event.button.state ,event.button.x,event.button.y);
			if (m_gui_player != NULL && event.button.state == 0) { // button released
				SDL_Point p;
				p.x = event.motion.x;
				p.y = event.motion.y;
				m_gui_player->user_event(p, 0);
			}
			break;
		case SDL_MOUSEWHEEL: // mouse wheel motion
			break;
#ifdef TBD
		case SDL_FINGERDOWN: // finger pressed
		case SDL_FINGERUP: // finger released
			AM_DBG lib::logger::get_logger()->debug("%s %s: type=%d touchId=%d fingerId=%d, x=%f y=%f",__PRETTY_FUNCTION__, event.type == SDL_FINGERDOWN ? "SDL_FINGERDOWN":"SDL_FINGERUP", event.tfinger.type,  event.tfinger.touchId, event.tfinger.fingerId ,event.tfinger.x,event.tfinger.y);
			if (m_gui_player != NULL && event.type == SDL_FINGERUP) { // finger released
				SDL_Point p;
				int w, h;
				SDL_GetWindowSize(get_window(), &w, &h);
				p.x = round(w*event.tfinger.x);
				p.y = round(h*event.tfinger.y);
				m_gui_player->user_event(p, 0);
			}
			break;
#endif//TBD
		default:
			break;
		}
	}
}

#ifdef ANDROID
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "SDL/Ambulant", __VA_ARGS__))
#include <jni.h>
extern "C" {
	/* Called before SDL_main() to initialize JNI bindings in SDL library */
	extern void SDL_Android_Init(JNIEnv* env, jclass cls);

	// Helper: Return a public static string from a java class 
	char* get_static_string_from_java_class (JNIEnv* jep, const char* class_name, const char* var_name, const char* signature) {
		// signatures can be found by: javap -s <javaclass>.class
		char* rv = NULL;
		jclass classID = jep->FindClass(class_name);
		LOGI("%s: class_name=%s var_name=%s signature=%s", "get_static_string_from_java_class", class_name, var_name, signature);
		if (classID == NULL) return NULL;
		jstring javastring = NULL;
		jfieldID fieldID = jep->GetStaticFieldID(classID, var_name, signature);
		if (fieldID != NULL) {
			javastring = (jstring) jep->GetStaticObjectField (classID, fieldID);
		} else LOGI("fieldID==NULL");
		if (javastring != NULL) {
			jboolean isCopy = JNI_FALSE;
			rv = (char*) jep->GetStringUTFChars (javastring, &isCopy);
			if (isCopy) jep->DeleteLocalRef((jobject)javastring);  //release
		} else LOGI("javastring==NULL");

		if (fieldID != NULL) { jep->DeleteLocalRef((jobject)fieldID); /*release*/ }
		if (classID != NULL) { jep->DeleteLocalRef((jobject)classID);/*release*/ }
		return rv;
	}

	/* Start up the SDL app */
	void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jstring path)
	{
		/* This interface could expand with ABI negotiation, calbacks, etc. */
		LOGI("native_init start");
		setenv ("SDL_WINDOW_FLAGS","1" /*SDL_WINDOW_FULLSCREEN*/, 1);
//XX		setenv ("AMBULANT_FFMPEG_DEBUG","1", 1);
//XX		setenv ("FFREPORT","/sdcard/ffreport", 1);
		SDL_Android_Init(env, cls);

		LOGI("native_init: calling SDL_SetMainReady()");
		SDL_SetMainReady();
		LOGI("native_init: returned from SDL_SetMainReady()");
		
		/* Run the application code! */
		int status;
		char *argv[3];
		jboolean isCopy;
//		char *str_path = (char*) env->GetStringUTFChars(path, &isCopy);
		argv[0] = SDL_strdup("AmbulantPlayer_SDL");
//		argv[1] =	str_path;
//		char cwd[512];
//		getcwd(cwd, 512);
//		LOGI("path=%s", cwd);
		const char* data_dir = "/sdcard/";//get_static_string_from_java_class (env, "org.libsdl.app.AmbulantSDLPlayer", "data_dir", "Ljava/lang/String;");
		LOGI("data_dir = %s", data_dir);
		char buf[512];
		sprintf (buf,"%s/%s", data_dir, "AmbulantSDLPlayerInfo.smil");
		argv[1]= SDL_strdup(buf);
		system ("echo \"<smil> <body> <par dur=\\\"600\\\" >"
			"<smilText begin=\\\"1\\\" textPlace=\\\"start\\\" textFontSize=\\\"medium\\\" textFontWeight=\\\"normal\\\" >"
			"<span textPlace=\\\"center\\\" textFontSize=\\\"x-large\\\" textFontWeight=\\\"bold\\\" >AmbulantSDLPlayer</span>"
			"<br/><br/>AmbulantSDLPlayer is a SMIL3.0 Player for Android.<br/>"
			"It can be activated by tapping a downloaded SMIL file in a suitable File Manager (e.g. Total Commander) or by "
			"tapping a link to a SMIL file in a Web Browser (e.g. Chrome), such as the links marked "
			"'http' at www.ambulantplayer.org/Demos.<br/><br/>AmbulantSDLPlayer can be stopped by tapping the BackButton "
			"(bottom middle left).<br/><br/>Enjoy !</smilText></par></body></smil>\" >/sdcard/AmbulantSDLPlayerInfo.smil");
//		argv[1]= SDL_strdup("http://homepages.cwi.nl/~kees/ambulant/Welcome/Welcome.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/Welcome/Welcome.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/Welcome/Welcome-smiltext.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/smilText/NYC-sT.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/PanZoom/Fruits-4s.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/Birthday/HappyBirthday.smil");
//		argv[1]= SDL_strdup("http://ambulantplayer.org/Demos/Birthday/HappyBirthday.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/News/DanesV2-Desktop.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/Euros/EUROshow.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/Flashlight/Flashlight/Flashlight-US.smil");
//		argv[1]= SDL_strdup("/sdcard/Download/VideoTests/VideoTests.smil");
		char* url = get_static_string_from_java_class (env, "org.libsdl.app.AmbulantSDLPlayer", "my_string", "Ljava/lang/String;");
		if (url != NULL) {
			argv[1] = url;
		}
		LOGI("argv[1]=%s url=%s", argv[1], url == NULL ? "<null>":url);
		argv[2] = NULL;
		status = SDL_main(2, argv);
//   env->ReleaseStringUTFChars(path, str_path);

	/* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
	/* exit(status); */
	}
}//extern "C"
#endif // ANDROID

int
main (int argc, char*argv[]) {

//#ifndef ANDROID_MAIN
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		fprintf (stderr, "Ambulant: SDL_Init failed: %s\n", SDL_GetError());
		exit (-1);
	}
	int img_flags = 0; // XXXJACK: don't think we need these: IMG_INIT_JPG | IMG_INIT_PNG ; 
#ifdef WITH_SDL_IMAGE
	if (IMG_Init(img_flags) != img_flags) {
		fprintf (stderr, "Ambulant: IMG_Init failed: %s\n", SDL_GetError());
		exit (-2);
	}
#endif // WITH_SDL_IMAGE
#ifdef	ENABLE_NLS
	// Load localisation database
	bool private_locale = false;
	char *home = getenv("HOME");
	if (home) {
		std::string localedir = std::string(home) + "/.ambulant/locale";
		if (access(localedir.c_str(), 0) >= 0) {
			private_locale = true;
			bindtextdomain(PACKAGE, localedir.c_str());
		}
	}
	if (!private_locale)
		bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif /*ENABLE_NLS*/

	// Load preferences, initialize app and logger
	unix_preferences unix_prefs;
	unix_prefs.load_preferences();
// TBD begin code from gtk_logger
	common::preferences* prefs = common::preferences::get_preferences();
	lib::logger* logger = lib::logger::get_logger();
	if (logger == NULL) {
		printf("Programmer error: could not create '%s' data stucture\n", "logger");
		exit(-1);
	}
	if (prefs == NULL) {
		logger->error("Programmer error: could not create '%s' data stucture", "preferences");
		exit(-1);
	}
	// Connect logger to our message displayer and output processor
	// logger->set_show_message(show_message);
	// Tell the logger about the output level preference
	int level = prefs->m_log_level;
	logger->set_level(level);
// TBD end code from gtk_logger

//X No inital window, logging on console
	/* Setup surface */
//X	sdl_gui *gui = new sdl_gui(argv[0], NULL);
	// take log level from preferences
//X	sdl_logger::set_sdl_logger_gui(gui);
	sdl_logger* sdl_logger = sdl_logger::get_sdl_logger();
	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
//X	// Print welcome banner
//   	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
//	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/SDL"), __DATE__);
   	lib::logger::get_logger()->debug("Ambulant Player: compile time version %s, runtime version %s", AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug("Ambulant Player: built on %s for Unix/SDL", __DATE__);
#if ENABLE_NLS
//X	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif

	bool exec_flag = true; // for make check

	// establish the .smil document to play
	char* document_url = NULL;

//X no initial Welcome display
//X	if (argc <= 1) {
//X		if (prefs && ! prefs->m_welcome_seen) {
//X			document_url = (char*) find_datafile(welcome_locations);
//X			prefs->m_welcome_seen = true;
//X		}
//X		exec_flag = true;
//X	} else {
		if (argc == 2) {
			document_url = argv[1];
		}
//X	}
#ifdef ANDROID
//    document_url="/mnt/sdcard/smil/Fruits-4shgb.smil";
  LOGI("Document is %s", document_url);
#endif // android
	if (document_url == NULL) {
		logger->error("Usage: %s <filename>|<url>", argv[0]);
		exit(-1);
	}    
	// If the URL starts with "ambulant:" this is the trick-uri-scheme to
	// open URLs in Ambulant from the browser (iOS). Remove the trick.
	char last[6];
	if (strncmp(document_url, "ambulant:", 9) == 0)
		document_url += 9;

	int len = strlen(document_url);
	strcpy(last, &document_url[len-5]);
	if (strcmp(last, ".smil") != 0
		&& strcmp(&last[1], ".smi") != 0
		& strcmp(&last[1], ".sml") != 0) {
		logger->error("<filename> or <url> should end with '%s': '%s' or '%s'", ".smil", ".smi", ".sml");
		exit(-1);
	}
	sdl_gui *gui = new sdl_gui(argv[0], strdup(document_url));
	if (gui == NULL) {
		logger->error("Programmer error: could not create '%s' data structure",  "gui_player");
		exit(-1);
	}
	gui->m_gui_player->play();
#ifdef ANDROID
	LOGI("Starting sdl_loop()");
#endif // ANDROID
	gui->sdl_loop();
 #ifdef __ANDROID__
//http://ambulantplayer.org/Demos/PanZoom/Fruits-4s.smil
//	document_url=(char*)"/mnt/sdcard/smil/Fruits-4s.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/smilText/NYC-sT.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/PanZoom/Fruits-4s.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/VideoTests/VideoTests.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/Birthday/HappyBirthday.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/Flashlight/Flashlight-US.smil";
//	document_url=(char*)"http://ambulantplayer.org/Demos/News/DanesV2-Desktop.smil";
//	document_url=(char*)"http://homepages.cwi.nl/~kees/ambulant/Welcome/Welcome.smil";
//	document_url=(char*)"http://homepages.cwi.nl/~kees/ambulant/Welcome/Welcome-smiltext.smil";


	LOGI("After SDL loop");
#endif // android
	prefs->save_preferences();
	// delete logger; // logger will be deleted by loggers_manager called at exit()
	gui->quit();
  delete gui;
#ifdef ANDROID
	LOGI("Deleted gui");
	gui = NULL;
#endif // ANDROID
	SDL_Quit();
#ifdef ANDROID
	LOGI("SDL Quit done");
	gui = NULL;
#endif // ANDROID
	return exec_flag ? 0 : -1;
//#endif//#ifndef  ANDROID_MAIN
}

#endif//WITH_SDL2
