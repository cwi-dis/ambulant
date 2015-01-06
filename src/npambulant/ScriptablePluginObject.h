/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SCRIPTABLEPLUGINOBJECT_H__
#define __SCRIPTABLEPLUGINOBJECT_H__
/*AMBULANT_FOREIGN_INDENT_RULES*/

#include "ScriptablePluginObjectBase.h"
class ScriptablePluginObject : public ScriptablePluginObjectBase
{
public:
  ScriptablePluginObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
  {
  }

  virtual bool HasMethod(NPIdentifier name);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
};

extern NPIdentifier sStartPlayer_id;
extern NPIdentifier sStopPlayer_id;
extern NPIdentifier sRestartPlayer_id;
extern NPIdentifier sPausePlayer_id;
extern NPIdentifier sResumePlayer_id;
extern NPIdentifier sIsDone_id;

extern NPIdentifier sPluginType_id;
extern NPIdentifier sDocument_id;
extern NPIdentifier sCreateElement_id;
extern NPIdentifier sCreateTextNode_id;
extern NPIdentifier sAppendChild_id;
extern NPIdentifier sBody_id;
extern NPObject* sWindowObj;
#endif //  __SCRIPTABLEPLUGINOBJECT_H_

