#Requires -Version 5.1
<#
.SYNOPSIS
    Resolve a local Qt 6 MSVC kit without requiring -QtPath on the command line.
#>

function Resolve-QtPath {
    [CmdletBinding()]
    param(
        [string]$PreferredPath
    )

    $candidates = @()
    if ($PreferredPath) { $candidates += $PreferredPath }
    if ($env:QT_PATH) { $candidates += $env:QT_PATH }
    if ($env:CMAKE_PREFIX_PATH) {
        $candidates += ($env:CMAKE_PREFIX_PATH -split ';' | Where-Object { $_ })
    }
    $candidates += @(
        "D:\Qt\6.8.3\msvc2022_64",
        "D:\Qt\6.8.3\msvc2019_64",
        "C:\Qt\6.8.3\msvc2022_64"
    )

    foreach ($root in @("D:\Qt", "C:\Qt")) {
        if (Test-Path -LiteralPath $root) {
            Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue | ForEach-Object {
                Get-ChildItem -LiteralPath $_.FullName -Directory -Filter "msvc*_64" -ErrorAction SilentlyContinue | ForEach-Object {
                    $candidates += $_.FullName
                }
            }
        }
    }

    $seen = @{}
    foreach ($candidate in $candidates) {
        if (-not $candidate -or $seen.ContainsKey($candidate)) { continue }
        $seen[$candidate] = $true
        $coreDll = Join-Path $candidate "bin\Qt6Core.dll"
        if (Test-Path -LiteralPath $coreDll) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw @"
Qt 6 not found. Install with:
  python -m pip install --user aqtinstall
  python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O "D:\Qt"
Or set QT_PATH / pass -QtPath.
"@
}
