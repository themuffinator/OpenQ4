[CmdletBinding()]
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Args
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Get-VsDevCmdPath {
    if ($env:VSINSTALLDIR) {
        $candidate = Join-Path $env:VSINSTALLDIR "Common7\Tools\VsDevCmd.bat"
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "Could not locate vswhere.exe."
    }

    $installPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if ([string]::IsNullOrWhiteSpace($installPath)) {
        throw "No Visual Studio installation with C++ tools was found."
    }

    $vsDevCmd = Join-Path $installPath "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
        throw "Could not locate VsDevCmd.bat at '$vsDevCmd'."
    }

    return $vsDevCmd
}

function Quote-CmdArg([string]$Value) {
    if ($Value -match '[\s"&<>|()]') {
        return '"' + ($Value -replace '"', '\"') + '"'
    }
    return $Value
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rcWrapper = Join-Path $scriptDir "rc.cmd"
if (-not (Test-Path $rcWrapper)) {
    throw "WINDRES wrapper not found at '$rcWrapper'."
}

$env:WINDRES = $rcWrapper

$cl = Get-Command cl -ErrorAction SilentlyContinue
if ($null -ne $cl) {
    & meson @Args
    exit $LASTEXITCODE
}

$vsDevCmd = Get-VsDevCmdPath
$mesonCmd = "meson " + (($Args | ForEach-Object { Quote-CmdArg $_ }) -join " ")
$fullCmd = '"' + $vsDevCmd + '" -arch=x64 -host_arch=x64 >nul && ' + $mesonCmd

& $env:ComSpec /d /c $fullCmd
exit $LASTEXITCODE
