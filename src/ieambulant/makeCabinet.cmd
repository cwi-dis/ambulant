:: This script needs both Visual Studio 2005 and Visual Studio 2009 installed.
:: To create and codesign ieambulant.cab, first build Ambulant-win32.sln,
:: then ieambulant.
:: Using CommandPrompt, CD to "Release" directory, and run this script:
:: ..\makeCabinet.cmd
::
:: This script works with a test code signing certificate.
:: This will only work in Internet Explorer when the server were you host
:: the resulting cabinet (.cab) file is added to "Trusted sites" and security
:: level for "Tusted sites" is set to "Low".
:: For a generally useable plugin, change the file name of the "certificate.pfx"
:: to that of a valid certificate Personal Information Exchange (.pfx) file.
:: For an example how to use the .cab, see: ieambulentWeb.htm
:: ----------------------------------------------------------------------------
:: copy all files for the cab into this directory
@echo "copying all files for the cab into this directory"
::
:: redistributable C-runtime 
@copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest" Microsoft.VC80.CRT.manifest
@copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll" msvcr80.dll 
@copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll" msvcp80.dll
@copy "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcm80.dll" msvcm80.dll
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest" Microsoft.VC90.CRT.manifest
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcr90.dll" msvcr90.dll 
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcp90.dll" msvcp90.dll
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcm90.dll" msvcm90.dll
:: redistributable Microsoft Foundation classes
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\Microsoft.VC90.MFC.manifest" Microsoft.VC90.MFC.manifest
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\mfc90.dll" mfc90.dll
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\mfc90u.dll" mfc90u.dll
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\mfcm90.dll" mfcm90.dll
@copy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\mfcm90u.dll" mfcm90u.dll
:: use manifest tool
@mt.exe -manifest Microsoft.VC90.CRT.manifest -outputresource:msvcr90.dll;2
:: Ambulant dll's
@copy ..\..\..\bin\win32\libambulant_shwin32.dll libambulant_shwin32.dll
@copy ..\..\..\bin\win32\libamplugin_ffmpeg.dll libamplugin_ffmpeg.dll 
@copy ..\..\..\bin\win32\libamplugin_plugin.dll libamplugin_plugin.dll
@copy ..\..\..\bin\win32\libamplugin_state_xpath.dll libamplugin_state_xpath.dll
@copy ..\..\..\bin\win32\avcodec-51.dll avcodec-51.dll
@copy ..\..\..\bin\win32\avformat-52.dll avformat-52.dll 
@copy ..\..\..\bin\win32\avutil-49.dll avutil-49.dll
@copy ..\..\..\bin\win32\SDL.dll SDL.dll
@copy ..\..\..\bin\win32\xerces-c_2_8.dll xerces-c_2_8.dll
@copy ..\AmbulantActiveX.inf AmbulantActiveX.inf
:: Create a new cabinet (.cab) archive
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\CabArc.Exe" -s 6144 n ieambulant.cab Microsoft.VC90.MFC.manifest Microsoft.VC90.CRT.manifest  Microsoft.VC80.CRT.manifest mfc90.dll mfc90u.dll mfcm90.dll mfcm90u.dll msvcr90.dll msvcp90.dll msvcm90.dll msvcr80.dll msvcp80.dll msvcm80.dll AmbulantActiveX.dll libambulant_shwin32.dll libamplugin_ffmpeg.dll libamplugin_plugin.dll libamplugin_state_xpath.dll avcodec-51.dll avformat-52.dll avutil-49.dll SDL.dll xerces-c_2_8.dll AmbulantActiveX.inf
:: Code sign it with code signing certificate (.pfx = Personal Information Exchange) 
"C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin\signtool" sign /f ..\certificate.pfx /p ambulant /v ieambulant.cab
:: timestamp the signature
"C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin\signtool" timestamp  /v /t "http://timestamp.verisign.com/scripts/timstamp.dll" ieambulant.cab
:: verify the resulting cabinet (.cab) archive
"C:\Program Files\Microsoft SDKs\Windows\v6.1\Bin\signtool" verify /v /a /pa ieambulant.cab
@echo ------------------------------------------------------------------------------
@echo When the code was signed with the default (test) certificate only, you'll see:
@echo SignTool Error: File not valid: ieambulant.cab
@echo ------------------------------------------------------------------------------
@echo "erasing files copied"
@erase Microsoft.VC90.MFC.manifest Microsoft.VC90.CRT.manifest Microsoft.VC80.CRT.manifest mfc90.dll mfc90u.dll mfcm90.dll mfcm90u.dll msvcr90.dll msvcp90.dll msvcm90.dll msvcr80.dll msvcp80.dll msvcm80.dll libambulant_shwin32.dll libamplugin_ffmpeg.dll libamplugin_plugin.dll libamplugin_state_xpath.dll avcodec-51.dll avformat-52.dll avutil-49.dll SDL.dll xerces-c_2_8.dll AmbulantActiveX.inf
