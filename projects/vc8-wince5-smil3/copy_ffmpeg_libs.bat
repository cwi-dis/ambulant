rem Copy and modify ffmpeg libraries
rem
rem First setup tools environment
call "%VS80COMNTOOLS%vsvars32.bat"
rem
rem Copy DLLs
set FFMPEG="..\..\third_party_packages\ffmpeg-wm5"
copy %FFMPEG%\libavformat\avformat-52.dll ..\..\bin\wince\avformat-52.dll
copy %FFMPEG%\libavcodec\avcodec-51.dll ..\..\bin\wince\avcodec-51.dll
copy %FFMPEG%\libavutil\avutil-49.dll ..\..\bin\wince\avutil-49.dll
rem
rem Copy and convert .def files to .lib files
lib /machine:arm /out:..\..\lib\wince-arm\avformat.lib /def:%FFMPEG%\libavformat\avformat-52.def
lib /machine:arm /out:..\..\lib\wince-arm\avcodec.lib /def:%FFMPEG%\libavcodec\avcodec-51.def
lib /machine:arm /out:..\..\lib\wince-arm\avutil.lib /def:%FFMPEG%\libavutil\avutil-49.def
rem
rem Copy SDL library
set SDL="..\..\third_party_packages\SDL-1.2.12-wm5\VisualCE\SDL\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)"
copy %SDL%\Release\SDL.lib ..\..\lib\wince-arm\SDL.lib
copy %SDL%\Release\SDL.dll ..\..\bin\wince\SDL.dll
rem XXX copy %SDL%\Debug\SDL.lib ..\..\lib\wince-arm\SDL.lib
rem XXX copy %SDL%\Debug\SDL.dll ..\..\bin\wince\SDL.dll
rem
rem Copy live555 libraries
set LIVE="..\..\third_party_packages\live_VC8_wince\BUILD"
copy %LIVE%\BasicUsageEnvironment-Debug\BasicUsageEnvironmentD.lib ..\..\lib\wince-arm\BasicUsageEnvironmentD.lib
copy %LIVE%\UsageEnvironment-Debug\UsageEnvironmentD.lib ..\..\lib\wince-arm\UsageEnvironmentD.lib
copy %LIVE%\groupsock-Debug\groupsockD.lib ..\..\lib\wince-arm\groupsockD.lib
copy %LIVE%\liveMedia-Debug\liveMediaD.lib ..\..\lib\wince-arm\liveMediaD.lib
copy %LIVE%\BasicUsageEnvironment-Release\BasicUsageEnvironment.lib ..\..\lib\wince-arm\BasicUsageEnvironment.lib
copy %LIVE%\UsageEnvironment-Release\UsageEnvironment.lib ..\..\lib\wince-arm\UsageEnvironment.lib
copy %LIVE%\groupsock-Release\groupsock.lib ..\..\lib\wince-arm\groupsock.lib
copy %LIVE%\liveMedia-Release\liveMedia.lib ..\..\lib\wince-arm\liveMedia.lib
