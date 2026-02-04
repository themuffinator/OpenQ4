SetCompressor lzma

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "OpenQ4 SDK"
!define PRODUCT_VERSION "0.0.1"
!define PRODUCT_PUBLISHER "DarkMatter Productions"
!define PRODUCT_WEB_SITE "www.darkmatter-quake.com"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "OpenQ4_SDK\EULA.Development Kit.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "OpenQ4_${PRODUCT_VERSION}_SDK.exe"
InstallDir "C:\OpenQ4_SDK\"
ShowInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /R "OpenQ4_SDK\*.*"
SectionEnd

Section -Post
SectionEnd

