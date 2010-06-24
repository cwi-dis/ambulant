rem 
rem  Script to do a nightly clean build of a full Ambulant
rem  Windows version
rem 

rem
rem Location of various programs
rem

set CVSKEY="%USERPROFILE%\My Documents\Putty Keys\id_dsa.ppk"
set CVS_RSH="C:\Program Files\TortoiseCVS\TortoisePlink.exe" -i %CVSKEY%
set cvs="C:\Program Files\TortoiseCVS\cvs.exe"
set visualstudio="C:\Program Files\Microsoft Visual Studio 9.0"
set pscp="c:\Program Files\Putty\pscp.exe"
set nsis="c:\Program Files\NSIS\makensis.exe"

rem
rem Other settable parameters
rem

set AMBULANTVERSION=2.3
set CVSUSER=jackjansen
set CVSARGS=-d "%CVSUSER%@ambulant.cvs.sourceforge.net:/cvsroot/ambulant"
set CHECKOUTARGS=
set CVSPRIVUSER=jack
set CVSPRIVARGS=-d "%CVSPRIVUSER%@oratrix.oratrix.com:/ufs/jack/.CVSROOT"
set CHECKOUTPRIVARGS=
set BUILDHOME="%TEMP%\ambulant-nightly"
rem US/UK: set TODAY=%date:~-4%%date:~4,2%%date:~7,2%
set TODAY=%date:~-4%%date:~-7,2%%date:~-10,2%
set VERSIONSUFFIX=.%TODAY%
set BUILDDIR=ambulant-build-%TODAY%
set BUILD3PPARGS=win32
set DESTINATION="jack@ssh.cwi.nl:public_html/ambulant/"

rem
rem Setup variables
rem

call %visualstudio%\VC\bin\vcvars32.bat

rem 
rem  Check out a fresh copy of Ambulant
rem 

mkdir %buildhome%
cd /d %buildhome%
%cvs% %CVSARGS% checkout %CHECKOUTARGS% -d %builddir% ambulant
rem XXXX %cvs% %CVSPRIVARGS% checkout %CHECKOUTPRIVARGS% ambulant-private
if %errorlevel% neq 0 exit

rem 
rem  Prepare the tree
rem 

cd %builddir%\third_party_packages
echo XXXX python build-third-party-packages.py $BUILD3PPARGS
cd ..

rem 
rem  configure, make, make install
rem

cd projects\vc9
devenv third_party_packages.sln /build Release
if %errorlevel% neq 0 exit
devenv Ambulant-win32.sln /build ReleaseShlib
if %errorlevel% neq 0 exit

rem
rem Upload IE, Netscape plugins
rem

cd ..\..\bin\win32
rename npambulant-%AMBULANTVERSION%-win32.xpi npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi
rename ieambulant-%AMBULANTVERSION%-win32.cab ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab
%pscp% -i %KEYFILE% npambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.xpi %DESTINATION%
if %errorlevel% neq 0 exit
%pscp% -i %KEYFILE% ieambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.cab %DESTINATION%
if %errorlevel% neq 0 exit

rem 
rem  Create desktop installer, upload
rem 

cd ..\..\installers\nsis-win32
%nsis% setup-ambulant-installer.nsi
if %errorlevel% neq 0 exit
rename  Ambulant-%AMBULANTVERSION%-win32.exe Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe
%pscp% -i %KEYFILE% Ambulant-%AMBULANTVERSION%%VERSIONSUFFIX%-win32.exe %DESTINATION%
if %errorlevel% neq 0 exit

rem 
rem  Delete old installers, remember current
rem 
rem  XXX TODO
