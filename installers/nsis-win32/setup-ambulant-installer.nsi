; Ambulant Player NSIS installer script.
; (see http://nsis.sourceforge.net for details on NSIS).
; 
; These defines influence the rest of the installer.
; Two may need explanation:
; - DISTRIBUTE_DLL_BUILD: define this to select whether you want to distribute
;   a statically built Ambulant or a dll-based Ambulant (which allows for
;   plugins, etc).
; - DISTRIBUTE_PYTHON_PLUGIN: define this to include Python support (dll build only)
; - DISTRIBUTE_VC7_RT: define this if you built with VS2003
; - BUILD_SYSDIR: This is where to pick up some standard DLLs that we distribute.
; - DISTRIBUTE_VC8_RT: define this if you built with VS2005
; - VC8_DISTDIR: where to pick up these files.
; - DISTRIBUTE_VC9_RT: define this if you built with VS2008
; - VC9_DISTDIR: where to pick up these files.
; - DOWNLOAD_VC9_RT: define this to download VS2008 C++ runtime from Microsoft.
; - DOWNLOAD_VC9_RT_URL: where to get it from.
;
!define PRODUCT_NAME "Ambulant Player"
!define PRODUCT_VERSION "2.6"
!define PRODUCT_VERSION_BASE "2.6"
!define DISTRIBUTE_DLL_BUILD
; !define DISTRIBUTE_PYTHON_PLUGIN
!define PRODUCT_PUBLISHER "Centrum voor Wiskunde en Informatica"
!define PRODUCT_WEB_SITE "http://www.ambulantplayer.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AmbulantPlayer.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Where the system directory is on the machine where we are building the installer

; !define DISTRIBUTE_VC7_RT
; !define BUILD_SYSDIR "C:\WINDOWS\system32"  ; Most machines

; !define DISTRIBUTE_VC8_RT
; !define VC8_DISTDIR "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86"

; !define DISTRIBUTE_VC9_RT
; !define VC9_DISTDIR "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86"

; !define DOWNLOAD_VC9_RT
; !define DOWNLOAD_VC9_RT_URL "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe"

!define DOWNLOAD_VC10_RT
!define DOWNLOAD_VC10_RT_URL "http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"

; File associations
!include "FileAssociation.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "AmbulantInstallerIcon.ico"
!define MUI_UNICON "AmbulantUninstallerIcon.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "image.bmp"

; These macros define which pages the installer will have.
;
; The Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
; Component selection page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\AmbulantPlayer.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Also see the very end of the file, where the MUI descriptions are
; defined.
; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Ambulant-${PRODUCT_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\AmbulantPlayer-${PRODUCT_VERSION_BASE}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "Core Components" CoreSection

  SetOutPath "$INSTDIR"
  SetOverwrite on
!ifdef DISTRIBUTE_DLL_BUILD
  File /ONAME=AmbulantPlayer.exe "..\..\bin\win32\AmbulantPlayer.exe"
  File "..\..\bin\win32\libambulant_shwin32.dll"
  File "..\..\bin\win32\libamplugin_state_xpath.dll"
  File "..\..\bin\win32\libamplugin_ffmpeg.dll"
  File "..\..\bin\win32\libamplugin_xerces.dll"
  File "..\..\bin\win32\avcodec-55.dll"
  File "..\..\bin\win32\avformat-55.dll"
  File "..\..\bin\win32\avutil-52.dll"
  File "..\..\bin\win32\swscale-2.dll"
  File "..\..\bin\win32\swresample-0.dll"
  File "..\..\bin\win32\SDL.dll"
!else
  File "..\..\bin\win32\AmbulantPlayer.exe"
!endif
  ${registerExtension} "$INSTDIR\AmbulantPlayer.exe" ".smil" "SMIL Multimedia Presentation"
  ${registerExtension} "$INSTDIR\AmbulantPlayer.exe" ".smi" "SMIL Multimedia Presentation"
  ; registerExtension uses ,0 for the icon, which is incorrect. Fix it:
  WriteRegStr HKCR "SMIL Multimedia Presentation\DefaultIcon" "" "$INSTDIR\AmbulantPlayer.exe,1"

  CreateDirectory "$SMPROGRAMS\Ambulant"
  CreateDirectory "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Ambulant Player.lnk" "$INSTDIR\AmbulantPlayer.exe"
  CreateShortCut "$DESKTOP\Ambulant Player.lnk" "$INSTDIR\AmbulantPlayer.exe"
  File "..\..\bin\win32\xerces-c_3_1.dll"
  File "..\..\bin\win32\settings.xml"
  File /ONAME=license.txt "..\..\COPYING"
  File /ONAME=Readme.txt "..\..\README"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Read Me.lnk" "$INSTDIR\Readme.txt"
  File /ONAME=News.txt "..\..\NEWS"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\What's new?.lnk" "$INSTDIR\News.txt"
  File "..\..\Documentation\user-htmlhelp\AmbulantPlayerHelp.chm"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Documentation.lnk" "$INSTDIR\AmbulantPlayerHelp.chm"
; XXX Copy pyamplugin_scripting
  SetOutPath "$INSTDIR\Extras\DTDCache"
  File "..\..\Extras\DTDCache\mapping.txt"
  SetOutPath "$INSTDIR\Extras\DTDCache\Smil20"
  File "..\..\Extras\DTDCache\Smil20\*.*"
  SetOutPath "$INSTDIR\Extras\DTDCache\Smil21"
  File "..\..\Extras\DTDCache\Smil21\*.*"
  SetOutPath "$INSTDIR\Extras\DTDCache\Smil30"
  File "..\..\Extras\DTDCache\Smil30\*.*"

  SetOutPath "$INSTDIR\Extras\Welcome"
  File "..\..\Extras\Welcome\*.smil"
  SetOutPath "$INSTDIR\Extras\Welcome\data"
  File "..\..\Extras\Welcome\data\*.png"
  File "..\..\Extras\Welcome\data\*.mp3"
SectionEnd

Section /o "VC++ Runtime" RuntimeSection

!ifdef DISTRIBUTE_VC7_RT
; *** The all critical MSVC7 Dependencies
; *** Ideally this list is complete?

  SetOutPath "$SYSDIR"
  SetOverwrite off
  File "${BUILD_SYSDIR}\MSVCR71.dll"
  File "${BUILD_SYSDIR}\MSVCP71.dll"
  File "${BUILD_SYSDIR}\msvcrt.dll"
  File "${BUILD_SYSDIR}\mfc71u.dll"
  File "${BUILD_SYSDIR}\MFC71ENU.DLL"
!endif
!ifdef DISTRIBUTE_VC8_RT
; *** MSVC8 runtime goes into the application directory
  SetOutPath "$INSTDIR"
  File "${VC8_DISTDIR}\Microsoft.VC80.CRT\msvcr80.dll"
  File "${VC8_DISTDIR}\Microsoft.VC80.CRT\msvcp80.dll"
  File "${VC8_DISTDIR}\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"
  File "${VC8_DISTDIR}\Microsoft.VC80.MFC\mfc80u.dll"
  File "${VC8_DISTDIR}\Microsoft.VC80.MFC\Microsoft.VC80.MFC.manifest"
!endif
!ifdef DISTRIBUTE_VC9_RT
; *** MSVC9 runtime goes into the application directory
  SetOutPath "$INSTDIR"
  File "${VC9_DISTDIR}\Microsoft.VC90.CRT\msvcr90.dll"
  File "${VC9_DISTDIR}\Microsoft.VC90.CRT\msvcm90.dll"
  File "${VC9_DISTDIR}\Microsoft.VC90.CRT\msvcp90.dll"
  File "${VC9_DISTDIR}\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest"
  File "${VC9_DISTDIR}\Microsoft.VC90.MFC\mfc90u.dll"
  File "${VC9_DISTDIR}\Microsoft.VC90.MFC\Microsoft.VC90.MFC.manifest"
!endif
!ifdef DOWNLOAD_VC9_RT
;
; Download from Microsoft. Code (and idea) from <http://wiki.spench.net/wiki/NISRP_Installer_Script>
;
!define DOWNLOAD_LOCATION	"$INSTDIR\Downloaded"
!define RUNTIME_FILE		"${DOWNLOAD_LOCATION}\vcredist_x86_2008_sp1.exe"
	AddSize 4119
	# Download is placed in installation directory
	SetOutPath ${DOWNLOAD_LOCATION}
	NSISdl::download "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe" "${RUNTIME_FILE}"
	Pop $R0 ;Get the return value
	StrCmp $R0 "success" install
	MessageBox MB_YESNO|MB_ICONEXCLAMATION "Download of Microsoft VC++ runtime failed, maybe your internet connection is down?$\n$\n Ambulant will not run until this component is correctly installed.$\n$\nDo you wish to continue?" IDYES true IDNO false
	true:
		Goto finish
	false:
		Abort "Download failed: $R0"
install:
	# FIXME: Silent install?
	ExecWait "${RUNTIME_FILE}"
finish:

!endif
!ifdef DOWNLOAD_VC10_RT
;
; Download from Microsoft. Code (and idea) from <http://wiki.spench.net/wiki/NISRP_Installer_Script>
;
!define DOWNLOAD_LOCATION	"$INSTDIR\Downloaded"
!define RUNTIME_FILE		"${DOWNLOAD_LOCATION}\vcredist_x86_2008_sp1.exe"
	AddSize 4119
	# Download is placed in installation directory
	SetOutPath ${DOWNLOAD_LOCATION}
	NSISdl::download ${DOWNLOAD_VC10_RT_URL} "${RUNTIME_FILE}"
	Pop $R0 ;Get the return value
	StrCmp $R0 "success" install
	MessageBox MB_YESNO|MB_ICONEXCLAMATION "Download of Microsoft VC++ runtime failed, maybe your internet connection is down?$\n$\n Ambulant will not run until this component is correctly installed.$\n$\nDo you wish to continue?" IDYES true IDNO false
	true:
		Goto finish
	false:
		Abort "Download failed: $R0"
install:
	# FIXME: Silent install?
	ExecWait "${RUNTIME_FILE}"
finish:

!endif
SectionEnd

Section "Demo Presentation" DemoSection
  SetOutPath "$INSTDIR\Extras\DemoPresentation"
  File "..\..\Extras\DemoPresentation\*.smil"
  File "..\..\Extras\DemoPresentation\*.xml"
  File "..\..\Extras\DemoPresentation\NYC-ReadMe.txt"
  SetOutPath "$INSTDIR\Extras\DemoPresentation\NYCdata"
  File "..\..\Extras\DemoPresentation\NYCdata\*.gif"
  File "..\..\Extras\DemoPresentation\NYCdata\*.txt"
  File "..\..\Extras\DemoPresentation\NYCdata\*.mp3"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Demo Presentation.lnk" "$INSTDIR\Extras\DemoPresentation\NYC-SMIL2.smil"
SectionEnd

!ifdef DISTRIBUTE_PYTHON_PLUGIN
Section "Python Plugin" PythonSection
  SetOutPath "$INSTDIR"
  File "..\..\bin\win32\libamplugin_python.dll"
  File "..\..\bin\win32\ambulant.pyd"
!ifdef DISTRIBUTE_PYTHON_STATE_PLUGN
  SetOutPath "$INSTDIR\pyamplugin_state"
  File "..\..\bin\win32\pyamplugin_state\*.py"
  File "..\..\bin\win32\pyamplugin_state\*.pyc"
  File /nonfatal "..\..\bin\win32\pyamplugin_state\*.pyo"
!endif
SectionEnd
!endif

Section -AdditionalIcons
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}\Uninstall.lnk" "$INSTDIR\uninst.exe"
; XXXJACK: need help, readme, demo document
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"

  WriteRegStr HKCR "ambulant" "" "URL:Ambulant Protocol"
  WriteRegStr HKCR "ambulant" "URL Protocol" ""
  WriteRegStr HKCR "ambulant\shell\open\command" "" "$INSTDIR\AmbulantPlayer.exe $\"%1$\""

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\AmbulantPlayer.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\AmbulantPlayer.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function .onInit
  ; The core section is always installed
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${CoreSection} $0
  
!ifdef DOWNLOAD_VC9_RT
  ; Do we need to install the VC9 runtime?
  ; Don't install if SP1 or later has been installed.
  ; Note that a missing key reads as "0".
  ReadRegDWORD $0 HKLM SOFTWARE\Microsoft\DevDiv\VC\Servicing\9.0 SP
  IntCmp $0 1 +2 0 +2
    SectionSetFlags ${RuntimeSection} ${SF_SELECTED}
!endif
!ifdef DOWNLOAD_VC10_RT
  ; Do we need to install the VC10 runtime?
  ; Don't install if VC10 (any SP) has been installed.
  ; Note that a missing key reads as "0".
  ReadRegDWORD $0 HKLM SOFTWARE\Microsoft\DevDiv\VC\Servicing\9.0 SP
  IfErrors 0 +1
    SectionSetFlags ${RuntimeSection} ${SF_SELECTED}
!endif

FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\AmbulantPlayer.exe"
  Delete "$INSTDIR\libambulant_shwin32.dll"
  Delete "$INSTDIR\libamplugin_state_xpath.dll"
  Delete "$INSTDIR\libamplugin_ffmpeg.dll"
  Delete "$INSTDIR\libamplugin_xerces.dll"
  Delete "$INSTDIR\avcodec-55.dll"
  Delete "$INSTDIR\avformat-55.dll"
  Delete "$INSTDIR\avutil-52.dll"
  Delete "$INSTDIR\swscale-2.dll"
  Delete "$INSTDIR\swresample-0.dll"
  Delete "$INSTDIR\SDL.dll"

  RMDir /r "$INSTDIR\Extras"
  
  RMDir /r "$INSTDIR\Downloaded"

  Delete "$INSTDIR\xerces-c_3_1.dll"
  Delete "$INSTDIR\settings.xml"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\Readme.txt"
  Delete "$INSTDIR\News.txt"
  Delete "$INSTDIR\AmbulantPlayerHelp.chm"

  Delete "$DESKTOP\Ambulant Player.lnk"

  Delete "$INSTDIR\MSVCR71.dll"
  Delete "$INSTDIR\MSVCP71.dll"
  Delete "$INSTDIR\msvcrt.dll"
  Delete "$INSTDIR\mfc71u.dll"
  Delete "$INSTDIR\MFC71ENU.DLL"
  Delete "$INSTDIR\msvcr80.dll"
  Delete "$INSTDIR\msvcp80.dll"
  Delete "$INSTDIR\Microsoft.VC80.CRT.manifest"
  Delete "$INSTDIR\mfc80u.dll"
  Delete "$INSTDIR\Microsoft.VC80.MFC.manifest"

  Delete "$INSTDIR\libamplugin_python.dll"
  Delete "$INSTDIR\ambulant.pyd"
  RMDir  "$INSTDIR\pyamplugin_state"

  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  RMDir /r "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION_BASE}"
  Delete "$INSTDIR\uninst.exe"
  RMDir "$INSTDIR"
  RMDir "$SMPROGRAMS\Ambulant"

  ${unregisterExtension} ".smil" "SMIL Multimedia Presentation"
  ${unregisterExtension} ".smi" "SMIL Multimedia Presentation"
 
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
  
SectionEnd

; And define the text for the "description" boxes in the selection
; page.

LangString DESC_CoreSection ${LANG_ENGLISH} "The player, help files, readme and other required components."
LangString DESC_RuntimeSection ${LANG_ENGLISH} "The Microsoft VC++ Runtime support required by Ambulant."
LangString DESC_DemoSection ${LANG_ENGLISH} "A simple slideshow example document."
!ifdef DISTRIBUTE_PYTHON_PLUGIN
LangString DESC_PythonSection ${LANG_ENGLISH} "Enable Ambulant extensions in Python, SMIL Python scripting and embedding Ambulant in Python."
!endif

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 !insertmacro MUI_DESCRIPTION_TEXT ${CoreSection} $(DESC_CoreSection)
 !insertmacro MUI_DESCRIPTION_TEXT ${DemoSection} $(DESC_DemoSection)
 !insertmacro MUI_DESCRIPTION_TEXT ${RuntimeSection} $(DESC_RuntimeSection)
 !ifdef DISTRIBUTE_PYTHON_PLUGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${PythonSection} $(DESC_PythonSection)
 !endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END
