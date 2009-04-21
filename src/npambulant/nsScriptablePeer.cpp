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
#ifdef	XP_WIN32
#undef _GLOBAL_USING
#include <cstddef>		   // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#endif//XP_WIN32

#include "nsScriptablePeer.h"
#ifdef	MOZILLA_TRUNK
#include "xpconnect/nsIXPConnect.h"
#include "jscntxt.h"
#include "jsobj.h"
#endif //MOZILLA_TRUNK

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

static NS_DEFINE_IID(kIScriptableIID, NPAMBULANT_IID);
static NS_DEFINE_IID(kIClassInfoIID, NS_ICLASSINFO_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
#ifdef	MOZILLA_TRUNK
static NS_DEFINE_IID(kIXPConnectWrappedJSIID, NS_IXPCONNECTWRAPPEDJS_IID);
#endif //MOZILLA_TRUNK

nsScriptablePeer::nsScriptablePeer(nsPluginInstance* aPlugin)
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::nsScriptablePeer(0x%x) creating 0x%x\n", (void*)aPlugin, (void*)this);
  mPlugin = aPlugin;
  mRefCnt = 0;
}

nsScriptablePeer::~nsScriptablePeer()
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::~nsScriptablePeer() for 0x%x\n", (void*)this);
}

// AddRef, Release and QueryInterface are common methods and must 
// be implemented for any interface
NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::AddRef() 
{ 
  AM_DBG fprintf(stderr, "nsScriptablePeer::AddRef() for 0x%x\n", (void*)this);
  ++mRefCnt; 
  return mRefCnt; 
} 

NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::Release() 
{ 
  AM_DBG fprintf(stderr, "nsScriptablePeer::Release() for 0x%x\n", (void*)this);
  --mRefCnt; 
  if (mRefCnt == 0) { 
#ifdef	AMBULANT_PLATFORM_WIN32
	  if (mPlugin)
		  mPlugin->mScriptablePeer = NULL;
      delete this;
#endif//AMBULANT_PLATFORM_WIN32
      return 0; 
  } 
  return mRefCnt; 
} 
// here nsScriptablePeer should return three interfaces it can be asked for by their iid's
// static casts are necessary to ensure that correct pointer is returned
NS_IMETHODIMP nsScriptablePeer::QueryInterface(const nsIID& aIID, void** aInstancePtr) 
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::QueryInterface()\n"); // Parameter too difficult for now
  if(!aInstancePtr) 
    return NS_ERROR_NULL_POINTER; 

  if(aIID.Equals(kIScriptableIID)) {
      *aInstancePtr = static_cast<npambulant*>(this);
    AddRef();
	AM_DBG  fprintf(stderr, "nsScriptablePeer::QueryInterface: return kIScriptableIID 0x%x\n", *aInstancePtr);
    return NS_OK;
  }

  if(aIID.Equals(kIClassInfoIID)) {
    *aInstancePtr = static_cast<nsIClassInfo*>(this); 
    AddRef();
	AM_DBG  fprintf(stderr, "nsScriptablePeer::QueryInterface: return kIClassInfoIID 0x%x\n", *aInstancePtr);
    return NS_OK;
  }

  if(aIID.Equals(kISupportsIID)) {
      *aInstancePtr = static_cast<npambulant*>(this); 
    AddRef();
	AM_DBG  fprintf(stderr, "nsScriptablePeer::QueryInterface: return kISupportsIID 0x%x\n", *aInstancePtr);
    return NS_OK;
  }
  // Interfaces we've seen queries for:
  // bed52030-bca6-11d2-ba79-00805f8a5dd7 (nsIXPConnectWrappedJS)
  // 9cc0c2e0-f769-4f14-8cd6-2d2d40466f6c (nsIXPCScriptable)
 // AM_DBG fprintf(stderr, "nsScriptablePeer::QueryInterface: return NS_NOINTERFACE error for IID %s\n", aIID.ToString());
  AM_DBG fprintf(stderr, "nsScriptablePeer::QueryInterface: return NS_NOINTERFACE error for IID\n");
  return NS_NOINTERFACE; 
}

void nsScriptablePeer::SetInstance(nsPluginInstance* plugin)
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::SetInstance(0x%x)\n", plugin);
  mPlugin = plugin;
}

//
// the following methods will be callable from JavaScript
//
NS_IMETHODIMP nsScriptablePeer::StartPlayer(void)
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::StartPlayer() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  return mPlugin->StartPlayer();
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::StopPlayer()
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::StopPlayer() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  mPlugin->StopPlayer();
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::RestartPlayer()
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::RestartPlayer() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  mPlugin->RestartPlayer();
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::ResumePlayer()
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::ResumePlayer() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  mPlugin->ResumePlayer();
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::PausePlayer()
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::PausePlayer() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  mPlugin->PausePlayer();
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::IsDone(PRBool *isdone)
{
  AM_DBG fprintf(stderr, "nsScriptablePeer::Isdone() called\n");
  if (mPlugin == NULL) return NS_ERROR_NOT_INITIALIZED;
  mPlugin->IsDone(isdone);
  return NS_OK;
}
