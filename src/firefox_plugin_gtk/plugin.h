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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "pluginbase.h"
#include "nsScriptablePeer.h"
#undef MOZ_X11
#define DEBUG
#ifdef	XP_UNIX
#ifdef	MOZ_X11
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#endif/*MOZ_X11*/
#endif/*XP_UNIX*/

//#define AMBULANT_FIREFOX_PLUGIN
#ifdef   AMBULANT_FIREFOX_PLUGIN
#include <ambulant/version.h>
#include <ambulant/common/player.h>
//#include <ambulant/gui/gtk/gtk_factory.h>
//#include <ambulant/net/url.h>
//#include "../src/player_gtk/gtk_mainloop.h"
    // forward declarations of classes needed by the implementation
class gtk_mainloop; // borrowd from player_gtk
class ambulant::common::player;
class ambulant::lib::logger;
#endif // AMBULANT_FIREFOX_PLUGIN

class nsScriptablePeer;

class nsPluginInstance : public nsPluginInstanceBase, nsIAmbulantPlugin
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAMBULANTPLUGIN

  nsPluginInstance(NPP aInstance);
  ~nsPluginInstance();

  NPBool init(NPWindow* aWindow);
  void shut();
  NPBool isInitialized();

  // we need to provide implementation of this method as it will be
  // used by Mozilla to retrive the scriptable peer
  NPError	GetValue(NPPVariable variable, void *value);

  // locals
#ifndef   AMBULANT_FIREFOX_PLUGIN
  void showVersion();
  void clear();
#else //  AMBULANT_FIREFOX_PLUGIN
  void startPlayer();
  void stopPlayer();
  void restartPlayer();
  void resumePlayer();
  void pausePlayer();
#endif // AMBULANT_FIREFOX_PLUGIN
  nsScriptablePeer* getScriptablePeer();

private:
  NPWindow* mNPWindow;
  NPP mInstance;
  NPBool mInitialized;
  nsScriptablePeer * mScriptablePeer;

  char* get_document_location();

public:
  char mString[128];
#ifdef	XP_UNIX
  Window window;
  Display* display;
  int width, height;
#endif/*XP_UNIX*/
#ifdef   AMBULANT_FIREFOX_PLUGIN
    nsPluginCreateData mCreateData;
    gtk_mainloop* m_mainloop;
    ambulant::lib::logger* m_logger;
    ambulant::common::player* m_ambulant_player;
//KB ambulant::net::url m_url;
//KB ambulant_player_callbacks m_player_callbacks;
#ifdef	XP_WIN
  HWND m_hwnd;
#endif/*XP_WIN*/

  int m_cursor_id;

  NPP getNPP();
  const char* getValue(const char *name);
  const char * getVersion();
#endif // AMBULANT_FIREFOX_PLUGIN
};
#endif // __PLUGIN_H__
