This directory contains the source for creating a scriptable plugin for
Firefox on Windows XP (and may be some other variants) that plays .smil
files using AmbulantPlayer (currently local files only).

It is a separate Visual C++ Project.

Prequisites.
------------

1. AmbulantPlayer for Windows XP must have been compiled and installed
   in the usual places (..\..\bin\win32 and ..\..\lib\win32).
   During compilation, the following options need to be removed from
   Project->AmbulantPlayer Properties->C/C++->Preprocessor->Preprocessor Definitions
   - WITH_HTML_WIDGET
   and in libambulant_win32
   Project->Properties->C/C++->Preprocessor->Preprocessor Definitions
   - WITH_HTML_WIDGET
   - WITH_XERCES
   - WITH_XERCES_BUILIN

2. The original GeckoPluginSDK from www.mozilla.org/projects/plugins
   must have been downloaded and built in ..\..\third_party_packages.

3. Now this project can be built. It will install 2 files in:
   C:\Program File\Mozilla Firefox\plugins: npambulant.dll and
   nsIScriptablePluginSample.xpt
   Then start Firefox and File->Open File WelcomeButtons.html
   This .html file works if you have the Welcome document stored
   in C:\Program Files\AmbulantPlayer-1.4\Extras\Welcome.

   Kees Blom, Dec. 7th, 2005.
