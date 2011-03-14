rem 
rem  Script to do a nightly clean build of a full Ambulant
rem  Windows version
rem 

rem
rem Location of various programs
rem

set hg="C:\Program Files\TortoiseHg\hg.exe"
set pageant="C:\Program Files\PuTTY\pageant.exe"
set visualstudio="C:\Program Files\Microsoft Visual Studio 9.0"
set vcdir="vc9"
rem ALTERNATIVE set visualstudio="C:\Program Files\Microsoft Visual Studio 10.0"
rem ALTERNATIVE set vcdir="vc10"
set pscp="c:\Program Files\Putty\pscp.exe"
set nsis="c:\Program Files\NSIS\makensis.exe"
set python="c:\python26\python.exe"

rem
rem Other settable parameters
rem

set AMBULANTVERSION=2.3
set HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
set KEYFILE="%USERPROFILE%\My Documents\Putty Keys\id_dsa.ppk"
set HGCLONEPRIVARGS="ssh://hg@ambulantplayer.org/hgpriv/ambulant-private"
set CHECKOUTPRIVARGS=-P
set BUILDHOME="%USERPROFILE%\My Documents\AmbulantNightly"
rem XP US/UK: set TODAY=%date:~-4%%date:~4,2%%date:~7,2%
rem XP NL: set TODAY=%date:~-4%%date:~-7,2%%date:~-10,2%
set TODAY=%date%
set VERSIONSUFFIX=.%TODAY%
set BUILDDIR=build-%TODAY%
set BUILD3PPARGS=win32
set DESTINATION="jack@ssh.cwi.nl:public_html/ambulant/nightly/"
set DESTINATIONDESKTOP="%DESTINATION%/win32-desktop/"
set DESTINATIONNP="%DESTINATION%/win32-firefoxplugin/"
set DESTINATIONIE="%DESTINATION%/win32-ieplugin/"

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
if %errorlevel% neq 0 pause

rem 
rem  Prepare the tree
rem 

cd %builddir%\third_party_packages
%python% build-third-party-packages.py %BUILD3PPARGS%
if %errorlevel% neq 0 pause
cd ..

rem 
rem  configure, make, make install
rem

cd projects\%vcdir%
devenv third_party_packages.sln /build Release
if %errorlevel% neq 0 pause
devenv Ambulant-win32.sln /build Release
if %errorlevel% neq 0 pause

rem
rem Upload IE, Netscape plugins
rem

cd ..\..\bin\win32
if not exist npambulant-%AMBULANTVERSION%-win32.xpi goto skipnpambulant
rename npambulant-%AMBULANTVERSION%-win32.xpi npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi
%pscp% -i %KEYFILE% npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi %DESTINATIONNP%
if %errorlevel% neq 0 pause
:skipnpambulant
if not exist ieambulant-%AMBULANTVERSION%-win32.cab goto skipieambulant
rename ieambulant-%AMBULANTVERSION%-win32.cab ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab
%pscp% -i %KEYFILE% ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab %DESTINATIONIE%
if %errorlevel% neq 0 pause
:skipieambulant

rem 
rem  Create desktop installer, upload
rem 

cd ..\..\installers\nsis-win32
%nsis% setup-ambulant-installer.nsi
if %errorlevel% neq 0 pause
rename  Ambulant-%AMBULANTVERSION%-win32.exe Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe
%pscp% -i %KEYFILE% Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe %DESTINATIONDESKTOP%
if %errorlevel% neq 0 pause

rem 
rem  Delete old installers, remember current
rem 
rem  XXX TODO
