/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM nsIAmbulantPlugin.idl
 */

#ifndef __gen_nsIAmbulantPlugin_h__
#define __gen_nsIAmbulantPlugin_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIAmbulantPlugin */
#define NS_IAMBULANTPLUGIN_IID_STR "968eee2e-5d15-4e04-b182-67bebec8d38d"

#define NS_IAMBULANTPLUGIN_IID \
  {0x968eee2e, 0x5d15, 0x4e04, \
    { 0xb1, 0x82, 0x67, 0xbe, 0xbe, 0xc8, 0xd3, 0x8d }}

class NS_NO_VTABLE nsIAmbulantPlugin : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IAMBULANTPLUGIN_IID)

  /**
   * The object to be wrapped and exposed to JavaScript.  It should
   * be an XPCOM object, and it can be the same object as the plugin.
  readonly attribute nsQIResult scriptablePeer;
   */
/**
   * The interface that XPConnect should use when exposing the peer
   * object to JavaScript.  All scriptable methods on the interface
   * will be available to JavaScript.
  readonly attribute nsIIDPtr scriptableInterface;
   */
  /* void startPlayer (); */
  NS_IMETHOD StartPlayer(void) = 0;

  /* void stopPlayer (); */
  NS_IMETHOD StopPlayer(void) = 0;

  /* void restartPlayer (); */
  NS_IMETHOD RestartPlayer(void) = 0;

  /* void resumePlayer (); */
  NS_IMETHOD ResumePlayer(void) = 0;

  /* void pausePlayer (); */
  NS_IMETHOD PausePlayer(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIAMBULANTPLUGIN \
  NS_IMETHOD StartPlayer(void); \
  NS_IMETHOD StopPlayer(void); \
  NS_IMETHOD RestartPlayer(void); \
  NS_IMETHOD ResumePlayer(void); \
  NS_IMETHOD PausePlayer(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIAMBULANTPLUGIN(_to) \
  NS_IMETHOD StartPlayer(void) { return _to StartPlayer(); } \
  NS_IMETHOD StopPlayer(void) { return _to StopPlayer(); } \
  NS_IMETHOD RestartPlayer(void) { return _to RestartPlayer(); } \
  NS_IMETHOD ResumePlayer(void) { return _to ResumePlayer(); } \
  NS_IMETHOD PausePlayer(void) { return _to PausePlayer(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIAMBULANTPLUGIN(_to) \
  NS_IMETHOD StartPlayer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StartPlayer(); } \
  NS_IMETHOD StopPlayer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->StopPlayer(); } \
  NS_IMETHOD RestartPlayer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->RestartPlayer(); } \
  NS_IMETHOD ResumePlayer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResumePlayer(); } \
  NS_IMETHOD PausePlayer(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PausePlayer(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAmbulantPlugin : public nsIAmbulantPlugin
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAMBULANTPLUGIN

  nsAmbulantPlugin();

private:
  ~nsAmbulantPlugin();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAmbulantPlugin, nsIAmbulantPlugin)

nsAmbulantPlugin::nsAmbulantPlugin()
{
  /* member initializers and constructor code */
}

nsAmbulantPlugin::~nsAmbulantPlugin()
{
  /* destructor code */
}

/* void startPlayer (); */
NS_IMETHODIMP nsAmbulantPlugin::StartPlayer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stopPlayer (); */
NS_IMETHODIMP nsAmbulantPlugin::StopPlayer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void restartPlayer (); */
NS_IMETHODIMP nsAmbulantPlugin::RestartPlayer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void resumePlayer (); */
NS_IMETHODIMP nsAmbulantPlugin::ResumePlayer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void pausePlayer (); */
NS_IMETHODIMP nsAmbulantPlugin::PausePlayer()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIAmbulantPlugin_h__ */
