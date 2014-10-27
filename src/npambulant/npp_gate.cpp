/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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

/*AMBULANT_FOREIGN_INDENT_RULES*/

////////////////////////////////////////////////////////////
//
// Implementation of plugin entry points (NPP_*)
// most are just empty stubs for this particular plugin
//
#ifdef	XP_WIN32
#include <cstddef>		   	 // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
//#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#endif//XP_WIN32

#include "npambulant.h"
//#define AM_DBG
#define this (void*)0
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef XP_UNIX
const char*
NPP_GetMIMEDescription(void)
{
	const char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";
	LOG("mimetypes=%s",mimetypes);
	return mimetypes;
}
#endif//XP_UNIX

npambulant * s_npambulant = NULL;

NPError NPP_Initialize(void)
{
	LOG("called.");
	return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
	LOG(" ");
}

// here the plugin creates an instance of our npambulant object which
// will be associated with this newly created plugin instance and
// will do all the neccessary job
NPError NPP_New(NPMIMEType pluginType,
                NPP instance,
                uint16 mode,
                int16 argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
#ifdef WITH_CG
	// We need to request CoreGraphics support in stead of QuickDraw support.
	NPBool supportsCG = false;
	rv = NPN_GetValue(instance, NPNVsupportsCoreGraphicsBool, &supportsCG);
	if (rv) {
		LOG("GetValue(NPNVsupportsCoreGraphicsBool) returned %d\n", rv);
		return rv;
	}
	if (! supportsCG) {
		LOG("Browser does not support NPNVsupportsCoreGraphicsBool\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	rv = NPN_SetValue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreGraphics);
	if (rv) {
		LOG("SetValue(NPDrawingModelCoreGraphics) returned %d\n", rv);
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}

	// select the Cocoa event model
	NPBool supportsCocoaEvents = false;
	if (NPN_GetValue(instance, NPNVsupportsCocoaBool, &supportsCocoaEvents) == NPERR_NO_ERROR && supportsCocoaEvents) {
		NPN_SetValue(instance, NPPVpluginEventModel, (void*)NPEventModelCocoa);
	} else {
		LOG("Cocoa event model not supported, can't create a plugin instance.\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
#endif
	npambulant * pPlugin = new npambulant(pluginType,instance,mode,argc,argn,argv,saved);
	if (pPlugin == NULL) {
		return NPERR_OUT_OF_MEMORY_ERROR;
	}
	s_npambulant = pPlugin;
	instance->pdata = (void *)pPlugin;
	return rv;
}

// here is the place to clean up and destroy the npambulant object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
	npambulant * pPlugin = (npambulant *)instance->pdata;
	if (pPlugin != NULL) {
		pPlugin->shut();
		delete pPlugin;
		if (s_npambulant == pPlugin)
			s_npambulant = NULL;
	}
	return rv;
}

// during this call we know when the plugin window is ready or
// is about to be destroyed so we can do some gui specific
// initialization and shutdown
NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;

	if (pNPWindow == NULL) {
		return NPERR_GENERIC_ERROR;
	}
	npambulant *pPlugin = (npambulant *)instance->pdata;

	if (pPlugin == NULL) {
		return NPERR_GENERIC_ERROR;
	}
	if ( ! pPlugin->setWindow(pNPWindow)) {
		rv = NPERR_GENERIC_ERROR;
	}
	return rv;
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
NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
	LOG("called.");
	if (instance == NULL || value == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
	npambulant * plugin = (npambulant *)instance->pdata;
	if (plugin == NULL) {
		return NPERR_GENERIC_ERROR;
	}
	switch (variable) {
		case NPPVpluginNameString:
			*((const char **)value) = "npambulant";
			break;
		case NPPVpluginDescriptionString:
			*((const char **)value) = "SMIL3.0 player";
			break;
		case NPPVpluginScriptableNPObject:
			*(NPObject **)value = plugin->GetScriptableObject();
			break;
		case NPPVpluginNeedsXEmbed:
			*(NPBool *) value = TRUE;
			break;
		default:
//			rv = NPERR_GENERIC_ERROR;
			break;
	}
	return rv;
}

NPError NPP_NewStream(NPP instance,
                      NPMIMEType type,
                      NPStream* stream,
                      NPBool seekable,
                      uint16* stype)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
	npambulant *pPlugin = (npambulant *)instance->pdata;

	if (pPlugin == NULL) {
		return NPERR_GENERIC_ERROR;
	}
	if (pPlugin->isInitialized()) {
		LOG("npambulant: NPP_NewStream called twice\n");
		return rv;
	}
	if (!pPlugin->init()) {
		rv = NPERR_GENERIC_ERROR;
	}
	return rv;
}

int32_t NPP_WriteReady (NPP instance, NPStream *stream)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	int32 rv = 0x0fffffff;
	return rv;
}

int32_t NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	int32 rv = len;
	return rv;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
	return rv;
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
	LOG("called.");
	if (instance == NULL) {
		return;
	}
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
	LOG("called.");
	if (instance == NULL) {
		return;
	}
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
	LOG("called.");
	if (instance == NULL) {
		return;
	}
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
	LOG("called.");
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	NPError rv = NPERR_NO_ERROR;
	return rv;
}

int16	NPP_HandleEvent(NPP instance, void* event)
{
	LOG("called.");
	if (instance == NULL) {
		return 0;
	}
	int16 rv = 0;
	npambulant * pPlugin = (npambulant *)instance->pdata;
	if (pPlugin) {
		rv = pPlugin->handleEvent(event);
	}
	return rv;
}

#ifdef OJI
jref NPP_GetJavaClass (void)
{
	LOG("called.");
	return NULL;
}
#endif//OJI

NPObject *NPP_GetScriptableInstance(NPP instance)
{
	LOG("called.");
	if (!instance) {
		return 0;
	}
	NPObject *npobj = 0;
	npambulant * pPlugin = (npambulant *)instance->pdata;
	if (!pPlugin) {
		npobj = pPlugin->GetScriptableObject();
	}
	return npobj;
}
