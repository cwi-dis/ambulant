# Microsoft Developer Studio Project File - Name="libambulant_win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libambulant_win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libambulant_win32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libambulant_win32.mak" CFG="libambulant_win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libambulant_win32 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libambulant_win32 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libambulant_win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\third_party_packages\expat\lib" /I "..\..\third_party_packages\lib-src\jpeg" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libambulant_win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\third_party_packages\expat\lib" /I "..\..\third_party_packages\lib-src\jpeg" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libambulant_win32 - Win32 Release"
# Name "libambulant_win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common\player.cpp
# End Source File
# Begin Source File

SOURCE=.\common\region.cpp
# End Source File
# Begin Source File

SOURCE=.\common\renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\common\skeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\common\smil_handler.cpp
# End Source File
# Begin Source File

SOURCE=.\common\timeline_builder.cpp
# End Source File
# Begin Source File

SOURCE=.\common\timelines.cpp
# End Source File
# End Group
# Begin Group "lib"

# PROP Default_Filter ""
# Begin Group "lib_win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lib\win32\win32_asb.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\win32\win32_error.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\win32\win32_event_processor.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\win32\win32_factory.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\win32\win32_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\win32\win32_timer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\lib\colors.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\delta_timer.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\expat_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\logger.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\node.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\schema.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\sync_rule.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\time_attrs.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\time_node.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\time_state.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\timegraph.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\lib\tree_builder.cpp
# End Source File
# End Group
# Begin Group "net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\url.cpp

!IF  "$(CFG)" == "libambulant_win32 - Win32 Release"

!ELSEIF  "$(CFG)" == "libambulant_win32 - Win32 Debug"

# ADD CPP /I "..\..\include" /I "..\..\third_party_packages\expat\lib" /I "..\..\third_party_packages\lib-src\jpeg"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\ambulant\config\config.h
# End Source File
# End Group
# Begin Group "inc_lib"

# PROP Default_Filter ""
# Begin Group "inc_lib_win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\ambulant\lib\win32\win32_error.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\win32\win32_event_processor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\win32\win32_mtsync.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\win32\win32_thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\win32\win32_timer.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\include\ambulant\lib\abstract_mtsync.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\asb.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\basic_types.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\callback.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\colors.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\delta_timer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\document.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\event.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\event_processor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\expat_parser.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\filesys.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\gtypes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\mtsync.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\node.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\node_iterator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\node_navigator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\nscontext.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\playable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\refcount.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\sax_handler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\sax_types.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\schema.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\smil_time.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\string_util.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\sync_rule.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\time_attrs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\time_node.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\time_state.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\timegraph.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\timer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\lib\tree_builder.h
# End Source File
# End Group
# Begin Group "inc_common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\ambulant\common\parse_attrs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\player.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\region.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\region_dim.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\region_eval.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\region_node.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\renderer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\skeleton.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\smil_handler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\timeline_builder.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\common\timelines.h
# End Source File
# End Group
# Begin Group "inc_net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\ambulant\net\connection.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\net\datasource.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\net\socket.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ambulant\net\url.h
# End Source File
# End Group
# End Group
# End Target
# End Project
