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
$qtBin = Join-Path $resolvedQtPath "bin"
# Prefer Qt binaries; drop Conda/Anaconda Library\bin entries that often shadow Qt DLLs
# and cause ACCESS_VIOLATION (0xC0000005) on startup under `(base)` shells.
$pathParts = @($qtBin) + @(
    ($env:PATH -split ';' | Where-Object {
        $_ -and
        ($_ -ne $qtBin) -and
        ($_ -notmatch '(?i)[\\/](ana)?conda[0-9]*[\\/].*[\\/]Library[\\/]bin$') -and
        ($_ -notmatch '(?i)[\\/](ana)?conda[0-9]*[\\/].*[\\/]Library[\\/]mingw-w64[\\/]bin$')
    })
)
$env:PATH = ($pathParts -join ';')
$env:QT_PLUGIN_PATH = Join-Path $resolvedQtPath "plugins"
$env:QML_IMPORT_PATH = Join-Path $resolvedQtPath "qml"
$env:CMAKE_PREFIX_PATH = $resolvedQtPath

$exe = Join-Path $repoRoot "$BuildDir\bin\$Config\MixingStudio.exe"
$exeDir = Split-Path -Parent $exe

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
Write-Host "Working dir: $exeDir"
# Start from the exe directory (samples/ next to the binary). Do not inherit a
# Conda-first PATH — that commonly crashes Qt with 0xC0000005 under `(base)`.
$process = Start-Process -FilePath $exe -WorkingDirectory $exeDir -PassThru
Start-Sleep -Seconds 2
if ($null -eq $process) {
    throw "Failed to start MixingStudio."
}
if ($process.HasExited) {
    throw "MixingStudio exited immediately (code $($process.ExitCode)). Check Qt PATH / close other instances and retry."
}
Write-Host "MixingStudio is running (pid $($process.Id))." -ForegroundColor Green
exit 0
