# Tests

## 当前测试

| 测试目标 | 源文件 | 覆盖范围 |
| :------- | :----- | :------- |
| `dsp_processor` | `test_dsp_processor.cpp` | clamp/gain/pan、EQ/压缩/混音/限幅；波形降采样、VU、频谱、峰值/削波 |
| `audio_engine` | `test_audio_engine.cpp` | 播放闭环；轨 DSP 混音；`refreshAnalysis` 波形/频谱点数 |
| `project_store` | `test_project_store.cpp` | JSON 工程保存/加载（主音量、轨参、Loop、位置） |
| `asset_library` | `test_asset_library.cpp` | SQLite upsert、搜索、最近列表 |
| `common_types` | `test_common_types.cpp` | Common `planSolo` 等 |
| `viewmodel_commands` | `test_viewmodel_commands.cpp` | ViewModel slots：Import/Play/Pause/MasterVolume |
| `ui_binder` | `test_ui_binder.cpp` | UI 自动化演示：导入 `demo_tone` → Demo 加轨 → Spectrum/Library → Master 键盘 70% → Song Loop → Seek → Play/Pause/Stop → Save → Mock → Projects → 关闭。CTest 默认 headless + `PLAY_MS=3000`；本地看窗口直接跑 exe（`MIXINGSTUDIO_UI_STEP_MS` / `MIXINGSTUDIO_UI_PLAY_MS` 可调） |
| `wav_export` | `test_wav_export.cpp` | RIFF 头、`exportMixToWav` 短时混音导出 |
| `wav_decoder` | `test_wav_decoder.cpp` | WAV round-trip；`AudioFileDecoder` 路由 |

```powershell
.\scripts\run_tests.ps1
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

当前应通过：`dsp_processor`、`audio_engine`、`project_store`、`asset_library`、`common_types`、`viewmodel_commands`、`ui_binder`、`wav_export`、`wav_decoder`。