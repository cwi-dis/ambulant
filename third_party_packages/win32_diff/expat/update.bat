@echo off
SETLOCAL

REM Download expat from http://sourceforge.net/projects/expat/
REM Ambulant has been tested with version 1.95.7
REM Decompress the distribution and run this script to update the distribution

REM The directory where the expat lib was decompressed
set EXPAT_HOME=..\..\expat-1.95.7

REM Copy project files
copy libexpat.* %EXPAT_HOME%\lib
copy expat.* %EXPAT_HOME%\lib

ENDLOCAL
@echo on