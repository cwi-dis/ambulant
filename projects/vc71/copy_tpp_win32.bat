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
REM skip: copy %TPP_HOME%\expat\lib\Release\libexpat.dll %AMB_HOME%\bin\win32\libexpat.dll
copy %TPP_HOME%\expat\lib\Release\libexpat.lib %AMB_HOME%\lib\win32\libexpat.lib

REM PNG DLL
REM skip: copy %TPP_HOME%\lpng128\projects\visualc71\XXXXXX\libpng.dll %AMB_HOME%\bin\win32\libpng.dll
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_ASM_Release\libpng.lib %AMB_HOME%\lib\win32\libpng.lib

REM ZLib DLL
REM skip: copy %TPP_HOME%\lpng128\projects\visualc71\XXXXXX\zlib\zlib.dll %AMB_HOME%\bin\win32\zlib.dll
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_ASM_Release\zlib\zlib.lib %AMB_HOME%\lib\win32\zlib.lib

REM JPEG static library
copy %TPP_HOME%\jpeg\win32\Release\libjpeg.lib %AMB_HOME%\lib\win32\libjpeg.lib

REM Xerces lib, if it exists
set XER_BUILD=%TPP_HOME%\xerces-c-src_2_7_0\Build\Win32\VC7.1\Release
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2.lib %AMB_HOME%\lib\win32\xerces-c_2.lib
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2_7.dll %AMB_HOME%\bin\win32\xerces-c_2_7.dll
set XER_BUILD=%TPP_HOME%\xerces-c-src_2_7_0\Build\Win32\VC7.1\Debug
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2D.lib %AMB_HOME%\lib\win32\xerces-c_2D.lib
if exist %XER_BUILD% copy %XER_BUILD%\xerces-c_2_7D.dll %AMB_HOME%\bin\win32\xerces-c_2_7D.dll
@echo on