param(
    [string]$BaseBranch = "main",
    [string]$FeatureBranch = ""
)

$ErrorActionPreference = "Stop"

function Add-Result {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Detail
    )

    [PSCustomObject]@{
        Check  = $Name
        Passed = $Passed
        Detail = $Detail
    }
}

function Test-PathExists {
    param([string]$PathToCheck)
    return Test-Path -LiteralPath $PathToCheck
}

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
Set-Location $repoRoot

if ([string]::IsNullOrWhiteSpace($FeatureBranch)) {
    $FeatureBranch = (git branch --show-current).Trim()
}

$results = @()

$changedFiles = @(git diff --name-only "$BaseBranch...HEAD")

$results += Add-Result `
    -Name "Git branch is not main" `
    -Passed ($FeatureBranch -ne "main") `
    -Detail "Current branch: $FeatureBranch"

$results += Add-Result `
    -Name "No headers under src" `
    -Passed (-not (Get-ChildItem -Path "src" -Recurse -Filter "*.h" -ErrorAction SilentlyContinue)) `
    -Detail "Headers should live under include/DSP, include/Model, include/ViewModel."

$requiredHeaders = @(
    "include/DSP/DspProcessor.h",
    "include/Model/AudioEngine.h",
    "include/ViewModel/MixerViewModel.h",
    "include/ViewModel/TrackViewModel.h"
)

foreach ($header in $requiredHeaders) {
    $results += Add-Result `
        -Name "Required header exists: $header" `
        -Passed (Test-PathExists $header) `
        -Detail $header
}

$qmlFiles = @(Get-ChildItem -Path "src/View" -Recurse -Include "*.qml" -ErrorAction SilentlyContinue)
$qmlBoundaryViolations = @()
foreach ($file in $qmlFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "AudioEngine|DspProcessor|src/Model|src/DSP") {
        $qmlBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "QML does not directly access Model/DSP" `
    -Passed ($qmlBoundaryViolations.Count -eq 0) `
    -Detail (($qmlBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$modelDspFiles = @()
$modelDspFiles += Get-ChildItem -Path "src/Model" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspFiles += Get-ChildItem -Path "src/DSP" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspBoundaryViolations = @()
foreach ($file in $modelDspFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "ViewModel|QQml|QQuick|Main.qml|src/View") {
        $modelDspBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "Model/DSP do not depend on View/ViewModel" `
    -Passed ($modelDspBoundaryViolations.Count -eq 0) `
    -Detail (($modelDspBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$featureScopeViolations = @()
if ($FeatureBranch -match "chai/feat|feature/B|B-|vm|view") {
    $featureScopeViolations = $changedFiles | Where-Object {
        $_ -match "^(src/Model/|src/DSP/|include/Model/|include/DSP/)"
    }
}

$results += Add-Result `
    -Name "Feature branch stays within expected B-side scope" `
    -Passed ($featureScopeViolations.Count -eq 0) `
    -Detail (($featureScopeViolations -join "; "))

$requiredReportFiles = @(
    "report/shared/AI_USAGE_LOG.md",
    "report/shared/CROSS_TEST_LOG.md",
    "report/shared/TEST_AND_TOOLCHAIN.md"
)

foreach ($reportFile in $requiredReportFiles) {
    $results += Add-Result `
        -Name "Report evidence file exists: $reportFile" `
        -Passed (Test-PathExists $reportFile) `
        -Detail $reportFile
}

$cmakeContent = Get-Content -LiteralPath "CMakeLists.txt" -Raw
$results += Add-Result `
    -Name "CMake includes public header directory" `
    -Passed ($cmakeContent -match "target_include_directories" -and $cmakeContent -match "/include|\\include|include") `
    -Detail "CMakeLists.txt"

$failed = $results | Where-Object { -not $_.Passed }

$results | Format-Table -AutoSize

if ($failed.Count -gt 0) {
    Write-Host ""
    Write-Host "Feature validation failed: $($failed.Count) check(s)." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Feature validation passed: $($results.Count) check(s)." -ForegroundColor Green
