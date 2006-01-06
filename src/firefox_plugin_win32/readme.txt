This directory contains the source for creating a scriptable plugin for
Firefox on Windows XP (and may be some other variants) that plays .smil
files using AmbulantPlayer (currently local files only).

It is a separate Visual C++ Project.

Prequisites.
------------

1. AmbulantPlayer for Windows XP must have been compiled in Visual C++
   with Solution Configuration "DebugNoXercesNoHtml" or
   "ReleaseNoXercesNoHtml" and installed in the usual places
   (..\..\bin\win32 and ..\..\lib\win32)

2. The original GeckoPluginSDK from www.mozilla.org/projects/plugins
   must have been downloaded and built in ..\..\third_party_packages.
   If during the build you get errors during the execution of "xpidl"
   (like "libIDL-0.6.dll cannot be found"), the necessrary libraries
   are available in:
   ..\..\third_party_packages\GeckoPluginSDK-samples\Win32SDK\moztools\wintools\buildtools\windows\bin\x86
   Best copy these dll's to the directory where "xpidl" is stored.

3. Now this project can be built. It will install 2 files in:
   C:\Program File\Mozilla Firefox\plugins: npambulant.dll and
   nsIAmbulantPlugin.xpt
   Then start Firefox and File->Open File WelcomeButtons.html
   This .html file works if you have the Welcome document stored
   in C:\Program Files\AmbulantPlayer-1.6\Extras\Welcome.

4. To create a distribution:
   - Build the plugin in release mode
   - Create a folder ambulant-16-firefox-win32
   - Copy \Program Files\Mozilla Firefox\plugins\npambulant.dll there.
   - Copy \Program Files\Mozilla Firefox\plugins\nsIAmbulantPlugin.xpt there.
   - Copy INSTALL.txt there.
   - Copy toplevel COPYING there.
   - Zip the ambulant-16-firefox-win32 folder and distribute.



   Kees Blom, Dec. 21th, 2005.
