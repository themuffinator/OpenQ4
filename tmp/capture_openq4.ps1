param(
  [string]$ExePath,
  [string[]]$Args,
  [string]$ScreenshotPath,
  [int]$WaitForWindowSeconds = 15,
  [int]$WaitAfterWindowSeconds = 10
)

Add-Type -AssemblyName System.Drawing

$signature = @"
using System;
using System.Runtime.InteropServices;
public static class Win32 {
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);
  public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT pt);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
  public const int SW_RESTORE = 9;
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X; public int Y; }
}
"@
Add-Type $signature

function Find-WindowHandle([int]$procId) {
  $script:found = [IntPtr]::Zero
  $callback = [Win32+EnumWindowsProc] {
    param([IntPtr]$hWnd, [IntPtr]$lParam)
    $outPid = 0
    [Win32]::GetWindowThreadProcessId($hWnd, [ref]$outPid) | Out-Null
    if ($outPid -ne $procId) { return $true }
    $rect = New-Object Win32+RECT
    if (-not [Win32]::GetClientRect($hWnd, [ref]$rect)) { return $true }
    $w = $rect.Right - $rect.Left
    $h = $rect.Bottom - $rect.Top
    if ($w -le 0 -or $h -le 0) { return $true }
    $script:found = $hWnd
    return $false
  }
  [Win32]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null
  return $script:found
}

function Get-ValidWindowHandle([int]$procId, [int]$timeoutSeconds) {
  $sw = [Diagnostics.Stopwatch]::StartNew()
  while ($sw.Elapsed.TotalSeconds -lt $timeoutSeconds) {
    try {
      $pinfo = Get-Process -Id $procId -ErrorAction Stop
      if ($pinfo.MainWindowHandle -ne 0) { return [IntPtr]$pinfo.MainWindowHandle }
    } catch { }
    $h = Find-WindowHandle -procId $procId
    if ($h -ne [IntPtr]::Zero) { return $h }
    Start-Sleep -Milliseconds 200
  }
  return [IntPtr]::Zero
}

if (-not (Test-Path $ExePath)) { throw "Exe not found: $ExePath" }
$proc = Start-Process -FilePath $ExePath -ArgumentList $Args -PassThru
$hwnd = Get-ValidWindowHandle -procId $proc.Id -timeoutSeconds $WaitForWindowSeconds
if ($hwnd -eq [IntPtr]::Zero) {
  try { $proc.CloseMainWindow() | Out-Null } catch {}
  throw "Window not found for process $($proc.Id)"
}
[Win32]::ShowWindow($hwnd, [Win32]::SW_RESTORE) | Out-Null
[Win32]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Seconds $WaitAfterWindowSeconds

$rect = New-Object Win32+RECT
$maxTries = 20
for ($i = 0; $i -lt $maxTries; $i++) {
  if ([Win32]::GetClientRect($hwnd, [ref]$rect)) {
    $width = $rect.Right - $rect.Left
    $height = $rect.Bottom - $rect.Top
    if ($width -gt 0 -and $height -gt 0) { break }
  }
  $hwnd = Get-ValidWindowHandle -procId $proc.Id -timeoutSeconds 2
  Start-Sleep -Milliseconds 200
}
if (-not [Win32]::GetClientRect($hwnd, [ref]$rect)) {
  throw "GetClientRect failed"
}
$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top
if ($width -le 0 -or $height -le 0) {
  throw "Invalid client size: ${width}x${height}"
}

$pt = New-Object Win32+POINT
$pt.X = $rect.Left
$pt.Y = $rect.Top
[Win32]::ClientToScreen($hwnd, [ref]$pt) | Out-Null

$bmp = New-Object System.Drawing.Bitmap $width, $height
$gfx = [System.Drawing.Graphics]::FromImage($bmp)
$gfx.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size)
$gfx.Dispose()
$bmp.Save($ScreenshotPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()

try { $proc.CloseMainWindow() | Out-Null } catch {}
Start-Sleep -Seconds 3
if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force }

"Saved screenshot to $ScreenshotPath"
