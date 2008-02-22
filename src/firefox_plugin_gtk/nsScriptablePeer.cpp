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

// ==============================
// ! Scriptability related code !
// ==============================

/////////////////////////////////////////////////////
//
// This file implements the nsScriptablePeer object
// The native methods of this class are supposed to
// be callable from JavaScript
//
#define DEBUG
#include "nsScriptablePeer.h"
#ifdef	MOZILLA_TRUNK
#include "xpconnect/nsIXPConnect.h"
#include "jscntxt.h"
#include "jsobj.h"
#endif//MOZILLA_TRUNK

static NS_DEFINE_IID(kIScriptableIID, NS_IAMBULANTPLUGIN_IID);
static NS_DEFINE_IID(kIClassInfoIID, NS_ICLASSINFO_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
#ifdef	MOZILLA_TRUNK
static NS_DEFINE_IID(kIXPConnectWrappedJSIID, NS_IXPCONNECTWRAPPEDJS_IID);
#endif//MOZILLA_TRUNK

nsScriptablePeer::nsScriptablePeer(nsPluginInstance* aPlugin)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::nsScriptablePeer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"aPlugin",aPlugin);
#endif
  mPlugin = aPlugin;
  mRefCnt = 0;
}

nsScriptablePeer::~nsScriptablePeer()
{
#ifdef DEBUG
    char *id = "nsScriptablePeer:~:nsScriptablePeer";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"<empty>",0);
#endif
}

// AddRef, Release and QueryInterface are common methods and must 
// be implemented for any interface
NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::AddRef() 
{ 
#ifdef DEBUG
    char *id = "nsScriptablePeer::AddRef";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"mRefCnt",mRefCnt);
#endif
  ++mRefCnt; 
  return mRefCnt; 
} 

NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::Release() 
{ 
#ifdef DEBUG
    char *id = "nsScriptablePeer::Release";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"mRefCnt",mRefCnt);
#endif
  --mRefCnt; 
  if (mRefCnt == 0) { 
    delete this;
    return 0; 
  } 
  return mRefCnt; 
} 

// here nsScriptablePeer should return three interfaces it can be asked for by their iid's
// static casts are necessary to ensure that correct pointer is returned
NS_IMETHODIMP nsScriptablePeer::QueryInterface(const nsIID& aIID, void** aInstancePtr) 
{ 
#ifdef DEBUG
    char *id = "nsScriptablePeer::QueryInterface";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"aIID",aIID);
#endif
  if(!aInstancePtr) 
    return NS_ERROR_NULL_POINTER; 

  if(aIID.Equals(kIScriptableIID)) {
      *aInstancePtr = static_cast<nsIAmbulantPlugin*>(this);
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kIClassInfoIID)) {
    *aInstancePtr = static_cast<nsIClassInfo*>(this); 
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kISupportsIID)) {
      *aInstancePtr = static_cast<nsIAmbulantPlugin*>(this); 
    AddRef();
    return NS_OK;
  }
  /*
   //XXXX mozilla trunk static_cast<nsIXPConnectWrappedJS*>
  if(aIID.Equals(kIXPConnectWrappedJSIID)) {
    *aInstancePtr = static_cast<nsIAmbulantPlugin*>(this);
    AddRef();
    return NS_OK;
  }
  */
  return NS_NOINTERFACE; 
}

void nsScriptablePeer::SetInstance(nsPluginInstance* plugin)
{
#ifdef DEBUG
    char *id = "";
    fprintf(stderr, "%s(%x): %s=%d.\n",id,this,"plugin",plugin);
#endif
  mPlugin = plugin;
}

//
// the following methods will be callable from JavaScript
//

NS_IMETHODIMP nsScriptablePeer::GetLocation(char** aloc)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::Getlocation";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->getLocation();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}
NS_IMETHODIMP nsScriptablePeer::SetLocation(char const* aloc)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::Setlocation";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->setLocation();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}
NS_IMETHODIMP nsScriptablePeer::GetProperty(char const* prop, char** val)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::Getproperty";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->getProperty();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}
NS_IMETHODIMP nsScriptablePeer::SetProperty(char const* prop, char const* val)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::Setproperty";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->setProperty();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::StartPlayer(void)
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::StartPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->startPlayer();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::StopPlayer()
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::StopPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->stopPlayer();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::RestartPlayer()
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::RestartPlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->restartPlayer();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::ResumePlayer()
{
#ifdef DEBUG
    char *id = " nsScriptablePeer::ResumePlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->resumePlayer();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::PausePlayer()
{
#ifdef DEBUG
    char *id = "nsScriptablePeer::PausePlayer";
    fprintf(stderr, "%s(%x): %s=0x%x.\n",id,this,"mPlugin",mPlugin);
#endif
#ifdef   AMBULANT_FIREFOX_PLUGIN
  if (mPlugin)
    mPlugin->pausePlayer();
#endif // AMBULANT_FIREFOX_PLUGIN

  return NS_OK;
}
