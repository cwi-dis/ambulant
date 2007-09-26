rem Copy and modify ffmpeg libraries
rem
rem First setup tools environment
call "%VS80COMNTOOLS%vsvars32.bat"
rem
rem Copy DLLs
set FFMPEG="..\..\third_party_packages\ffmpeg-wm5"
copy %FFMPEG%\libavformat\avformat-51.dll ..\..\bin\wince\avformat-51.dll
copy %FFMPEG%\libavcodec\avcodec-51.dll ..\..\bin\wince\avcodec-51.dll
copy %FFMPEG%\libavutil\avutil-49.dll ..\..\bin\wince\avutil-49.dll
rem
rem Copy and convert .def files to .lib files
lib /machine:arm /out:..\..\lib\wince-arm\avformat.lib /def:%FFMPEG%\libavformat\avformat-51.def
lib /machine:arm /out:..\..\lib\wince-arm\avcodec.lib /def:%FFMPEG%\libavcodec\avcodec-51.def
lib /machine:arm /out:..\..\lib\wince-arm\avutil.lib /def:%FFMPEG%\libavutil\avutil-49.def
