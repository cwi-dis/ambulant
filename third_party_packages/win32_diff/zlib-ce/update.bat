@echo off
REM $Id$

SETLOCAL

REM Download zlibce from http://www.ciprian-miclaus.com/sources/zlibce.asp
REM Version ?
REM Unzip to zlibce directory. 
REM The directory zlibce should be under the same directory as lib png

REM For example:
REM third_party_packages
REM 	lpng125
REM 	zlib
REM    	zlibce

REM Run this script to update the png library

REM The directory where the libpng lib was decompressed
set PNG_HOME=..\..\lpng125

REM Create the required directories
if not exist %PNG_HOME%\projects\wince md %PNG_HOME%\projects\wince
copy .\wince\*.* %PNG_HOME%\projects\wince

ENDLOCAL
@echo on


