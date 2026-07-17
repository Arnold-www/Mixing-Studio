# 成员 A 实现过程记录

成员 A 主责：`Model / DSP / Persistence`。

## 交付文档（后期集成）

| 文档 | 说明 |
|------|------|
| [功能报告_后期集成与UI.md](./功能报告_后期集成与UI.md) | 已实现功能总览、行为约定、源码索引与已知限制 |
| [使用说明_成员B测试指南.md](./使用说明_成员B测试指南.md) | macOS 启动 + 各 UI 组件作用说明与简要验收表 |

交叉测试结果请回填 [report/shared/CROSS_TEST_LOG.md](../shared/CROSS_TEST_LOG.md)。

## 负责模块

- `include/DSP/`、`src/DSP/`
- `include/Model/`、`src/Model/`
- 音频导入、多轨加载、播放状态、主音量等 Model 接口
- 混音、Pan、Limiter 等 DSP 算法（按阶段逐步补齐）
- 波形/VU/频谱数据、工程保存、WAV 导出
- 真实 WAV 解码、`QAudioSink` 输出、本地 FileDialog 导入
- `tests/` 单元测试与 `scripts/` 验证脚本

## 阶段记录模板

### 阶段：

- 日期：
- 使用的大模型：
- 采用模式：
- 提示词摘要：
- AI 输出内容：
- 人工修改内容：
- 自测结果：
- 成员 B 交叉测试结果：
- 对应提交：
- 可放入报告的证据：

## A 侧阶段拆分

1. 阶段 1：搭建 CMake、`DspProcessor`、`AudioEngine` 基础接口和 DSP 测试骨架。
2. 阶段 2：跑通本地音频导入、多轨加载、播放状态、Seek、Loop、主音量底层接口。
3. 阶段 3：实现多轨线性混音、主输出限幅、轨道音量、Pan 等底层处理。
4. 阶段 4：手写波形降采样、VU 电平、简易频谱数据；实现 JSON 工程保存/加载。
5. 阶段 5：完成 WAV 导出验收、DSP 单元测试补齐和底层架构检查清单。



## 阶段 1：Model/DSP 基建与测试骨架

- 日期：2026-07-11
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，人工审查、构建验证、本地合并。
- 提示词摘要：实现阶段一基建功能，包括 `DspProcessor`、`AudioEngine` 基础接口、CMake 测试骨架，并与 B 同学 feat 分支本地合并测试阶段一。
- AI 输出内容：
  - `DspProcessor` 移除对 Qt 的依赖，保留 `clampSample`、`applyGain`、`panLeftGain`、`panRightGain` 等阶段一基础函数。
  - `AudioEngine` 在 `main` 骨架上补充 `masterVolume()` 读取接口，并对主音量做 `[0, 1]` clamp。
  - 新增 `tests/test_dsp_processor.cpp`，覆盖限幅、增益、声像等 5 项断言。
  - `CMakeLists.txt` 增加 `enable_testing()` 和 `test_dsp_processor` CTest 目标。
  - 新增 `scripts/run_tests.ps1` 和 `scripts/run_tests.sh`，支持阶段一一键配置、构建和测试。
- 人工修改内容：从 `main` 基线创建 `feature/A-model-dsp-sprint1-infra`，仅实现阶段一范围；提交后本地 merge `origin/chai/feat`，保留 B 侧 ViewModel/QML。
- 自测结果：
  - 已运行 `.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp`，CTest 1/1 通过。
  - 已运行 `.\scripts\validate_feature.ps1`，架构边界检查 13/13 通过。
  - 已构建 `build/bin/Debug/MixingStudio.exe`，合并后应用构建成功。
- 成员 B 交叉测试结果：待成员 B 在 macOS 执行 `./scripts/run_tests.sh`，并填写 `report/shared/CROSS_TEST_LOG.md`。
- 对应提交：`2654de0`
- 可放入报告的证据：`DspProcessor` 基础函数、`test_dsp_processor` 测试代码、CTest 输出、`run_tests.ps1` 执行记录。



## 阶段 1.1：与 B feat 本地合并交叉测试

- 日期：2026-07-11
- 使用的大模型：Codex
- 采用模式：AI 主导合并与验证，人工确认 A/B 接口边界。
- 提示词摘要：将 `origin/chai/feat` 合并到 A 侧阶段一分支，验证阶段一 MVVM 边界和 ViewModel 对 `AudioEngine` 的调用链。
- AI 输出内容：
  - 本地 merge `origin/chai/feat` 到 `feature/A-model-dsp-sprint1-infra`，CMake 自动合并成功。
  - 保留 B 侧 `MixerViewModel`、`TrackViewModel`、`Main.qml` 与 A 侧 Model/DSP/测试目标共存。
  - 更新 `report/shared/CROSS_TEST_LOG.md` 和 `report/shared/TEST_AND_TOOLCHAIN.md` 记录阶段一交叉测试。
- 人工修改内容：确认 QML 不直接访问 Model/DSP；确认 `play`/`pause`/`stop`/`importTrack` 均通过 ViewModel 转发至 `AudioEngine`。
- 自测结果：
  - QML 不直接访问 `AudioEngine`/`DspProcessor`，`validate_feature.ps1` 通过。
  - `MixerViewModel::play/pause/stop` 转发至 `AudioEngine`，代码审查通过。
  - `importMockTrack`/`importAssetByName` 调用 `AudioEngine::importTrack`，代码审查通过。
  - `playbackStateChanged` 已连接至 ViewModel 播放定时器逻辑。
- 成员 B 交叉测试结果：A 侧本地交叉测试通过；待 B 复测 `AudioEngine` 接口可用性与 DSP 测试结果。
- 对应提交：`9cac45c`、`7e49d75`
- 可放入报告的证据：`validate_feature.ps1` 输出、合并后构建日志、`report/shared/CROSS_TEST_LOG.md`、主界面运行截图。



## 阶段 2：播放闭环底层接口

- 日期：2026-07-11
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，人工构建与测试验证。
- 提示词摘要：尝试构建阶段 2，跑通本地音频导入、多轨加载、播放状态、Seek、Loop、主音量底层接口。
- AI 输出内容：
  - `AudioEngine` 增加 `AudioTrack` 元数据、`positionMs`/`durationMs`、播放定时器推进、`seek`、`setLoopRange`、`clearTracks`。
  - 导入后使用 180000ms 占位时长（真实解码留待后续阶段）。
  - 新增 `tests/test_audio_engine.cpp`，覆盖导入、播放推进、暂停、Seek clamp、Loop 回绕、主音量边界。
  - 最小改动对接 `MixerViewModel`：进度与 Seek 改为消费 Model 时钟，Mock 波形/频谱仍由 B 侧分析定时器刷新。
  - 更新 `run_tests.ps1` 运行全部 CTest。
- 人工修改内容：默认不自动开启 Loop（`loopEndMs == 0` 表示无循环）；从 `feature/A-model-dsp-sprint1-infra` 创建 `feature/A-model-dsp-sprint2-playback`。
- 自测结果：
  - 已运行 `cmake --build build --config Debug`，应用与测试构建通过。
  - 已运行 `ctest --test-dir build -C Debug --output-on-failure`，CTest 2/2 通过。
- 成员 B 交叉测试结果：待成员 B 使用 UI 验证 Play/Pause/Stop/Seek/进度条是否与 Model 时钟一致。
- 对应提交：`b244414`
- 可放入报告的证据：`test_audio_engine.cpp`、CTest 2/2 输出、播放进度对接代码。



## 阶段 3：混音与 DSP 闭环

- 日期：2026-07-14
- 使用的大模型：Cursor Grok
- 采用模式：AI 主导代码生成，人工规划测试用例与构建验证。
- 提示词摘要：仅完成阶段三 A 侧：多轨线性混音、主输出限幅、轨道音量/Pan、三段 EQ、Compressor、Bypass 底层；自主规划测试并写入报告。不改 B 侧 EQ/压缩 UI。
- AI 输出内容：
  - 扩展 `DspProcessor`：`dbToLinear`、三段 EQ、压缩器、`processTrackSample`、线性混音、`applyMasterChain` 限幅。
  - 扩展 `AudioEngine` 轨参数与 `renderMixFrame` 离线混音（Mute/Solo 可听性在 Model 计算）。
  - 最小对接：已有 Volume/Pan/Mute/Solo 经 ViewModel 同步到 Model（便于现有条带驱动底层）；EQ/Comp/Bypass 仅 Model API + 单测覆盖，留给 B 后续挂 UI。
  - 扩展 `test_dsp_processor` 与 `test_audio_engine` 阶段三断言。
- 人工修改内容：分支 `feature/A-model-dsp-sprint3-mixing`；EQ 为可测宽带增益代理。
- 自测结果：
  - `.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64"` → CTest **2/2 通过**。
  - `cmake --build build --config Debug --target MixingStudio` 通过。
  - `.\scripts\validate_feature.ps1` → **13/13 通过**。
- 成员 B 交叉测试结果：待 B 后续实现 EQ/Comp/Bypass 控件后再做 UI 交叉；当前可用单测验收 A 底层。
- 对应提交：`7291c53`
- 可放入报告的证据：`DspProcessor` 混音链、`AudioEngine::renderMixFrame`、扩展后的 CTest 输出、`tests/README.md` 阶段三测试计划。

## 阶段 5：WAV 导出与交付清单

- 日期：2026-07-16
- 使用的大模型：Cursor Grok
- 采用模式：AI 主导实现，人工验收；待队友审核。
- 提示词摘要：实现阶段 5：WAV 导出验收、DSP 单测补强、底层架构检查清单与 A 侧记录。
- AI 输出内容：
  - `WavExporter` + `AudioEngine::exportMixToWav`（经 `renderMixFrame` 离线混音写 16-bit PCM WAV）
  - `ExportMixCommand`、`IMixerViewModel::exportMix`、Library「Export WAV」
  - `test_wav_export`；DSP mute/bypass/master clamp 补强
  - `docs/ARCHITECTURE_CHECKLIST.md`
- 人工修改内容：UI 导出默认 3 秒样例；完整时长走 Model API。
- 自测结果：见 `report/shared/TEST_AND_TOOLCHAIN.md`（阶段 5）。
- 成员交叉测试结果：待审核。
- 对应提交：待提交
- 可放入报告的证据：导出 WAV、`test_wav_export`、架构清单。

