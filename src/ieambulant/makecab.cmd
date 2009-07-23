:: To create and codesign ieambulant.cab, first build Ambulant-win32.sln
:: then ieambulant. CD to "Release" directory, and run this script:
:: ../makecab.cmd
::
:: Create a new cabinet (.cab) archive
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\CabArc.Exe" -s 6144 n ieambulant.cab AmbulantActiveX.dll libambulant_shwin32.dll libamplugin_ffmpeg.dll libamplugin_plugin.dll libamplugin_state_xpath.dll avcodec-51.dll avformat-52.dll avutil-49.dll  SDL.dll xerces-c_2_8.dll AmbulantActiveX.inf
:: Code sign it with a test certificate
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" sign /f ..\mycert.pfx /p ambulant /v ieambulant.cab
:: timestamp the signature
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" timestamp  /v /t "http://timestamp.verisign.com/scripts/timstamp.dll" ieambulant.cab
:: verify the resulting cabinet (.cab) archive
"C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\Bin\signtool" verify /v /a ieambulant.cab
@echo --------------------------------------------------------------------
@echo Because the code is signed with a test certificate only, you'll see:
@echo SignTool Error: File not valid: ieambulant.cab
@echo --------------------------------------------------------------------
