# Tests

## 当前测试

| 测试目标 | 源文件 | 覆盖范围 |
| :------- | :----- | :------- |
| `dsp_processor` | `test_dsp_processor.cpp` | clamp/gain/pan、EQ/压缩/混音/限幅；波形降采样、VU、频谱、峰值/削波 |
| `audio_engine` | `test_audio_engine.cpp` | 播放闭环；轨 DSP 混音；`refreshAnalysis` 波形/频谱点数 |
| `project_store` | `test_project_store.cpp` | JSON 工程保存/加载（主音量、轨参、Loop、位置） |
| `asset_library` | `test_asset_library.cpp` | SQLite upsert、搜索、最近列表 |
| `common_types` | `test_common_types.cpp` | Common `planSolo` 等 |
| `commands` | `test_commands.cpp` | Command 层 Play/Pause/Import 执行 |

## 阶段四测试计划（A 自拟）

1. **DSP 分析**：正弦波降采样 bins、VU/峰值非零、未削波；注入 1.0 样点触发削波；频谱能量 > 0。
2. **Engine**：`refreshAnalysis` 输出 64 波形点 / 18 频谱带。
3. **JSON**：round-trip 恢复轨数、Mute、主音量、Loop、Seek。
4. **SQLite**：upsert 两条素材，按名搜索与 recent(limit)。
5. **架构**：`validate_feature.ps1`；QML 经 `IMixerViewModel` 消费分析/工程数据（阶段 4 改绑已完成）。

## 一键运行

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.8.3\msvc2022_64" -WithApp
.\scripts\validate_feature.ps1
```

当前应通过：`dsp_processor`、`audio_engine`、`project_store`、`asset_library`、`common_types`、`commands`。
