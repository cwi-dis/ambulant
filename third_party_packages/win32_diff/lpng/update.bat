@echo off
REM $Id$
SETLOCAL


REM Download libpng from http://sourceforge.net/projects/libpng/
REM Version 1.2.5

REM Download zlib from http://sourceforge.net/projects/libpng/
REM Version 1.2.1

REM Decompress both under the same directory
REM For example:
REM third_party_packages
REM 	lpng125
REM 	zlib

REM Run this script to update the distribution

REM The directory where the libpng lib was decompressed
set LPNG_HOME=..\..\lpng125

REM Copy project files
copy libpng.* %LPNG_HOME%\projects\msvc
copy zlib.* %LPNG_HOME%\projects\msvc

ENDLOCAL
@echo on