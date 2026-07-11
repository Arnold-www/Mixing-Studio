param(
    [string]$QtPrefix = "D:\Qt\6.5.3\msvc2019_64",
    [string]$BuildDir = "build",
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
Set-Location $repoRoot

if (-not (Test-Path -LiteralPath $QtPrefix)) {
    Write-Host "Qt prefix not found: $QtPrefix" -ForegroundColor Red
    Write-Host "Install Qt with:" -ForegroundColor Yellow
    Write-Host 'python -m pip install --user aqtinstall'
    Write-Host 'python -m aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -O "D:\Qt"'
    exit 1
}

cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$QtPrefix"
cmake --build $BuildDir --config $Config

Write-Host "Qt build completed: $BuildDir ($Config)" -ForegroundColor Green
