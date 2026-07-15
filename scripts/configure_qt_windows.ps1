#Requires -Version 5.1
<#
.SYNOPSIS
    Configure and build with auto-detected Qt (no path required).
#>
param(
    [string]$QtPrefix = "",
    [string]$BuildDir = "build",
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
Set-Location $repoRoot

. (Join-Path $PSScriptRoot "Resolve-QtPath.ps1")

try {
    $QtPrefix = Resolve-QtPath -PreferredPath $QtPrefix
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}

Write-Host "Using Qt: $QtPrefix"
$env:PATH = "$(Join-Path $QtPrefix 'bin');$env:PATH"
$env:CMAKE_PREFIX_PATH = $QtPrefix

cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$QtPrefix"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Qt build completed: $BuildDir ($Config)" -ForegroundColor Green
