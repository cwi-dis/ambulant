@echo on
REM $Id$

REM Ambulant home directory
set AMB_HOME=..\..

REM Third Party Packages home directory
set TPP_HOME=%AMB_HOME%\third_party_packages

REM Create required directories
if not exist %AMB_HOME%\lib md %AMB_HOME%\lib
if not exist %AMB_HOME%\lib\win32 md %AMB_HOME%\lib\win32
if not exist %AMB_HOME%\bin md %AMB_HOME%\bin
if not exist %AMB_HOME%\bin\win32 md %AMB_HOME%\bin\win32

REM Expat
copy %TPP_HOME%\expat\lib\Release\libexpat.lib %AMB_HOME%\lib\win32\libexpat.lib

REM PNG DLL
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_ASM_Release\libpng.lib %AMB_HOME%\lib\win32\libpng.lib

REM ZLib DLL
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_Release\zlib\zlib.lib %AMB_HOME%\lib\win32\zlib.lib

REM JPEG static library
copy %TPP_HOME%\jpeg\win32\Release\libjpeg.lib %AMB_HOME%\lib\win32\libjpeg.lib

REM Xerces lib, if it exists.
set XER_BUILD=%TPP_HOME%\xerces-c-src_2_8_0\Build\Win32\VC8\Release
set XERD_BUILD=%TPP_HOME%\xerces-c-src_2_8_0\Build\Win32\VC8\Debug
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2.lib %AMB_HOME%\lib\win32\xerces-c_2.lib
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2_8.dll %AMB_HOME%\bin\win32\xerces-c_2_8.dll
if exist %XER_BUILD% copy %XERD_BUILD%\xerces-c_2D.lib %AMB_HOME%\lib\win32\xerces-c_2D.lib
if exist %XER_BUILD% copy %XERD_BUILD%\xerces-c_2_8D.dll %AMB_HOME%\bin\win32\xerces-c_2_8D.dll

REM ffmpeg
copy %TPP_HOME%\ffmpeg\libavcodec\avcodec-51.dll %AMB_HOME%\bin\win32\avcodec-51.dll
copy %TPP_HOME%\ffmpeg\libavcodec\avcodec-51.lib %AMB_HOME%\lib\win32\avcodec.lib
copy %TPP_HOME%\ffmpeg\libavformat\avformat-52.dll %AMB_HOME%\bin\win32\avformat-52.dll
copy %TPP_HOME%\ffmpeg\libavformat\avformat-52.lib %AMB_HOME%\lib\win32\avformat.lib
copy %TPP_HOME%\ffmpeg\libavutil\avutil-49.dll %AMB_HOME%\bin\win32\avutil-49.dll
copy %TPP_HOME%\ffmpeg\libavutil\avutil-49.lib %AMB_HOME%\lib\win32\avutil.lib

REM sdl
copy %TPP_HOME%\SDL-1.2.12\lib\SDL.dll %AMB_HOME%\bin\win32\SDL.dll
copy %TPP_HOME%\SDL-1.2.12\lib\SDL.lib %AMB_HOME%\lib\win32\SDL.lib
copy %TPP_HOME%\SDL-1.2.12\lib\SDLmain.lib %AMB_HOME%\lib\win32\SDLmain.lib

REM Live555
copy %TPP_HOME%\live_VC9\BUILD\BasicUsageEnvironment-Release\BasicUsageEnvironment.lib %AMB_HOME%\lib\win32\BasicUsageEnvironment.lib
copy %TPP_HOME%\live_VC9\BUILD\groupsock-Release\groupsock.lib %AMB_HOME%\lib\win32\groupsock.lib
copy %TPP_HOME%\live_VC9\BUILD\liveMedia-Release\liveMedia.lib %AMB_HOME%\lib\win32\liveMedia.lib
copy %TPP_HOME%\live_VC9\BUILD\UsageEnvironment-Release\UsageEnvironment.lib %AMB_HOME%\lib\win32\UsageEnvironment.lib

REM libxml2
copy %TPP_HOME%\libxml2-2.7.3\win32\bin.msvc\libxml2_a.lib %AMB_HOME%\lib\win32\libxml2_a.lib
@echo on