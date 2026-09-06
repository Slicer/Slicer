# Slicer Uninstall Guard
#
# The uninstallers of 3D Slicer releases from 5.9.0-2025-09-18 up to
# 5.13.0-2026-09-04 recursively delete the entire HKCU\Software\Classes
# registry key, destroying the file associations of all applications of the
# current user (see https://github.com/Slicer/Slicer/issues/9383).
#
# Registry key deletion stops at the first error, therefore a key that sorts
# before all other keys ("!" sorts first) and that cannot be deleted prevents
# the damage. This script creates such a key and denies its deletion to everyone.
#
# The script is run by the Windows installer. It is idempotent and can also be
# run manually. The key is intentionally left in place when the application is
# uninstalled, since it protects against the uninstallers of other (older)
# Slicer installations.
#
# Status of the guard can be checked with:
#   (Get-Acl 'HKCU:\Software\Classes\!SlicerUninstallGuard').Access | Where-Object { $_.AccessControlType -eq 'Deny' }
#
# The guard can be removed (not recommended) with:
#   $k = 'HKCU:\Software\Classes\!SlicerUninstallGuard'; $a = Get-Acl $k
#   @($a.Access | Where-Object { $_.AccessControlType -eq 'Deny' }) | ForEach-Object { [void]$a.RemoveAccessRule($_) }
#   Set-Acl $k $a; Remove-Item $k -Recurse -Force

$ErrorActionPreference = 'Stop'

$key = 'HKCU:\Software\Classes\!SlicerUninstallGuard'
$deleteRight = [int][System.Security.AccessControl.RegistryRights]::Delete

try {
  $created = $false
  if (-not (Test-Path $key)) {
    New-Item $key -Force | Out-Null
    Set-ItemProperty $key '(default)' 'Guard against Slicer bug github.com/Slicer/Slicer/issues/9383'
    $created = $true
  }

  $acl = Get-Acl $key
  $hasDenyDelete = @($acl.Access | Where-Object {
      $_.AccessControlType -eq 'Deny' -and (([int]$_.RegistryRights -band $deleteRight) -ne 0)
    }).Count -gt 0
  if (-not $hasDenyDelete) {
    $everyone = New-Object System.Security.Principal.SecurityIdentifier 'S-1-1-0'
    $rule = New-Object System.Security.AccessControl.RegistryAccessRule($everyone, 'Delete', 'Deny')
    $acl.AddAccessRule($rule)
    Set-Acl $key $acl
  }

  # Verify that the key cannot be deleted
  $deleted = $false
  try {
    Remove-Item $key -Force -ErrorAction Stop
    $deleted = $true
  } catch {
    $deleted = $false
  }
  if ($deleted) {
    Write-Output 'FAILED: Slicer Uninstall Guard is not effective (the registry key could be deleted)'
    exit 1
  }
  if ($created -or -not $hasDenyDelete) {
    Write-Output 'Slicer Uninstall Guard added and verified'
  } else {
    Write-Output 'Slicer Uninstall Guard already present'
  }
  exit 0
}
catch {
  Write-Output "FAILED: Slicer Uninstall Guard could not be added: $($_.Exception.Message)"
  exit 1
}
