#Requires -Version 5.1
<#
.SYNOPSIS
    Sprint 1 one-click configure, build, and run DSP unit tests.
#>
[CmdletBinding()]
param(
    [string]$QtPath = $env:QT_PATH,
    [string]$BuildDir = "build",
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [switch]$SkipConfigure,
    [switch]$WithApp
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

function Resolve-QtPath {
    param([string]$PreferredPath)
    $candidates = @()
    if ($PreferredPath) { $candidates += $PreferredPath }
    if ($env:CMAKE_PREFIX_PATH) { $candidates += ($env:CMAKE_PREFIX_PATH -split ';' | Where-Object { $_ }) }
    $candidates += @(
        "D:\Qt\6.5.3\msvc2019_64",
        "D:\Qt\6.5.3\msvc2022_64",
        "C:\Qt\6.5.3\msvc2019_64"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path (Join-Path $candidate "bin\Qt6Core.dll")) {
            return (Resolve-Path $candidate).Path
        }
    }
    throw "Qt 6 not found. Pass -QtPath or set QT_PATH."
}

function Invoke-Step {
    param([string]$Title, [scriptblock]$Action)
    Write-Host "`n==> $Title" -ForegroundColor Cyan
    & $Action
    if ($LASTEXITCODE -ne 0) { throw "$Title failed with exit code $LASTEXITCODE" }
}

$resolvedQtPath = Resolve-QtPath -PreferredPath $QtPath
$env:PATH = "$(Join-Path $resolvedQtPath 'bin');$env:PATH"
$env:CMAKE_PREFIX_PATH = $resolvedQtPath

Write-Host "Sprint 1 test runner"
Write-Host "Repository : $repoRoot"
Write-Host "Qt Path    : $resolvedQtPath"

if (-not $SkipConfigure) {
    Invoke-Step "Configure CMake" { cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$resolvedQtPath" }
}

if ($WithApp) {
    Invoke-Step "Build app + tests" { cmake --build $BuildDir --config $Config }
} else {
    Invoke-Step "Build DSP test" { cmake --build $BuildDir --config $Config --target test_dsp_processor }
}

Invoke-Step "Run CTest (dsp_processor)" { ctest --test-dir $BuildDir -C $Config --output-on-failure -R dsp_processor }

Write-Host "`nSprint 1 DSP tests passed." -ForegroundColor Green
