@echo off

REM Ambulant home directory
set AMB_HOME=..

REM Third Party Packages home directory
set TPP_HOME=.

REM Expat DLL
copy %TPP_HOME%\expat\lib\Release\libexpat.dll %AMB_HOME%\bin\win32\libexpat.dll
copy %TPP_HOME%\expat\lib\Release\libexpat.lib %AMB_HOME%\lib\win32\libexpat.lib

REM PNG DLL
copy %TPP_HOME%\lpng125\projects\msvc\win32\libpng\dll\libpng13.dll %AMB_HOME%\bin\win32\libpng13.dll
copy %TPP_HOME%\lpng125\projects\msvc\win32\libpng\dll\libpng13.lib %AMB_HOME%\lib\win32\libpng13.lib

REM ZLib DLL
copy %TPP_HOME%\lpng125\projects\msvc\win32\zlib\dll\zlib.dll %AMB_HOME%\bin\win32\zlib.dll
copy %TPP_HOME%\lpng125\projects\msvc\win32\zlib\dll\zlib.dll %AMB_HOME%\lib\win32\zlib.lib

REM JPEG static library
copy %TPP_HOME%\jpeg\win32\Release\libjpeg.lib %AMB_HOME%\lib\win32\libjpeg.lib

@echo on