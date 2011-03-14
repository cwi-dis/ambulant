rem 
rem  Script to do a nightly clean build of a full Ambulant, indirectly
rem  Windows version
rem 

set python="c:\python26\python.exe"
cd %TEMP%
%python% -c "import urllib; urllib.urlretrieve('http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild.win32.bat', 'nightlybuild.win32.bat')"
call nightlybuild.win32.bat
