:: To create and codesign ieambulant.cab, first build Ambulant-win32.sln
:: then ieambulant. CD to "Release" directory, and run this script:
:: ../makeCWIcab.cmd
::
:: This script works with a fake code signing certifiacte for testing.
:: It will pnly work in Internet Explorer when the server were you host
:: the resulting cabinet (.cab) file is added to "Trusted sites" and security
:: level for "Tusted sites" is set to "Low".
:: For a useable plugin, change the file name of the "certificate.pfx"
:: -----------------------------------------------------------------------
:: copy all files for the cab into this directory
@echo "copying all files for the cab into this directory"
::
:: redistributable C-runtime 
copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest" Microsoft.VC80.CRT.manifest
copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll" msvcr80.dll 
copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll" msvcp80.dll
:: redistributable Microsoft Foundation classes
copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.MFC\Microsoft.VC80.MFC.manifest" Microsoft.VC80.MFC.manifest
copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.MFC\mfc80.dll" mfc80.dll
mt.exe -manifest Microsoft.VC80.CRT.manifest -outputresource:msvcr80.dll;2
:: Ambulant dll's
copy ..\..\..\bin\win32\libambulant_shwin32.dll libambulant_shwin32.dll
copy ..\..\..\bin\win32\libamplugin_ffmpeg.dll libamplugin_ffmpeg.dll 
copy ..\..\..\bin\win32\libamplugin_plugin.dll libamplugin_plugin.dll
copy ..\..\..\bin\win32\libamplugin_state_xpath.dll libamplugin_state_xpath.dll
copy ..\..\..\bin\win32\avcodec-51.dll avcodec-51.dll
copy ..\..\..\bin\win32\avformat-52.dll avformat-52.dll 
copy ..\..\..\bin\win32\avutil-49.dll avutil-49.dll
copy ..\..\..\bin\win32\SDL.dll SDL.dll
copy ..\..\..\bin\win32\xerces-c_2_8.dll xerces-c_2_8.dll
copy ..\AmbulantActiveX.inf AmbulantActiveX.inf
:: Create a new cabinet (.cab) archive
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\CabArc.Exe" -s 6144 n ieambulant.cab Microsoft.VC80.MFC.manifest Microsoft.VC80.CRT.manifest mfc80.dll msvcr80.dll msvcp80.dll AmbulantActiveX.dll libambulant_shwin32.dll libamplugin_ffmpeg.dll libamplugin_plugin.dll libamplugin_state_xpath.dll avcodec-51.dll avformat-52.dll avutil-49.dll SDL.dll xerces-c_2_8.dll AmbulantActiveX.inf
:: Code sign it with code signing certificate (.pfx = Personal Information Exchange) 
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" sign /f ..\certificate.pfx /p ambulant /v ieambulant.cab
:: timestamp the signature
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" timestamp  /v /t "http://timestamp.verisign.com/scripts/timstamp.dll" ieambulant.cab
:: verify the resulting cabinet (.cab) archive
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" verify /v /a /pa ieambulant.cab
@echo ------------------------------------------------------------------------------
@echo When the code was signed with the default (test) certificate only, you'll see:
@echo SignTool Error: File not valid: ieambulant.cab
@echo ------------------------------------------------------------------------------
:: erase files copied
@erase Microsoft.VC80.MFC.manifest Microsoft.VC80.CRT.manifest mfc80.dll msvcr80.dll msvcp80.dll libambulant_shwin32.dll libamplugin_ffmpeg.dll libamplugin_plugin.dll libamplugin_state_xpath.dll avcodec-51.dll avformat-52.dll avutil-49.dll SDL.dll xerces-c_2_8.dll AmbulantActiveX.inf