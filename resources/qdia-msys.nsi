!define APPNAME "QDia"
!define DESCRIPTION "QDia is a simple diagram/schematic editor."
# These three must be integers
!define VERSIONMAJOR 0
!define VERSIONMINOR 4
!define VERSIONBUILD 0
# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client
!define HELPURL "https://github.com/sunderme/qdia" # "Support Information" link
!define UPDATEURL "https://github.com/sunderme/qdia" # "Product Updates" link
!define ABOUTURL "https://github.com/sunderme/qdia" # "Publisher" link

!include "FileAssociation.nsh"

# Include Modern UI

!include "MUI2.nsh"

# define the name of the installer
Outfile "qdia_installer.exe"

RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)
 
InstallDir "$PROGRAMFILES\qdia"

# Get installation folder from registry if available
InstallDirRegKey HKCU "Software\${APPNAME}" ""

ManifestDPIAware true

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

  ;Show all languages, despite user's codepage
  !define MUI_LANGDLL_ALLLANGUAGES

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\${APPNAME}" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"


Name "${APPNAME}"

#page directory
#page instfiles
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" ; The first language is the default language
  

#!macro VerifyUserIsAdmin
#UserInfo::GetAccountType
#pop $0
#${If} $0 != "admin" ;Require admin rights on NT4+
#        messageBox mb_iconstop "Administrator rights required!"
#        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
#        quit
#${EndIf}
#!macroend

# default section
Section "install"

# sets $SMPROGRAMS to all users 
SetShellVarContext all

# define the output path for this file
SetOutPath $INSTDIR
 
# define what to install and place it in the output path
File qdia.exe

File package-zip\*.dll

SetOutPath $INSTDIR\platforms

File package-zip\platforms\*

SetOutPath $INSTDIR\imageformats

File package-zip\imageformats\*

SetOutPath $INSTDIR\iconengines

File package-zip\iconengines\*

SetOutPath $INSTDIR\styles

File package-zip\styles\*

#SetOutPath $INSTDIR\translations

#File translations\*.qm

# Store installation folder
WriteRegStr HKCU "Software\${APPNAME}" "" $INSTDIR
# define uninstaller name
WriteUninstaller $INSTDIR\uninstall.exe

# associate .tex
${registerExtension} $INSTDIR\qdia.exe ".qdia" "QDia File"


# Start Menu
createShortCut "$SMPROGRAMS\${APPNAME}.lnk" \
"$INSTDIR\qdia.exe" "" ""

# Registry information for add/remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME} - ${DESCRIPTION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$\"$INSTDIR\logo.ico$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "Jan Sundermeyer"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "$\"${HELPURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
 
SectionEnd

# Uninstaller
 
function un.onInit
	SetShellVarContext all
 
	#Verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Permanently remove ${APPNAME}?" /SD IDOK IDOK next
		Abort
	next:
	#!insertmacro VerifyUserIsAdmin
functionEnd

# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
Section "Uninstall"

# sets $SMPROGRAMS to all users 
SetShellVarContext all
	
# Remove Start Menu launcher
delete "$SMPROGRAMS\${APPNAME}.lnk"
 
# Always delete uninstaller first
Delete $INSTDIR\uninstaller.exe

# remove file association
${unregisterExtension} ".tex" "tex File"
 
# now delete installed file
RMDir /r $INSTDIR\translations
RMDir /r $INSTDIR\templates
RMDir /r $INSTDIR\help
RMDir /r $INSTDIR\imageformats
RMDir /r $INSTDIR\platforms
RMDir /r $INSTDIR\styles
Delete $INSTDIR\*

# Try to remove the install directory - this will only happen if it is empty
RMDir $INSTDIR

# Remove uninstaller information from the registry
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
DeleteRegKey /ifempty HKCU "Software\${APPNAME}"
 
SectionEnd
