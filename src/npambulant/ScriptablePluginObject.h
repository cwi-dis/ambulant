#ifndef __SCRIPTABLEPLUGINOBJECT_H__
#define __SCRIPTABLEPLUGINOBJECT_H__

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

