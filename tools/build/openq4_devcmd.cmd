@echo off
set "VSDEVCMD="
set "VSROOT="

if defined VSINSTALLDIR (
    set "VSDEVCMD=%VSINSTALLDIR%Common7\Tools\VsDevCmd.bat"
    if exist "%VSDEVCMD%" goto :run_devcmd
)

for /f "usebackq delims=" %%I in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'; if (Test-Path $vswhere) { & $vswhere -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath }"`) do (
    set "VSROOT=%%I"
)

if not defined VSROOT (
    for /f "usebackq delims=" %%I in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'; if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath }"`) do (
        set "VSROOT=%%I"
    )
)

if not defined VSROOT (
    echo [openq4] No Visual Studio installation with C++ tools was found.
    set "OPENQ4_MSVC_ENV=0"
    exit /b 1
)

set "VSDEVCMD=%VSROOT%\Common7\Tools\VsDevCmd.bat"
if not exist "%VSDEVCMD%" (
    echo [openq4] Could not find VsDevCmd.bat at "%VSDEVCMD%".
    set "OPENQ4_MSVC_ENV=0"
    exit /b 1
)

:run_devcmd
call "%VSDEVCMD%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 (
    echo [openq4] Failed to initialize Visual Studio developer environment.
    set "OPENQ4_MSVC_ENV=0"
    exit /b 1
)

set "OPENQ4_MSVC_ENV=1"
echo [openq4] MSVC developer environment ready.
exit /b 0
