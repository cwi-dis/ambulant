rem 
rem  Script to do a nightly clean build of a full Ambulant
rem  Windows version
rem 

rem
rem NOTES ON THINGS THIS SCRIPT NEEDS, BUT WHICH ARE NOT CHECKED
rem
rem Add the following to your mercurial.ini file:
rem [ui]
rem username = Jack Jansen <Jack.Jansen@cwi.nl>
rem ssh = "TortoisePlink.exe" -ssh -2 -i "C:\Users\Jack\Documents\Putty Keys\id_dsa.ppk"
rem
rem Make sure your data format (Region and Language control panel) is M/d/yyyy
rem
rem Cabarc should live in a folder bin\Cabarc\bin\Cabarc.exe from where this script runs

set

rem
rem Location of various programs
rem

set hg="C:\Program Files\TortoiseHg\hg.exe"
set pageant="C:\Program Files\PuTTY\pageant.exe"

rem ALTERNATIVE set visualstudio="C:\Program Files\Microsoft Visual Studio 9.0"
rem ALTERNATIVE set vcdir="vc9"
rem ALTERNATIVE set VS100COMNTOOLS=

set visualstudio="C:\Program Files\Microsoft Visual Studio 10.0"
set vcdir="vc10"
set VS90COMNTOOLS=

set KEYFILE="%USERPROFILE%\Documents\Putty Keys\id_dsa.ppk"
set pscp="c:\Program Files\Putty\pscp.exe" -i %KEYFILE%
set nsis="c:\Program Files\NSIS\makensis.exe"
set python="c:\python27\python.exe"

rem
rem Getting the date in a reasonable format depends on the machine:
rem

rem XP US/UK: set TODAY=%date:~-4%%date:~4,2%%date:~7,2%
set TODAY=%date:~-4%%date:~-10,2%%date:~-7,2%

rem
rem Other settable parameters
rem

set AMBULANTVERSION=2.7
set BRANCH=default

echo LogLocation=win32-%TODAY%-%BRANCH%.txt

rem
rem Set VERSIONSUFFIX to empty for distribution builds
rem

set VERSIONSUFFIX=.%TODAY%
rem For the non-standard build, add vs2008 or vs2010
rem set VERSIONSUFFIX=.%TODAY%-vs2010

set HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
set KEYFILE="%USERPROFILE%\Documents\Putty Keys\id_dsa.ppk"
set HGCLONEPRIVARGS="ssh://hg@ambulantplayer.org/hgpriv/ambulant-private"
set CHECKOUTPRIVARGS=-P
set BUILDHOME="%USERPROFILE%\Documents\AmbulantNightly"
set BUILDDIR=build-%TODAY%-%BRANCH%
set BUILD3PPARGS=win32
set DESTINATION="sen5@ambulantplayer.org:/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds/%BRANCH%"
set DESTINATIONDESKTOP="%DESTINATION%/win32-intel-desktop/"
set DESTINATIONNP="%DESTINATION%/win32-intel-firefoxplugin/"
set DESTINATIONIE="%DESTINATION%/win32-intel-ieplugin/"
set DESTINATIONIEURL="http://www.ambulantplayer.org/nightlybuilds/%BRANCH%/win32-intel-ieplugin"

rem
rem Setup variables
rem

call %visualstudio%\VC\bin\vcvars32.bat

rem 
rem Check out a fresh copy of Ambulant after possibly cleaning out
rem old build dir (twice: this is windows, after all:-).
rem 

mkdir %buildhome%
cd /d %buildhome%
if exist %builddir% rmdir /s /q %builddir%
if exist %builddir% rmdir /s /q %builddir%
%hg% clone %HGCLONEARGS% %builddir%
if exist ambulant-private rmdir /s /q ambulant-private
%hg% clone %HGCLONEPRIVARGS%
rem XXXX %cvs% %CVSARGS% checkout %CHECKOUTARGS% -d %builddir% ambulant
rem XXXX %cvs% %CVSPRIVARGS% checkout %CHECKOUTPRIVARGS% ambulant-private
if %errorlevel% neq 0 goto errorexit

rem 
rem  Prepare the tree
rem 

cd %builddir%
hg up -r %BRANCH%
cd third_party_packages
%python% ..\scripts\build-third-party-packages.py %BUILD3PPARGS%
if %errorlevel% neq 0 goto errorexit
cd ..

rem 
rem  configure, make, make install
rem

cd projects\%vcdir%
devenv third_party_packages.sln /build Release
if %errorlevel% gtr 0 goto errorexit
devenv Ambulant-win32.sln /build Release
if %errorlevel% gtr 0 goto errorexit

rem
rem Upload IE, Netscape plugins
rem

cd ..\..\bin\win32
if not exist npambulant-%AMBULANTVERSION%-win32.xpi goto skipnpambulant
rename npambulant-%AMBULANTVERSION%-win32.xpi npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi
%pscp% -i %KEYFILE% npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi %DESTINATIONNP%
if %errorlevel% neq 0 goto errorexit
:skipnpambulant
if not exist ieambulant-%AMBULANTVERSION%-win32.cab goto skipieambulant
rename ieambulant-%AMBULANTVERSION%-win32.cab ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab
%pscp% -i %KEYFILE% ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab %DESTINATIONIE%
%python% ..\..\scripts\geniepluginwebpage.py ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32 %DESTINATIONIEURL%/ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab > ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.html
%pscp% -i %KEYFILE% ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.html %DESTINATIONIE%

if %errorlevel% neq 0 goto errorexit
:skipieambulant

rem 
rem  Create desktop installer, upload
rem 

cd ..\..\installers\nsis-win32
%nsis% setup-ambulant-installer.nsi
if %errorlevel% neq 0 goto errorexit
rename  Ambulant-%AMBULANTVERSION%-win32.exe Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe
%pscp% -i %KEYFILE% Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe %DESTINATIONDESKTOP%
if %errorlevel% neq 0 goto errorexit

:errorexit
exit

