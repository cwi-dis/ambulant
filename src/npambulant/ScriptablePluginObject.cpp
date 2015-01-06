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

/*AMBULANT_FOREIGN_INDENT_RULES*/
#ifdef	XP_WIN32
#include <cstddef>		   	 // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
#ifdef _DEBUG
#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#endif//_DEBUG
#endif//XP_WIN32

#include "ScriptablePluginObject.h"
#include "ConstructablePluginObject.h"
#include "npambulant.h"

static NPObject *
AllocateConstructablePluginObject(NPP npp, NPClass *aClass)
{
  return new ConstructablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ConstructablePluginObject,
                                 AllocateConstructablePluginObject);

bool
ConstructablePluginObject::Construct(const NPVariant *args, uint32_t argCount,
                                     NPVariant *result)
{
  printf("Creating new ConstructablePluginObject!\n");

  NPObject *myobj =
    NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
  if (!myobj)
    return false;

  OBJECT_TO_NPVARIANT(myobj, *result);

  return true;
}
NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
  return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);

bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
  return false
    || name == sStartPlayer_id
	|| name == sStopPlayer_id
	|| name == sRestartPlayer_id
	|| name == sPausePlayer_id
	|| name == sResumePlayer_id
	|| name == sIsDone_id
	;
}

bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
  return (name == sPluginType_id);
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
  VOID_TO_NPVARIANT(*result);

  if (name == sPluginType_id) {
    NPObject *myobj =
      NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
    if (!myobj) {
      return false;
    }

    OBJECT_TO_NPVARIANT(myobj, *result);

    return true;
  }

  return true;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
  if ( ! mNpp)
	return FALSE;
  if (name == sStartPlayer_id) {
	printf ("startPlayer called !\n");
	((npambulant*)mNpp->pdata)->startPlayer();
  } else  if (name == sStopPlayer_id) {
	printf ("stopPlayer called !\n");
	((npambulant*)mNpp->pdata)->stopPlayer();
  } else  if (name == sRestartPlayer_id) {
	printf ("restartPlayer called !\n");
	((npambulant*)mNpp->pdata)->restartPlayer();
  } else  if (name == sPausePlayer_id) {
	printf ("pausePlayer called !\n");
	((npambulant*)mNpp->pdata)->pausePlayer();
  } else  if (name == sResumePlayer_id) {
	printf ("resumePlayer called !\n");
	((npambulant*)mNpp->pdata)->resumePlayer();
  } else  if (name == sIsDone_id) {
	printf ("isDone called !\n");
	bool rv = ((npambulant*)mNpp->pdata)->isDone();
	BOOLEAN_TO_NPVARIANT(rv, *result);
  } else return FALSE;
  return TRUE;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
  printf ("ScriptablePluginObject default method called!\n");

  STRINGZ_TO_NPVARIANT(strdup("default method return val"), *result);

  return TRUE;
}
