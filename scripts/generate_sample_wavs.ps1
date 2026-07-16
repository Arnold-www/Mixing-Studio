#Requires -Version 5.1
<#
.SYNOPSIS
    Generate short synthetic PCM WAV sample assets under samples/.
    Prefer scripts/fetch_sample_assets.ps1 for real CC0 clips (see samples/ATTRIBUTION.md).
#>
param(
    [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutDir)) {
    $OutDir = Join-Path $repoRoot "samples"
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

function Write-Pcm16Wav {
    param(
        [string]$Path,
        [int]$SampleRate = 44100,
        [float[]]$MonoSamples
    )

    $channels = 2
    $bits = 16
    $frameCount = $MonoSamples.Length
    $dataBytes = $frameCount * $channels * 2
    $byteRate = $SampleRate * $channels * 2
    $blockAlign = $channels * 2

    $ms = New-Object System.IO.MemoryStream
    $bw = New-Object System.IO.BinaryWriter $ms

    function Write-Ascii([string]$s) {
        $bw.Write([System.Text.Encoding]::ASCII.GetBytes($s))
    }
    function Write-U32([uint32]$v) { $bw.Write([BitConverter]::GetBytes($v)) }
    function Write-U16([uint16]$v) { $bw.Write([BitConverter]::GetBytes($v)) }

    Write-Ascii "RIFF"
    Write-U32 ([uint32](36 + $dataBytes))
    Write-Ascii "WAVE"
    Write-Ascii "fmt "
    Write-U32 16
    Write-U16 1
    Write-U16 ([uint16]$channels)
    Write-U32 ([uint32]$SampleRate)
    Write-U32 ([uint32]$byteRate)
    Write-U16 ([uint16]$blockAlign)
    Write-U16 ([uint16]$bits)
    Write-Ascii "data"
    Write-U32 ([uint32]$dataBytes)

    foreach ($s in $MonoSamples) {
        $clamped = [Math]::Max(-1.0, [Math]::Min(1.0, [double]$s))
        $pcm = if ($clamped -lt 0) { [int][Math]::Round($clamped * 32768.0) } else { [int][Math]::Round($clamped * 32767.0) }
        if ($pcm -gt 32767) { $pcm = 32767 }
        if ($pcm -lt -32768) { $pcm = -32768 }
        $bw.Write([int16]$pcm)
        $bw.Write([int16]$pcm)
    }

    $bw.Flush()
    [System.IO.File]::WriteAllBytes($Path, $ms.ToArray())
    $bw.Dispose()
    $ms.Dispose()
}

function New-Tone {
    param([int]$SampleRate, [double]$Seconds, [double]$Freq, [double]$Amp = 0.35, [double]$Decay = 0.0)
    $n = [int]($SampleRate * $Seconds)
    $samples = New-Object 'float[]' $n
    for ($i = 0; $i -lt $n; $i++) {
        $t = $i / [double]$SampleRate
        $env = if ($Decay -gt 0) { [Math]::Exp(-$Decay * $t) } else { 1.0 }
        $samples[$i] = [float]($Amp * $env * [Math]::Sin(2.0 * [Math]::PI * $Freq * $t))
    }
    return ,$samples
}

function New-NoiseBurst {
    param([int]$SampleRate, [double]$Seconds, [double]$Amp = 0.25, [double]$Decay = 12.0)
    $n = [int]($SampleRate * $Seconds)
    $samples = New-Object 'float[]' $n
    $rnd = New-Object System.Random 42
    for ($i = 0; $i -lt $n; $i++) {
        $t = $i / [double]$SampleRate
        $env = [Math]::Exp(-$Decay * $t)
        $noise = ($rnd.NextDouble() * 2.0 - 1.0)
        $samples[$i] = [float]($Amp * $env * $noise)
    }
    return ,$samples
}

function New-Kick {
    param([int]$SampleRate = 44100)
    $n = [int]($SampleRate * 0.45)
    $samples = New-Object 'float[]' $n
    for ($i = 0; $i -lt $n; $i++) {
        $t = $i / [double]$SampleRate
        $freq = 150.0 * [Math]::Exp(-18.0 * $t) + 40.0
        $env = [Math]::Exp(-6.0 * $t)
        $samples[$i] = [float](0.7 * $env * [Math]::Sin(2.0 * [Math]::PI * $freq * $t))
    }
    return ,$samples
}

$sr = 44100
$assets = @{
    "kick.wav"       = (New-Kick -SampleRate $sr)
    "snare.wav"      = (New-NoiseBurst -SampleRate $sr -Seconds 0.35 -Amp 0.45 -Decay 14.0)
    "hihat.wav"      = (New-NoiseBurst -SampleRate $sr -Seconds 0.18 -Amp 0.22 -Decay 28.0)
    "bass_110.wav"   = (New-Tone -SampleRate $sr -Seconds 1.5 -Freq 110 -Amp 0.4 -Decay 1.2)
    "pad_440.wav"    = (New-Tone -SampleRate $sr -Seconds 2.0 -Freq 440 -Amp 0.22 -Decay 0.4)
    "lead_880.wav"   = (New-Tone -SampleRate $sr -Seconds 1.2 -Freq 880 -Amp 0.28 -Decay 1.5)
    "noise_perc.wav" = (New-NoiseBurst -SampleRate $sr -Seconds 0.5 -Amp 0.3 -Decay 8.0)
    "demo_tone.wav"  = (New-Tone -SampleRate $sr -Seconds 1.0 -Freq 440 -Amp 0.3 -Decay 0.0)
}

foreach ($name in $assets.Keys) {
    $path = Join-Path $OutDir $name
    Write-Pcm16Wav -Path $path -SampleRate $sr -MonoSamples $assets[$name]
    Write-Host "Wrote $path"
}

Write-Host "Generated $($assets.Count) WAV files in $OutDir"
