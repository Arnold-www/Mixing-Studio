#Requires -Version 5.1
<#
.SYNOPSIS
    One-click configure, build, and run CTest targets.
    Qt is auto-detected; -QtPath is optional.
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

. (Join-Path $PSScriptRoot "Resolve-QtPath.ps1")

function Invoke-Step {
    param([string]$Title, [scriptblock]$Action)
    Write-Host "`n==> $Title" -ForegroundColor Cyan
    & $Action
    if ($LASTEXITCODE -ne 0) { throw "$Title failed with exit code $LASTEXITCODE" }
}

$resolvedQtPath = Resolve-QtPath -PreferredPath $QtPath
$env:PATH = "$(Join-Path $resolvedQtPath 'bin');$env:PATH"
$env:CMAKE_PREFIX_PATH = $resolvedQtPath

Write-Host "Test runner"
Write-Host "Repository : $repoRoot"
Write-Host "Qt Path    : $resolvedQtPath"

if (-not $SkipConfigure) {
    Invoke-Step "Configure CMake" { cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$resolvedQtPath" }
}

if ($WithApp) {
    Invoke-Step "Build app + tests" { cmake --build $BuildDir --config $Config }
} else {
    Invoke-Step "Build tests" {
        cmake --build $BuildDir --config $Config --target test_dsp_processor
        cmake --build $BuildDir --config $Config --target test_audio_engine
        cmake --build $BuildDir --config $Config --target test_project_store
        cmake --build $BuildDir --config $Config --target test_asset_library
        cmake --build $BuildDir --config $Config --target test_common_types
        cmake --build $BuildDir --config $Config --target test_viewmodel_commands
        cmake --build $BuildDir --config $Config --target test_ui_binder
        cmake --build $BuildDir --config $Config --target test_wav_export
        cmake --build $BuildDir --config $Config --target test_wav_decoder
        cmake --build $BuildDir --config $Config --target test_automation
        cmake --build $BuildDir --config $Config --target test_audio_pipeline_e2e
    }
}

Invoke-Step "Run CTest" { ctest --test-dir $BuildDir -C $Config --output-on-failure }

Write-Host "`nAll tests passed." -ForegroundColor Green
