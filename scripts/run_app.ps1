#Requires -Version 5.1
<#
.SYNOPSIS
    Auto-detect Qt, configure/build MixingStudio if needed, then launch with PATH set.
.DESCRIPTION
    Zero-arg usage: .\scripts\run_app.ps1
    No need to pass -QtPath or CMAKE_PREFIX_PATH.
#>
[CmdletBinding()]
param(
    [string]$QtPath = $env:QT_PATH,
    [string]$BuildDir = "build",
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [switch]$SkipBuild,
    [switch]$Deploy
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

. (Join-Path $PSScriptRoot "Resolve-QtPath.ps1")

$resolvedQtPath = Resolve-QtPath -PreferredPath $QtPath
$env:PATH = "$(Join-Path $resolvedQtPath 'bin');$env:PATH"
$env:CMAKE_PREFIX_PATH = $resolvedQtPath

$exe = Join-Path $repoRoot "$BuildDir\bin\$Config\MixingStudio.exe"

Write-Host "Mixing Studio launcher"
Write-Host "Repository : $repoRoot"
Write-Host "Qt Path    : $resolvedQtPath"

if (-not $SkipBuild) {
    Write-Host "`n==> Configure CMake" -ForegroundColor Cyan
    cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$resolvedQtPath"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }

    Write-Host "`n==> Build MixingStudio" -ForegroundColor Cyan
    cmake --build $BuildDir --config $Config --target MixingStudio
    if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE" }
}

if (-not (Test-Path -LiteralPath $exe)) {
    throw "Executable not found: $exe`nRun without -SkipBuild, or run .\scripts\configure_qt_windows.ps1 first."
}

if ($Deploy) {
    $windeploy = Join-Path $resolvedQtPath "bin\windeployqt.exe"
    if (-not (Test-Path -LiteralPath $windeploy)) {
        throw "windeployqt not found: $windeploy"
    }
    Write-Host "`n==> Deploy Qt runtime next to exe" -ForegroundColor Cyan
    & $windeploy --qmldir (Join-Path $repoRoot "src\View") $exe
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE" }
}

Write-Host "`n==> Launch $exe" -ForegroundColor Cyan
& $exe
exit $LASTEXITCODE
