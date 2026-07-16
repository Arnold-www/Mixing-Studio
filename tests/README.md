# Tests

## 当前测试

| 测试目标 | 源文件 | 覆盖范围 |
| :------- | :----- | :------- |
| `dsp_processor` | `test_dsp_processor.cpp` | clamp/gain/pan、EQ/压缩/混音/限幅；波形降采样、VU、频谱、峰值/削波 |
| `audio_engine` | `test_audio_engine.cpp` | 播放闭环；轨 DSP 混音；`refreshAnalysis` 波形/频谱点数 |
| `project_store` | `test_project_store.cpp` | JSON 工程保存/加载（主音量、轨参、Loop、位置） |
| `asset_library` | `test_asset_library.cpp` | SQLite upsert、搜索、最近列表 |
| `common_types` | `test_common_types.cpp` | Common `planSolo` 等 |
| `commands` | `test_commands.cpp` | Command 层 Play/Pause/Import/Save/Load |
| `wav_export` | `test_wav_export.cpp` | RIFF 头、`exportMixToWav` 短时混音导出 |
| `wav_decoder` | `test_wav_decoder.cpp` | WAV round-trip；`AudioFileDecoder` 路由 |

```powershell
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

当前应通过：`dsp_processor`、`audio_engine`、`project_store`、`asset_library`、`common_types`、`commands`、`wav_export`、`wav_decoder`。
