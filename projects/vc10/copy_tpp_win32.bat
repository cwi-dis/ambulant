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
copy %expat_dir%\lib\Release\libexpat.lib %AMB_HOME%\lib\win32\libexpat.lib

REM PNG DLL
copy %lpng_dir%\projects\visualc71\Win32_LIB_ASM_Release\libpng.lib %AMB_HOME%\lib\win32\libpng.lib

REM ZLib DLL
copy %lpng_dir%\projects\visualc71\Win32_LIB_Release\zlib\zlib.lib %AMB_HOME%\lib\win32\zlib.lib

REM JPEG static library
copy %jpeg_dir%\win32\Release\libjpeg.lib %AMB_HOME%\lib\win32\libjpeg.lib

REM Xerces lib, if it exists.
set XER_BUILD=%xerces_dir%\Build\Win32\VC10\Release
set XERD_BUILD=%xerces_dir%\Build\Win32\VC10\Debug
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_%xerces_major%.lib %AMB_HOME%\lib\win32\xerces-c_%xerces_major%.lib
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_%xerces_major%_%xerces_minor%.dll %AMB_HOME%\bin\win32\xerces-c_%xerces_major%_%xerces_minor%.dll
if exist %XERD_BUILD% copy %XERD_BUILD%\xerces-c_%xerces_major%D.lib %AMB_HOME%\lib\win32\xerces-c_%xerces_major%D.lib
if exist %XERD_BUILD% copy %XERD_BUILD%\xerces-c_%xerces_major%_%xerces_minor%D.dll %AMB_HOME%\bin\win32\xerces-c_%xerces_major%_%xerces_minor%D.dll

REM ffmpeg
copy %ffmpeg_bin_dir%\bin\avcodec-55.dll %AMB_HOME%\bin\win32\
copy %ffmpeg_bin_dir%\bin\avformat-55.dll %AMB_HOME%\bin\win32\
copy %ffmpeg_bin_dir%\bin\avutil-52.dll %AMB_HOME%\bin\win32\
copy %ffmpeg_bin_dir%\bin\swscale-2.dll %AMB_HOME%\bin\win32\swscale-2.dll
copy %ffmpeg_bin_dir%\bin\swresample-0.dll %AMB_HOME%\bin\win32\swresample-0.dll
copy %ffmpeg_dev_dir%\lib\avcodec.lib %AMB_HOME%\lib\win32\avcodec.lib
copy %ffmpeg_dev_dir%\lib\avformat.lib %AMB_HOME%\lib\win32\avformat.lib
copy %ffmpeg_dev_dir%\lib\avutil.lib %AMB_HOME%\lib\win32\avutil.lib
copy %ffmpeg_dev_dir%\lib\swscale.lib %AMB_HOME%\lib\win32\swscale.lib
copy %ffmpeg_dev_dir%\lib\swresample.lib %AMB_HOME%\lib\win32\swresample.lib

REM sdl
if exist %sdl_dir%\lib copy %sdl_dir%\lib\SDL.dll %AMB_HOME%\bin\win32\SDL.dll
if exist %sdl_dir%\lib copy %sdl_dir%\lib\SDL.lib %AMB_HOME%\lib\win32\SDL.lib
if exist %sdl_dir%\lib copy %sdl_dir%\lib\SDLmain.lib %AMB_HOME%\lib\win32\SDLmain.lib
if exist %sdl_dir%\VisualC\SDL\Release copy %sdl_dir%\VisualC\SDL\Release\SDL.dll %AMB_HOME%\bin\win32\SDL.dll
if exist %sdl_dir%\VisualC\SDL\Release copy %sdl_dir%\VisualC\SDL\Release\SDL.lib %AMB_HOME%\lib\win32\SDL.lib
if exist %sdl_dir%\VisualC\SDLmain\Release copy %sdl_dir%\VisualC\SDLmain\Release\SDLmain.lib %AMB_HOME%\lib\win32\SDLmain.lib
if exist %sdl_dir%\VisualC\SDL\Win32\Release copy %sdl_dir%\VisualC\SDL\Win32\Release\SDL.dll %AMB_HOME%\bin\win32\SDL.dll
if exist %sdl_dir%\VisualC\SDL\Win32\Release copy %sdl_dir%\VisualC\SDL\Win32\Release\SDL.lib %AMB_HOME%\lib\win32\SDL.lib
if exist %sdl_dir%\VisualC\SDLmain\Win32\Release copy %sdl_dir%\VisualC\SDLmain\Win32\Release\SDLmain.lib %AMB_HOME%\lib\win32\SDLmain.lib

REM libxml2
copy %libxml2_dir%\win32\bin.msvc\libxml2_a.lib %AMB_HOME%\lib\win32\libxml2_a.lib

REM libdispatch
if exist %libdispatch_dir%\bin\win32\StaticRelease\libdispatch.lib copy %libdispatch_dir%\bin\win32\StaticRelease\libdispatch.lib %AMB_HOME%\lib\win32\libdispatch.lib
if exist %libdispatch_dir%\bin\win32\StaticDebug\libdispatch.lib copy %libdispatch_dir%\bin\win32\StaticDebug\libdispatch.lib %AMB_HOME%\lib\win32\libdispatchD.lib
@echo on