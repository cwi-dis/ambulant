rem 
rem  Script to do a nightly clean build of a full Ambulant, indirectly
rem  Windows version
rem 
rem Run this script automatically by having the user auto-login, and add this
rem script to to the "Task Scheduler" for the "logon" event (with elevated permissions).
rem See http://www.techrepublic.com/blog/window-on-windows/make-vista-launch-uac-restricted-programs-at-startup-with-task-scheduler/616
rem
rem It will not work correctly as gpedit.msc login script, for some reason.

set python="c:\python27\python.exe"

cd \users\jack\documents
set
%python% -c "import urllib; urllib.urlretrieve('http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild/nightlybuild.win32.bat', 'nightlybuild.win32.bat')"

echo hello world

rem Remove old build stuff

if exist AmbulantNightly rmdir /s /q AmbulantNightly
if exist AmbulantNightly rmdir /s /q AmbulantNightly

rem Build. This will pause if unsuccessful.

cmd/k nightlybuild.win32.bat >nightlybuild.out.txt 2>&1
if %errorlevel% neq 0 pause

rem Copy the output file back to the mac

copy nightlybuild.out.txt \\psf\Home\tmp\ambulant-nightly\nightlybuild.win32.out.txt
 %errorlevel% neq 0 pause

rem Shutdown
shutdown /s

pause
