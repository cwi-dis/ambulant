:: Script to create CAB file for IEAmbulant ActiveX plugin.
:: NOTE: Do not run this script, btu copy the contents into
:: the IEAmbulant-Installer project, "make" command lines.
::
:: This script expects to be run from within Visual Studio only,
:: because it depends on various variable names defined there.
::
:: NOTE: if you change this file you must also change the
:: AmbulantActiveX.inf file, the contents of the two files
:: MUST match.
::
:: For an example how to use the .cab, see: ieambulentWeb.htm
:: ----------------------------------------------------------------------------
:: copy all files for the cab into this directory
@echo on

;; VS2010 no longer seems to set this...
set TargetPath=$(OutDir)ieambulantDX-2.3-win32.cab
;;
@echo "copying all files for the cab into this directory"
del $(IntDir)\*.*
mkdir $(IntDir)
::
:: redistributable C-runtime
copy ^"$(WindowsSDKDir)\Bootstrapper\Packages\vcredist_x86\vcredist_x86.exe^" $(intdir)\vcredist_x86.exe
:: Ambulant dll's
copy ..\..\bin\win32\AmbulantActiveXDX.dll $(intdir)\AmbulantActiveXDX.dll
copy ..\..\bin\win32\libambulantDX_shwin32.dll $(intdir)\libambulantDX_shwin32.dll
copy ..\..\bin\win32\libamplugin_ffmpegDX.dll $(intdir)\libamplugin_ffmpegDX.dll 
copy ..\..\bin\win32\libamplugin_state_xpathDX.dll $(intdir)\libamplugin_state_xpathDX.dll
copy ..\..\bin\win32\avcodec-52.dll $(intdir)\avcodec-52.dll
copy ..\..\bin\win32\avformat-52.dll $(intdir)\avformat-52.dll 
copy ..\..\bin\win32\avutil-50.dll $(intdir)\avutil-50.dll
copy ..\..\bin\win32\avcore-0.dll $(intdir)\avcore-0.dll
copy ..\..\bin\win32\swscale-0.dll $(intdir)\swscale-0.dll
copy ..\..\bin\win32\SDL.dll $(intdir)\SDL.dll
copy ..\..\bin\win32\xerces-c_3_1.dll $(intdir)\xerces-c_3_1.dll
copy ..\..\src\ieambulant\AmbulantActiveXDX.inf $(intdir)\AmbulantActiveXDX.inf
:: Create a new cabinet (.cab) archive
^"$(CabArc)^" -s 6144 n %TargetPath% $(IntDir)*.exe $(IntDir)*.dll $(IntDir)*.inf
:: Code sign it with code signing certificate (.pfx = Personal Information Exchange) 
set signtool=^"$(WindowsSDKDir)Bin\signtool.exe^"
%signtool% sign /f $(ieambulant_certificate) /p Ambulant /v %TargetPath%
:: timestamp the signature
%signtool% timestamp  /v /t "http://timestamp.verisign.com/scripts/timstamp.dll" %TargetPath%
:: verify the resulting cabinet^" (.cab) archive
%signtool% verify /v /a /pa %TargetPath%