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
    -Detail "Headers should live under include/Common, Model, DSP, App, ViewModel."

$requiredHeaders = @(
    "include/Common/MixerTypes.h",
    "include/DSP/DspProcessor.h",
    "include/DSP/DspAnalysis.h",
    "include/Model/AudioEngine.h",
    "include/Model/AudioTrack.h",
    "include/Model/WavExporter.h",
    "include/Model/WavDecoder.h",
    "include/Model/AudioFileDecoder.h",
    "include/App/MixingStudioApp.h",
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

$legacyCommandFiles = @(
    "include/Command/ICommand.h",
    "include/Command/PlaybackCommands.h",
    "include/Command/ProjectCommands.h",
    "include/Command/TrackDspCommands.h",
    "include/Command/MixerCommands.h",
    "include/Common/ICommandBase.h",
    "src/Command/PlaybackCommands.cpp",
    "src/Command/ProjectCommands.cpp",
    "src/Command/TrackDspCommands.cpp"
)
$legacyCommandPresent = @($legacyCommandFiles | Where-Object { Test-PathExists $_ })
$results += Add-Result `
    -Name "Legacy Command layer removed" `
    -Passed ($legacyCommandPresent.Count -eq 0) `
    -Detail ($(if ($legacyCommandPresent.Count -eq 0) { "Command layer replaced by ViewModel slots" } else { ($legacyCommandPresent -join "; ") }))

$qmlFiles = @(Get-ChildItem -Path "src/View" -Recurse -Include "*.qml" -ErrorAction SilentlyContinue)
$qmlBoundaryViolations = @()
foreach ($file in $qmlFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -cmatch "AudioEngine|DspProcessor|MixerApp|RealMixerViewModel|\bICommand\b|\bPlayCommand\b|src/Model|src/DSP|src/App|src/Command") {
        $qmlBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "QML does not access Model/DSP/App/RealVM" `
    -Passed ($qmlBoundaryViolations.Count -eq 0) `
    -Detail (($qmlBoundaryViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$pureViewViolations = @()
foreach ($file in $qmlFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -cmatch "mixerViewModel") {
        $pureViewViolations += $file.FullName
    }
}
$results += Add-Result `
    -Name "All QML Views do not reference mixerViewModel" `
    -Passed ($pureViewViolations.Count -eq 0) `
    -Detail ($(if ($pureViewViolations.Count -eq 0) { "MixingStudioViewBinder wires pure View root" } else { ($pureViewViolations -join "; ") -replace [regex]::Escape($repoRoot), "." }))

$modelDspFiles = @()
$modelDspFiles += Get-ChildItem -Path "src/Model" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspFiles += Get-ChildItem -Path "src/DSP" -Recurse -Include "*.cpp" -ErrorAction SilentlyContinue
$modelDspBoundaryViolations = @()
foreach ($file in $modelDspFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -cmatch '#include\s*[<"][^>"]*(ViewModel|MixingStudioApp|ICommand|QQml|QQuick)|Main\.qml|src/View|src/App|src/Command') {
        $modelDspBoundaryViolations += $file.FullName
    }
}

$results += Add-Result `
    -Name "Model/DSP do not depend on App/View/ViewModel" `
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
$injectOk = ($appContent -match "RealMixerViewModel") `
    -and ($appContent -match "MixingStudioViewBinder") `
    -and ($appContent -match "\.bind\(") `
    -and ($appContent -notmatch "setContextProperty") `
    -and ($appContent -notmatch "setInitialProperties") `
    -and ($appContent -notmatch "bindView")
$results += Add-Result `
    -Name "App loads View and ViewBinder wires ViewModel" `
    -Passed $injectOk `
    -Detail "App creates VM + load QML + MixingStudioViewBinder.bind(root); no ContextProperty / setInitialProperties / VM.bindView"

$vmContent = ""
if (Test-PathExists "src/ViewModel/RealMixerViewModel.cpp") {
    $vmContent = Get-Content -LiteralPath "src/ViewModel/RealMixerViewModel.cpp" -Raw
}
$viewModelHeader = ""
if (Test-PathExists "include/ViewModel/RealMixerViewModel.h") {
    $viewModelHeader = Get-Content -LiteralPath "include/ViewModel/RealMixerViewModel.h" -Raw
}
$vmNoViewOk = ($viewModelHeader -notmatch "bindView") `
    -and ($vmContent -notmatch "m_viewRoot|syncRoot|bindRootSignals|setRootProp") `
    -and ($vmContent -notmatch "findChild") `
    -and ($vmContent -notmatch "QQuickItem") `
    -and ($viewModelHeader -notmatch "QQuickItem|m_viewRoot|bindView")
$results += Add-Result `
    -Name "ViewModel has no View root wiring (no bindView / m_viewRoot)" `
    -Passed $vmNoViewOk `
    -Detail "Connect/sync lives in MixingStudioViewBinder only"

$binderGone = (-not (Test-PathExists "include/App/MixerViewBinder.h")) `
    -and (-not (Test-PathExists "src/App/MixerViewBinder.cpp"))
$appHeader = ""
if (Test-PathExists "include/App/MixingStudioApp.h") {
    $appHeader = Get-Content -LiteralPath "include/App/MixingStudioApp.h" -Raw
}
$binderInAppOk = $binderGone `
    -and ($appHeader -match "class MixingStudioViewBinder") `
    -and ($appContent -match "syncRoot|setProperty") `
    -and ($appContent -notmatch "findChild") `
    -and ($appContent -notmatch "QQuickItem")
$results += Add-Result `
    -Name "App ViewBinder lives in MixingStudioApp (no separate binder file)" `
    -Passed $binderInAppOk `
    -Detail "MixingStudioViewBinder in MixingStudioApp.h/.cpp; MixerViewBinder.* must not exist"

$vmGuiViolations = @()
$vmFiles = @()
$vmFiles += Get-ChildItem -Path "include/ViewModel" -Recurse -Include "*.h" -ErrorAction SilentlyContinue
$vmFiles += Get-ChildItem -Path "src/ViewModel" -Recurse -Include "*.cpp","*.h" -ErrorAction SilentlyContinue
foreach ($file in $vmFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -cmatch "QQuickItem|findChild\s*<") {
        $vmGuiViolations += $file.FullName
    }
}
$results += Add-Result `
    -Name "ViewModel has no QQuickItem / findChild UI coupling" `
    -Passed ($vmGuiViolations.Count -eq 0) `
    -Detail (($vmGuiViolations -join "; ") -replace [regex]::Escape($repoRoot), ".")

$vmSlotsOk = ($viewModelHeader -match "void play\(\)") `
    -and ($viewModelHeader -match "void saveProject\(\)") `
    -and ($viewModelHeader -match "Q_OBJECT") `
    -and ($viewModelHeader -notmatch "IMixerViewModel")
$results += Add-Result `
    -Name "RealMixerViewModel exposes business slots (no I* interface)" `
    -Passed $vmSlotsOk `
    -Detail "Concrete RealMixerViewModel with slots; IMixer/ITrack removed"

$selectionOk = ($viewModelHeader -match "selectedAssetIndex") `
    -and ($viewModelHeader -match "selectedRecentProjectIndex")
$results += Add-Result `
    -Name "ViewModel exposes selection state" `
    -Passed $selectionOk `
    -Detail "selectedAssetIndex / selectedRecentProjectIndex"

$ifaceGone = (-not (Test-PathExists "include/ViewModel/IMixerViewModel.h")) `
    -and (-not (Test-PathExists "include/ViewModel/ITrackViewModel.h"))
$results += Add-Result `
    -Name "IMixerViewModel / ITrackViewModel removed" `
    -Passed $ifaceGone `
    -Detail "Interface headers must not exist; use concrete ViewModels"

$qmlCommandViolations = @()
foreach ($file in $qmlFiles) {
    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match "mixerViewModel\.(play|pause|stop|saveProject|exportMix|loadSampleProject|importMockTrack|importLocalFile|importAssetByName|clearAutomation|deleteSelectedTrack)\s*\(") {
        $qmlCommandViolations += $file.FullName
    }
}
$results += Add-Result `
    -Name "QML does not call ViewModel action slots" `
    -Passed ($qmlCommandViolations.Count -eq 0) `
    -Detail ($(if ($qmlCommandViolations.Count -eq 0) { "Root signals → Binder → ViewModel slots" } else { ($qmlCommandViolations -join "; ") -replace [regex]::Escape($repoRoot), "." }))

$uiCommandGone = (-not (Test-PathExists "include/ViewModel/UiCommand.h")) `
    -and (-not (Test-PathExists "src/ViewModel/UiCommand.cpp"))
$results += Add-Result `
    -Name "UiCommand layer removed" `
    -Passed $uiCommandGone `
    -Detail "include/ViewModel/UiCommand.h and src/ViewModel/UiCommand.cpp must not exist"

$mainQml = ""
if (Test-PathExists "src/View/Main.qml") {
    $mainQml = Get-Content -LiteralPath "src/View/Main.qml" -Raw
}
$mainRootApiOk = ($mainQml -match 'objectName:\s*"mainWindow"') `
    -and ($mainQml -match "signal playRequested") `
    -and ($mainQml -match "property bool playing")
$results += Add-Result `
    -Name "Main.qml root exposes property/signal API for Binder" `
    -Passed $mainRootApiOk `
    -Detail "Root objectName mainWindow + inbound properties + outbound signals"
$results += Add-Result `
    -Name "UI automation test exists" `
    -Passed ((Test-PathExists "tests/test_ui_binder.cpp") -and (Select-String -Path "CMakeLists.txt" -Pattern "ui_binder" -Quiet)) `
    -Detail "tests/test_ui_binder.cpp + CTest name ui_binder"

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

$sampleWavs = @(Get-ChildItem -Path "samples" -Filter "*.wav" -ErrorAction SilentlyContinue)
$results += Add-Result `
    -Name "Sample library has at least 8 WAV files" `
    -Passed ($sampleWavs.Count -ge 8) `
    -Detail ("Found {0} wav files under samples/" -f $sampleWavs.Count)

$results += Add-Result `
    -Name "Demo session project exists" `
    -Passed (Test-PathExists "samples/demo_session.json") `
    -Detail "samples/demo_session.json"

$failed = $results | Where-Object { -not $_.Passed }

$results | Format-Table -AutoSize

if ($failed.Count -gt 0) {
    Write-Host ""
    Write-Host "Feature validation failed: $($failed.Count) check(s)." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Feature validation passed: $($results.Count) check(s)." -ForegroundColor Green
