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
    -Detail "Headers should live under include/Common, Model, DSP, App, Command, ViewModel."

$requiredHeaders = @(
    "include/Common/ICommandBase.h",
    "include/Common/MixerTypes.h",
    "include/DSP/DspProcessor.h",
    "include/DSP/DspAnalysis.h",
    "include/Model/AudioEngine.h",
    "include/Model/AudioTrack.h",
    "include/App/MixingStudioApp.h",
    "include/Command/ICommand.h",
    "include/Command/PlaybackCommands.h",
    "include/Command/ProjectCommands.h",
    "include/Command/TrackDspCommands.h",
    "include/Command/MixerCommands.h",
    "include/ViewModel/IMixerViewModel.h",
    "include/ViewModel/ITrackViewModel.h",
    "include/ViewModel/RealMixerViewModel.h",
    "include/ViewModel/TrackViewModel.h"
)

foreach ($header in $requiredHeaders) {
    $results += Add-Result `
        -Name "Required header exists: $header" `
        -Passed (Test-PathExists $header) `
        -Detail $header
}

$results += Add-Result `
    -Name "Legacy MixerApp facade removed" `
    -Passed (-not (Test-PathExists "include/App/MixerApp.h")) `
    -Detail "include/App/MixerApp.h should not exist"

$qmlFiles = @(Get-ChildItem -Path "src/View" -Recurse -Include "*.qml" -ErrorAction SilentlyContinue)
$qmlBoundaryViolations = @()
foreach ($file in $qmlFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "AudioEngine|DspProcessor|MixerApp|RealMixerViewModel|ICommand|PlayCommand|src/Model|src/DSP|src/App|src/Command") {
        $qmlBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "QML does not access Model/DSP/App/Command/RealVM" `
    -Passed ($qmlBoundaryViolations.Count -eq 0) `
    -Detail (($qmlBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$modelDspFiles = @()
$modelDspFiles += Get-ChildItem -Path "src/Model" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspFiles += Get-ChildItem -Path "src/DSP" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspBoundaryViolations = @()
foreach ($file in $modelDspFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "ViewModel|MixingStudioApp|ICommand|QQml|QQuick|Main.qml|src/View|src/App|src/Command") {
        $modelDspBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "Model/DSP do not depend on App/Command/View/ViewModel" `
    -Passed ($modelDspBoundaryViolations.Count -eq 0) `
    -Detail (($modelDspBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$mainContent = ""
if (Test-PathExists "src/main.cpp") {
    $mainContent = Get-Content -LiteralPath "src/main.cpp" -Raw
}
$appContent = ""
if (Test-PathExists "src/App/MixingStudioApp.cpp") {
    $appContent = Get-Content -LiteralPath "src/App/MixingStudioApp.cpp" -Raw
}
$injectOk = ($appContent -match "IMixerViewModel") -and ($appContent -match "setContextProperty") -and ($appContent -notmatch "setContextProperty\([^\)]*RealMixerViewModel")
$results += Add-Result `
    -Name "App injects IMixerViewModel interface pointer" `
    -Passed $injectOk `
    -Detail "MixingStudioApp must setContextProperty with IMixerViewModel*"

$commandFiles = @()
$commandFiles += Get-ChildItem -Path "src/Command" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$commandBoundaryViolations = @()
foreach ($file in $commandFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "ViewModel|QQml|QQuick|Main.qml|src/View") {
        $commandBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "Command does not depend on View/ViewModel" `
    -Passed ($commandBoundaryViolations.Count -eq 0) `
    -Detail (($commandBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

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
