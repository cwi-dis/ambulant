;
; Ambulant Player NSIS installer script.
; (see http://nsis.sourceforge.net for details on NSIS).
;
; Created by Jack Jansen, May 2007.
; 
; These defines influence the rest of the installer.
; Two may need explanation:
; - DISTRIBUTE_DLL_BUILD: define this to select whether you want to distribute
;   a statically built Ambulant or a dll-based Ambulant (which allows for
;   plugins, etc).
; - DISTRIBUTE_PYTHON_PLUGIN: define this to include Python support (dll build only)
; - BUILD_SYSDIR: This is where to pick up some standard DLLs that we distribute.
;
!define PRODUCT_NAME "Ambulant Player"
!define PRODUCT_VERSION "1.9"
!define DISTRIBUTE_DLL_BUILD
; !define DISTRIBUTE_PYTHON_PLUGIN
!define PRODUCT_PUBLISHER "Centrum voor Wiskunde en Informatica"
!define PRODUCT_WEB_SITE "http://www.ambulantplayer.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\AmbulantPlayer.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} ${PRODUCT_VERSION}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Where the system directory is on the machine where we are building the installer

!define BUILD_SYSDIR "C:\WINDOWS\system32"  ; Most machines
;!define BUILD_SYSDIR "E:\WINNT\system32"   ; Jack's CWI desktop machine

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
InstallDir "$PROGRAMFILES\AmbulantPlayer-${PRODUCT_VERSION}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "Core Components" CoreSection

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
!ifdef DISTRIBUTE_DLL_BUILD
  File /ONAME=AmbulantPlayer.exe "..\..\bin\win32\AmbulantPlayer_shared.exe"
  File "..\..\bin\win32\libambulant_shwin32.dll"
  File "..\..\bin\win32\libamplugin_state_xpath.dll"
!else
  File "..\..\bin\win32\AmbulantPlayer.exe"
!endif
  CreateDirectory "$SMPROGRAMS\Ambulant"
  CreateDirectory "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Ambulant Player.lnk" "$INSTDIR\AmbulantPlayer.exe"
  CreateShortCut "$DESKTOP\Ambulant Player.lnk" "$INSTDIR\AmbulantPlayer.exe"
  File "..\..\bin\win32\xerces-c_2_7.dll"
  File "..\..\bin\win32\settings.xml"
  File /ONAME=license.txt "..\..\COPYING"
  File /ONAME=Readme.txt "..\..\README"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Read Me.lnk" "$INSTDIR\Readme.txt"
  File "..\..\Documentation\user-htmlhelp\AmbulantPlayerHelp.chm"
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Documentation.lnk" "$INSTDIR\AmbulantPlayerHelp.chm"
; XXX Copy pyamplugin_scripting
  SetOutPath "$INSTDIR\Extras\DTDCache"
  File "..\..\Extras\DTDCache\mapping.txt"
  SetOutPath "$INSTDIR\Extras\DTDCache\Smil20"
  File "..\..\Extras\DTDCache\Smil20\*.*"
  SetOutPath "$INSTDIR\Extras\DTDCache\Smil21"
  File "..\..\Extras\DTDCache\Smil21\*.*"

  SetOutPath "$INSTDIR\Extras\Welcome"
  File "..\..\Extras\Welcome\Welcome.smil"
  SetOutPath "$INSTDIR\Extras\Welcome\data"
  File "..\..\Extras\Welcome\data\*.png"
  File "..\..\Extras\Welcome\data\*.mp3"

; *** The all critical MSVC7 Dependencies
; *** Ideally this list is complete?

  SetOutPath "$SYSDIR"
  SetOverwrite off
  File "${BUILD_SYSDIR}\MSVCR71.dll"
  File "${BUILD_SYSDIR}\MSVCP71.dll"
  File "${BUILD_SYSDIR}\msvcrt.dll"
  File "${BUILD_SYSDIR}\mfc71u.dll"
  File "${BUILD_SYSDIR}\MFC71ENU.DLL"
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
  CreateShortcut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Demo Presentation.lnk" "$INSTDIR\Extras\DemoPresentation\NYC-SMIL2.smil"
SectionEnd

!ifdef DISTRIBUTE_PYTHON_PLUGIN
Section "Python Plugin" PythonSection
  SetOutPath "$INSTDIR"
  File "..\..\bin\win32\libamplugin_python.dll"
  File "..\..\bin\win32\ambulant.pyd"
  SetOutPath "$INSTDIR\pyamplugin_scripting"
  File "..\..\bin\win32\pyamplugin_scripting\*.py"
  File "..\..\bin\win32\pyamplugin_scripting\*.pyc"
  File /nonfatal "..\..\bin\win32\pyamplugin_scripting\*.pyo"
SectionEnd
!endif

Section -AdditionalIcons
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Ambulant\${PRODUCT_NAME} ${PRODUCT_VERSION}\Uninstall.lnk" "$INSTDIR\uninst.exe"
; XXXJACK: need help, readme, demo document
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\AmbulantPlayer.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\AmbulantPlayer.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd



Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
; XXXJack need to check the filenames here too
  Delete "$INSTDIR\uninst.exe"
 
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
  
SectionEnd

; And define the text for the "description" boxes in the selection
; page.

LangString DESC_CoreSection ${LANG_ENGLISH} "The player, help files, readme and other required components."
LangString DESC_DemoSection ${LANG_ENGLISH} "A simple slideshow example document."
!ifdef DISTRIBUTE_PYTHON_PLUGIN
LangString DESC_PythonSection ${LANG_ENGLISH} "Enable Ambulant extensions in Python, SMIL Python scripting and embedding Ambulant in Python."
!endif

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 !insertmacro MUI_DESCRIPTION_TEXT ${CoreSection} $(DESC_CoreSection)
 !insertmacro MUI_DESCRIPTION_TEXT ${DemoSection} $(DESC_DemoSection)
 !ifdef DISTRIBUTE_PYTHON_PLUGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${PythonSection} $(DESC_PythonSection)
 !endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END
