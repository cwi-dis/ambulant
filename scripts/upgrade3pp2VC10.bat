rem
rem Upgrade projects from VC9 to VC10, if needed
rem
if not exist expat\lib\expat.vcxproj devenv expat\lib\expat.vcproj /Upgrade 
if not exist jpeg\win32\libjpeg.vcxproj devenv jpeg\win32\libjpeg.vcproj /Upgrade 
if not exist lpng128\projects\visualc71\libpng.vcxproj devenv lpng128\projects\visualc71\libpng.vcproj /Upgrade
if not exist lpng128\projects\visualc71\zlib.vcxproj devenv lpng128\projects\visualc71\zlib.vcproj /Upgrade
