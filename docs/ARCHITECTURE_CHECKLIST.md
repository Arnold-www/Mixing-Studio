# 底层架构检查清单（阶段 5 / 成员彭）

本清单用于最终交付前对 Model / DSP / Command / App 边界做人工核对。自动化部分由 `scripts/validate_feature.ps1` 覆盖。

## A. 目录与依赖

| 检查项 | 期望 | 验证方式 |
| :--- | :--- | :--- |
| 头文件在 `include/`，实现在 `src/` | 通过 | `validate_feature`：No headers under src |
| QML 不引用 Model/DSP/App/Command/RealVM | 通过 | `validate_feature`：QML boundary |
| Model/DSP 不依赖 View/ViewModel/App/Command | 通过 | `validate_feature`：Model/DSP deps |
| App 仅装配并以 `IMixerViewModel*` 注入 | 通过 | `MixingStudioApp::bindViewModelToQml` |
| Command 操作 `AudioEngine*`，不碰 QML | 通过 | `include/Command/*` |

## B. Model / DSP 能力

| 检查项 | 期望 | 验证方式 |
| :--- | :--- | :--- |
| 播放 / Seek / Loop / Master | 可用 | `test_audio_engine` |
| 轨 Volume/Pan/Mute/Solo + 混音限幅 | 可用 | `test_audio_engine` / `test_dsp_processor` |
| 波形 / VU / 频谱 / 削波 | 可用 | `test_dsp_processor` / `refreshAnalysis` |
| JSON 工程保存/加载 | 可用 | `test_project_store` |
| SQLite 素材库 | 可用 | `test_asset_library` |
| WAV 混音导出 | 可用 | `test_wav_export`；UI：Export WAV |
| WAV 解码 / 本地导入 | 可用 | `test_wav_decoder`；UI：Import Audio（WAV/MP3） |
| MP3 解码 | 可用 | `AudioFileDecoder` + Qt Multimedia |
| `QAudioSink` 输出 | 可用 | Play 走设备；失败时回退定时器时钟 |
| EQ / Compressor UI | 可用 | Track strip FX / EQ / Comp 控件 |
| Loop 区间 UI | 可用 | Transport Loop 开关 + Start/End；开启后各轨按自身长度/变速独立循环 |
| 最近工程扫描 / 删除 | 可用 | 启动扫描 AppData `projects/*.json`；Delete Selected |
| 样例工程 | 可用 | `samples/` + Load Sample（多轨 demo_session） |
| Mock 验证模式 | 可用 | Transport Mock 开关 |
| 素材库种子 WAV | 可用 | CC0 实采：`fetch_sample_assets.ps1`；见 `samples/ATTRIBUTION.md` |
| 10 段图形 EQ | 可用 | 轨条 EQ 10；`applyGraphicEq`；工程 JSON `eqBands` |
| 音量自动化包络 | 可用 | Waveform 点击/拖点；选中轨 Clear Auto |
| 全时长波形总览 | 可用 | 256 bins；播放中 live EMA + 60FPS 插值绘制 |
| 3D 实时频谱 | 可用 | 32 带 + 16 层纵深伪 3D；16ms 刷新 |
| 夜间 UI | 可用 | 硬编码夜色 + Material Dark；顶栏 ToolIconButton |
| 主台布局 | 可用 | Waveform 固定中上；轨条整行；Library/Projects/Spectrum 顶栏抽屉 |
| 单轨变速 | 可用 | Speed 0.5x–2.0x；线性插值重采样 |
| 删除音轨 | 可用 | Mixer Delete Track；`AudioEngine::removeTrack` |

## C. 交付闭环（人工冒烟）

1. `.\scripts\run_app.ps1`
2. **Load Sample** 或 **Import Audio** → Play 听声卡输出；Library 应列出多个样例 WAV
3. 选中轨调 **EQ 10**，在波形区点加音量自动化点 → Save / Restore / Export
4. `.\scripts\run_tests.ps1 -WithApp`
5. `.\scripts\validate_feature.ps1`

## D. 阶段边界说明

- 支持 PCM WAV 与 MP3（依赖 Qt Multimedia FFmpeg 插件）；其它格式需先转换。
- UI 导出默认 **3 秒** 样例；完整时长导出走 `AudioEngine::exportMixToWav(path)`。
- **Demo Track** 仍生成短占位正弦；**Mock** 仅替换分析显示，不关掉播放。
- 图形 EQ 为增益积原型（非完整 Biquad）；自动化仅支持 **Volume** 单参数。
