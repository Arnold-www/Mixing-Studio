<#
.SYNOPSIS
    Download CC0 demo WAV assets into samples/ (replaces synthetic tones).

.DESCRIPTION
    Fetches short clips from BushDrum and stargate-sample-pack via jsDelivr.
    See samples/ATTRIBUTION.md for licenses and mapping.
#>
[CmdletBinding()]
param(
    [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutDir)) {
    $OutDir = Join-Path $repoRoot "samples"
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$cdnBush = "https://cdn.jsdelivr.net/gh/EwonRael/BushDrum@main"
$cdnSg = "https://cdn.jsdelivr.net/gh/stargatedaw/stargate-sample-pack@main/stargate-sample-pack"

# localName -> @{ Url; ExpectedBytes }
$assets = [ordered]@{
    "kick.wav" = @{
        Url = "$cdnBush/kick.wav"
        ExpectedBytes = 18090
    }
    "snare.wav" = @{
        Url = "$cdnBush/snare-m.wav"
        ExpectedBytes = 12444
    }
    "hihat.wav" = @{
        Url = "$cdnBush/hihat-closed.wav"
        ExpectedBytes = 10230
    }
    "noise_perc.wav" = @{
        Url = "$cdnBush/cabasa.wav"
        ExpectedBytes = 7174
    }
    "bass_110.wav" = @{
        Url = "$cdnSg/microlag/Loops/Bass_A_Major_72BPM.wav"
        ExpectedBytes = 1764736
    }
    "pad_440.wav" = @{
        Url = "$cdnSg/microlag/Loops/Ambience_A_85BPM.wav"
        ExpectedBytes = 1494946
    }
    "lead_880.wav" = @{
        Url = "$cdnSg/microlag/Loops/Melody_B_Minor_73BPM.wav"
        ExpectedBytes = 1740574
    }
    "demo_tone.wav" = @{
        Url = "$cdnSg/fugue-state-audio/loops/130-discobeat.wav"
        ExpectedBytes = 977912
    }
}

function Test-RiffWave([string]$Path) {
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 12) { return $false }
    $riff = [System.Text.Encoding]::ASCII.GetString($bytes, 0, 4)
    $wave = [System.Text.Encoding]::ASCII.GetString($bytes, 8, 4)
    return ($riff -eq "RIFF" -and $wave -eq "WAVE")
}

foreach ($name in $assets.Keys) {
    $meta = $assets[$name]
    $out = Join-Path $OutDir $name
    Write-Host "GET $name"
    & curl.exe -L --retry 3 --retry-delay 2 --connect-timeout 20 --max-time 180 `
        -A "MixingStudio-fetch_sample_assets" -o $out $meta.Url
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $out)) {
        throw "Download failed: $name"
    }
    $len = (Get-Item $out).Length
    if ($len -ne $meta.ExpectedBytes) {
        throw "Size mismatch for $name : got $len expected $($meta.ExpectedBytes)"
    }
    if (-not (Test-RiffWave $out)) {
        throw "Not a RIFF/WAVE file: $name"
    }
    Write-Host "OK $name ($len bytes)"
}

Write-Host "Done. Wrote $($assets.Count) WAV files to $OutDir"
Write-Host "See samples/ATTRIBUTION.md for license notes."
