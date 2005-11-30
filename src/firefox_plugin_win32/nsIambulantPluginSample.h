/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:\Documents and Settings\kees\My Documents\My Work\ambulant\src\firefox_plugin_win32\nsIambulantPluginSample.idl
 */

#ifndef __gen_nsIambulantPluginSample_h__
#define __gen_nsIambulantPluginSample_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIambulantPluginSample */
#define NS_IAMBULANTPLUGINSAMPLE_IID_STR "d2d536a0-b6fc-11d5-9d10-0060b0fbd8ac"

#define NS_IAMBULANTPLUGINSAMPLE_IID \
  {0xd2d536a0, 0xb6fc, 0x11d5, \
    { 0x9d, 0x10, 0x00, 0x60, 0xb0, 0xfb, 0xd8, 0xac }}

class NS_NO_VTABLE nsIambulantPluginSample : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IAMBULANTPLUGINSAMPLE_IID)

  /* void showVersion (); */
  NS_IMETHOD ShowVersion(void) = 0;

  /* void clear (); */
  NS_IMETHOD Clear(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIAMBULANTPLUGINSAMPLE \
  NS_IMETHOD ShowVersion(void); \
  NS_IMETHOD Clear(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIAMBULANTPLUGINSAMPLE(_to) \
  NS_IMETHOD ShowVersion(void) { return _to ShowVersion(); } \
  NS_IMETHOD Clear(void) { return _to Clear(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIAMBULANTPLUGINSAMPLE(_to) \
  NS_IMETHOD ShowVersion(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShowVersion(); } \
  NS_IMETHOD Clear(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Clear(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsambulantPluginSample : public nsIambulantPluginSample
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAMBULANTPLUGINSAMPLE

  nsambulantPluginSample();
  virtual ~nsambulantPluginSample();
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsambulantPluginSample, nsIambulantPluginSample)

nsambulantPluginSample::nsambulantPluginSample()
{
  /* member initializers and constructor code */
}

nsambulantPluginSample::~nsambulantPluginSample()
{
  /* destructor code */
}

/* void showVersion (); */
NS_IMETHODIMP nsambulantPluginSample::ShowVersion()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clear (); */
NS_IMETHODIMP nsambulantPluginSample::Clear()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIambulantPluginSample_h__ */
