@echo off
REM $Id$
SETLOCAL

REM Download the JPEG lib from http://www.ijg.org/files/
REM Ambulant has been tested with jpeg-6b
REM Decompress the distribution and run this script to update the distribution

REM The directory where the jpeg lib was decompressed
set JPEG_HOME=..\..\jpeg-6b

REM The current directory
set UPD_HOME=%~dp0

REM Create the project directories
if not exist %JPEG_HOME%\win32 md %JPEG_HOME%\win32
if not exist %JPEG_HOME%\wince md %JPEG_HOME%\wince

REM Ambulant needs to update jinclude.h
cd %JPEG_HOME%
rename jinclude.h jinclude.h.org
cd %UPD_HOME%
copy %UPD_HOME%\jinclude.h %JPEG_HOME%\jinclude.h

REM Copy project files
copy %UPD_HOME%\win32\*.* %JPEG_HOME%\win32
copy %UPD_HOME%\wince\*.* %JPEG_HOME%\wince

ENDLOCAL
@echo on