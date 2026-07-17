# Sample asset attribution

Bundled demo WAVs under `samples/` are **not** the synthetic tones from `scripts/generate_sample_wavs.ps1`.
They are short clips from public CC0 / public-domain packs, fetched by `scripts/fetch_sample_assets.ps1`.

| Local file | Source | Upstream file | License |
|---|---|---|---|
| `kick.wav` | [BushDrum](https://github.com/EwonRael/BushDrum) | `kick.wav` | CC0-1.0 |
| `snare.wav` | BushDrum | `snare-m.wav` | CC0-1.0 |
| `hihat.wav` | BushDrum | `hihat-closed.wav` | CC0-1.0 |
| `noise_perc.wav` | BushDrum | `cabasa.wav` | CC0-1.0 |
| `bass_110.wav` | [stargate-sample-pack](https://github.com/stargatedaw/stargate-sample-pack) (microlag) | `Loops/Bass_A_Major_72BPM.wav` | CC0 / public domain (see pack LICENSE) |
| `pad_440.wav` | stargate-sample-pack (microlag) | `Loops/Ambience_A_85BPM.wav` | same |
| `lead_880.wav` | stargate-sample-pack (microlag) | `Loops/Melody_B_Minor_73BPM.wav` | same |
| `demo_tone.wav` | stargate-sample-pack (fugue-state-audio) | `loops/130-discobeat.wav` | same |

Attribution is not required under CC0, but credit is appreciated.

Refresh locally:

```powershell
.\scripts\fetch_sample_assets.ps1
```

Synthetic fallback (offline):

```powershell
.\scripts\generate_sample_wavs.ps1
```
