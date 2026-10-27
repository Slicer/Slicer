<#
    Put 3D Slicer on the desktop, and pin it to the taskbar (Windows).

    Usage:
        slicer-shortcuts.ps1              add a desktop shortcut, and show how to
                                          pin Slicer to the taskbar
        slicer-shortcuts.ps1 -Desktop     only the desktop shortcut
        slicer-shortcuts.ps1 -Taskbar     only the taskbar
        slicer-shortcuts.ps1 -Remove      remove the desktop shortcut again

    Parameters:
        -Desktop          the desktop shortcut
        -Taskbar          the taskbar. Windows does not let a program pin itself
                          to the taskbar -- that is reserved to the user -- so
                          this shows how to pin Slicer by hand
        -Remove           remove instead of adding
        -SlicerExe <path> the Slicer.exe to use, when this script cannot find
                          the one it ships next to

    Environment:
        SLICER_QUIET      silence progress messages; warnings and errors still show

    The shortcut launches the installation this script sits in, and is named the
    way Slicer's own installer names its desktop shortcut ("Slicer 5.12.3"), so
    uninstalling Slicer removes it too. -Remove deletes only shortcuts that launch
    this installation, so another installed version's are left alone.

    Windows blocks running .ps1 files by default; run this one with:
        powershell -ExecutionPolicy Bypass -File <path>\slicer-shortcuts.ps1
#>
[CmdletBinding()]
param(
    [switch]$Desktop,
    [switch]$Taskbar,
    [switch]$Remove,
    [string]$SlicerExe = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Informational output, suppressed when SLICER_QUIET is set (any non-empty
# value). Warnings and errors go to their own streams and are never silenced.
function Write-Step($msg) { if (-not $env:SLICER_QUIET) { Write-Host "==> $msg" -ForegroundColor Cyan } }
function Write-Note($msg) { if (-not $env:SLICER_QUIET) { Write-Host "    $msg" -ForegroundColor DarkGray } }
function Write-Done($msg) { if (-not $env:SLICER_QUIET) { Write-Host $msg -ForegroundColor Green } }

# The Slicer.exe belonging to the installation this script sits in. It is
# installed into Slicer's own bin directory, so the launcher is one level up and
# nothing has to be passed in.
function Find-Slicer {
    if (-not $PSScriptRoot) { return '' }
    $root = Split-Path -Parent $PSScriptRoot
    foreach ($candidate in @((Join-Path $root 'Slicer.exe'), (Join-Path $PSScriptRoot 'Slicer.exe'))) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) { return $candidate }
    }
    return ''
}

# The name Slicer's installer gave the installation in $Root: the DisplayName of
# the uninstall entry it registered ("Slicer 5.12.3"), which is also what it
# names the desktop shortcut it creates, and deletes again on uninstall. Falls
# back to the directory's own name -- the same string, unless Slicer was
# installed into a directory of the user's choosing.
#
# Property access goes through PSObject.Properties because Set-StrictMode turns a
# missing registry value into a terminating error rather than $null.
function Get-ShortcutName {
    param([string]$Root)
    $keys = @(
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*'
    )
    $want = $Root.TrimEnd('\')
    foreach ($entry in @(Get-ItemProperty $keys -ErrorAction SilentlyContinue)) {
        if (-not $entry.PSObject.Properties['DisplayName']) { continue }

        $dirs = @()
        if ($entry.PSObject.Properties['InstallLocation']) {
            $dirs += ([string]$entry.InstallLocation).Trim('"').TrimEnd('\')
        }
        if ($entry.PSObject.Properties['UninstallString']) {
            $cmd = [string]$entry.UninstallString
            if ($cmd -match '^\s*"([^"]+)"' -or $cmd -match '^\s*(\S+)') { $dirs += Split-Path -Parent $Matches[1] }
        }
        # -eq on strings ignores case, as Windows paths do.
        if ($dirs -contains $want) {
            $name = [string]$entry.DisplayName
            foreach ($c in [IO.Path]::GetInvalidFileNameChars()) { $name = $name.Replace([string]$c, '_') }
            if ($name) { return $name }
        }
    }
    return Split-Path -Leaf $want
}

# The desktop shortcuts that launch $Exe, whatever they are called.
function Get-DesktopShortcuts {
    param([string]$Folder, [string]$Exe)
    $shell = New-Object -ComObject WScript.Shell
    Get-ChildItem -LiteralPath $Folder -Filter '*.lnk' -File -ErrorAction SilentlyContinue |
        Where-Object { $shell.CreateShortcut($_.FullName).TargetPath -eq $Exe }
}

function Add-DesktopShortcut {
    param([string]$Folder, [string]$Exe)

    $existing = @(Get-DesktopShortcuts -Folder $Folder -Exe $Exe)
    if ($existing.Count -gt 0) {
        Write-Step "Desktop shortcut already there: $($existing[0].FullName)"
        return
    }

    $root = Split-Path -Parent $Exe
    $lnk = Join-Path $Folder ((Get-ShortcutName -Root $root) + '.lnk')
    $shortcut = (New-Object -ComObject WScript.Shell).CreateShortcut($lnk)
    $shortcut.TargetPath = $Exe
    $shortcut.WorkingDirectory = $root
    $shortcut.IconLocation = "$Exe,0"
    $shortcut.Description = '3D Slicer'
    $shortcut.Save()
    Write-Step "Desktop shortcut created: $lnk"
}

function Remove-DesktopShortcut {
    param([string]$Folder, [string]$Exe)
    foreach ($lnk in @(Get-DesktopShortcuts -Folder $Folder -Exe $Exe)) {
        Remove-Item -LiteralPath $lnk.FullName -Force
        Write-Step "Desktop shortcut removed: $($lnk.FullName)"
    }
}

# Windows offers programs no way to pin themselves to the taskbar: the shell
# verb that used to do it was withdrawn in Windows 10 version 1809, and the API
# Windows 11 added is limited to packaged (Store) apps. Pinning is the user's
# call, so say how.
function Show-TaskbarSteps {
    if ($Remove) {
        Write-Step 'To unpin 3D Slicer, right-click its taskbar icon and choose "Unpin from taskbar".'
        return
    }
    Write-Step 'Windows lets only you pin apps to the taskbar. To pin 3D Slicer:'
    Write-Note '1. Open the Start menu and find Slicer, or start Slicer and find its taskbar icon.'
    Write-Note '2. Right-click it and choose "Pin to taskbar".'
}

# ---------------------------------------------------------------------------- #
# main
# ---------------------------------------------------------------------------- #
if ((-not $Desktop) -and (-not $Taskbar)) {
    $Desktop = $true
    $Taskbar = $true
}

$exe = if ($SlicerExe) { $SlicerExe } else { Find-Slicer }
if ((-not $exe) -or (-not (Test-Path -LiteralPath $exe -PathType Leaf))) {
    Write-Error 'Could not find Slicer.exe next to this script; pass -SlicerExe <path> to point at it.'
    exit 1
}
# Shortcuts record full paths, so compare against one.
$exe = (Resolve-Path -LiteralPath $exe).ProviderPath

if ($Desktop) {
    # The Desktop special folder, not $HOME\Desktop: OneDrive and folder
    # redirection move it elsewhere.
    $folder = [Environment]::GetFolderPath('Desktop')
    if ((-not $folder) -or (-not (Test-Path -LiteralPath $folder -PathType Container))) {
        Write-Warning 'No desktop folder found; skipping the desktop shortcut.'
    } elseif ($Remove) {
        Remove-DesktopShortcut -Folder $folder -Exe $exe
    } else {
        Add-DesktopShortcut -Folder $folder -Exe $exe
    }
}

if ($Taskbar) {
    Show-TaskbarSteps
}
