@echo off

REM Ambulant home directory
set AMB_HOME=..

REM Third Party Packages home directory
set TPP_HOME=.

set WCE_ARM_LIB=%AMB_HOME%\lib\wince-arm

REM Expat static library
copy %TPP_HOME%\expat\lib\ARMRel\libexpat.lib %WCE_ARM_LIB%\libexpat.lib

REM PNG static library
copy %TPP_HOME%\lpng125\projects\wince\ARMRel\libpng.lib %WCE_ARM_LIB%\libpng.lib

REM ZLib static library
copy %TPP_HOME%\zlibce\ARMRel\zlibce.lib %WCE_ARM_LIB%\zlibce.lib

REM JPEG static library
copy %TPP_HOME%\jpeg\wince\ARMRel\libjpeg.lib %WCE_ARM_LIB%\libjpeg.lib

REM MP3LIB static library
copy %TPP_HOME%\mp3lib\ARMRel\mp3lib.lib %WCE_ARM_LIB%\mp3lib.lib

@echo on