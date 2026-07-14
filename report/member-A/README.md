# 成员 A 中期分报告

> 成员 A：Model / DSP / Persistence / 底层测试  
> 对应当前集成分支：`release/midterm-integration`  
> 最新集成提交：`194d823 merge: integrate sprint3 mixing`  
> 截止日期：2026-07-14

## 一、个人负责内容概述

本人在本项目中主要负责 Model、DSP、底层测试脚本，以及与 ViewModel 对接的音频引擎契约。具体包括：

1. 设计并实现 `AudioEngine`：轨元数据导入、播放状态机、Seek/Loop、主音量、轨道 Volume/Pan/Mute/Solo，以及离线混音入口 `renderMixFrame`。
2. 设计并实现纯 C++ `DspProcessor`：样本限幅、增益、声像、三段 EQ 代理、压缩器、单轨处理、线性混音与 master 限幅，便于单元测试且不依赖 Qt UI。
3. 搭建 CMake/CTest 与 `scripts/run_tests.*`、`validate_feature.ps1`，用自动化测试证明 Model/DSP 行为，并用架构检查保证 QML 不直接访问 Model/DSP。
4. 在 Sprint 2/3 中以最小改动对接 B 侧 ViewModel：播控时钟与 Seek 由 Model 提供；Volume/Pan/Mute/Solo 经 ViewModel 同步进引擎；EQ/Compressor/Bypass 仅提供 Model API，不改 B 侧 EQ UI。
5. 按阶段在 `feature/A-model-dsp-sprint*` 分支提交，供成员 B 合入 `release/midterm-integration` 作为中期发布分支。

## 二、与评分要求的对应

| 评分项 | A 侧贡献 |
| :--- | :--- |
| 成员协作与有效提交 | A 侧在独立 feature 分支完成 Sprint 1–3 提交并推送；Sprint 2/3 由 B 合入 `release/midterm-integration`；阶段一曾与 B 的 `chai/feat` 本地交叉合并验证。 |
| 先进框架开发 | A 侧使用 C++17 + Qt 6（引擎状态用 `QObject`/`QTimer`）+ CMake/CTest。DSP 与核心算法放在 Model/DSP，经 ViewModel 契约暴露，QML 不直接调用。 |
| 完整报告 | A 侧维护 `report/member-A/README.md` 分报告与阶段过程证据，并参与 `report/shared/` 交叉测试与工具链记录。 |

## 三、有效提交记录

| 提交 | 类型 | 内容 |
| :--- | :--- | :--- |
| `2654de0` | Model/DSP | Sprint 1：DSP 测试骨架与 `AudioEngine` getter |
| `9cac45c` | 交叉 | 本地合并 B 侧 ViewModel/View，完成阶段一交叉测试 |
| `7e49d75` | 报告 | 记录阶段一交叉测试与工具链 |
| `b244414` | Model | Sprint 2：播放时钟、Seek、Loop 与 `AudioEngine` 测试 |
| `7291c53` | Model/DSP | Sprint 3：混音链、轨 DSP 参数与测试 |
| `6e9a5aa` | 报告 | 记录 Sprint 3 提交哈希 |
| `b28ea5c` | 集成 | B 合入 Sprint 2 playback（内容来自 A） |
| `194d823` | 集成 | B 合入 Sprint 3 mixing（内容来自 A） |

## 四、A 侧技术实现

### 1. AudioEngine 播放与轨参数

`AudioEngine` 是 Model 层对外主入口，中期已提供：

- `importTrack` / `clearTracks`：注册占位轨元数据（尚未真实 WAV 解码）
- `play` / `pause` / `stop`：播放状态与定时推进
- `seek` / `setLoopRange`：进度跳转与循环区间
- `setMasterVolume`：主音量 `[0, 1]`
- `setTrackVolume` / `setTrackPan` / `setTrackMuted` / `setTrackSolo`：轨混音参数
- `setTrackEq` / `setTrackCompressor` / `setTrackFxBypass`：FX 参数 API（UI 未挂）
- `renderMixFrame`：按 Mute/Solo 可听性做离线立体声混音帧
- `positionMs` / `durationMs` / `playing` 等属性信号：供 ViewModel 绑定

导入后默认占位时长 180000ms，为后续真实解码保留接口形状。

### 2. DspProcessor 混音链

`DspProcessor` 为无 Qt UI 依赖的纯函数工具集，中期覆盖：

- 基础：`clampSample`、`applyGain`、`panLeftGain` / `panRightGain`、`dbToLinear`
- 单轨：三段 EQ 宽带增益代理、压缩器、`processTrackSample`
- 总线：线性混音、`applyMasterChain` 主音量与限幅

该层由 `test_dsp_processor` 直接断言，不经过 QML。

### 3. 测试与验证脚本

- `tests/test_dsp_processor.cpp`：限幅、增益、声像、EQ/压缩、混音与 master 链
- `tests/test_audio_engine.cpp`：导入、播放推进、暂停、Seek clamp、Loop、主音量与轨参数/混音帧
- `scripts/run_tests.ps1` / `run_tests.sh`：一键配置、构建、CTest
- `scripts/validate_feature.ps1`：架构边界检查（QML 不直接碰 Model/DSP）

中期集成分支上 CTest 目标为 **2/2**（`dsp_processor`、`audio_engine`）。

### 4. 与 ViewModel 的契约

- Sprint 2：B 将进度/Seek/Master 从 ViewModel 自维护秒表改为消费 `AudioEngine` 时钟。
- Sprint 3：B 通过 `syncTrackToEngine()` 把 Volume/Pan/Mute/Solo 写入 Model；A 不改 B 的 EQ/压缩 UI。
- 波形/频谱/素材库/工程面板仍由 B 侧 Mock；A 侧分析与持久化未进入本次中期集成交付。

## 五、当前实现边界

需要明确的是，A 侧中期交付的是可测试的播控与混音参数链路，不是完整真实音频 I/O。

当前能用的部分：

- `AudioEngine` 播放时钟、Seek、Loop、主音量可被 ViewModel 驱动。
- 多轨 Volume/Pan/Mute/Solo 可写入 Model，并参与 `renderMixFrame`。
- DSP 混音链与引擎行为可用 CTest 证明。
- 架构检查可通过，QML 不直接访问 Model/DSP。

当前仍是占位或未合入中期的部分：

- `importTrack` 不解码真实 WAV；时长为 180s 占位。
- `play()` 推进时钟，不向声卡输出真实声音。
- EQ/Compressor/Bypass 有 Model API 与单测，无对应 UI。
- 波形/VU/频谱真实分析、SQLite 素材库、JSON 工程保存等阶段四能力：**本地开发中，尚未合入 `release/midterm-integration`，不作为中期已完成项。**

## 六、自测与集成验证

A 侧在 Windows（Qt 6.5.3 MSVC）上对 Sprint 2/3 及中期等价能力完成构建与测试：

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Debug --target MixingStudio
.\scripts\validate_feature.ps1
```

测试结果（与中期集成分支一致）：

```text
1/2 Test #1: dsp_processor .................... Passed
2/2 Test #2: audio_engine ..................... Passed
100% tests passed, 0 tests failed out of 2
```

补充检查：

- `MixingStudio` Debug 构建通过。
- `validate_feature.ps1` 架构边界 **13/13** 通过。
- 由于未接真实声卡输出，验证证明的是 Model/DSP 状态与混音计算链路，不能证明真实音频播放完成。

B 侧已在 macOS 对 `release/midterm-integration` 复测 CTest 2/2 与应用启动，详见 `report/member-B/README.md` 与 `report/shared/`。

## 七、问题反思

A 侧前期用占位时长与离线 `renderMixFrame` 推进接口是合理的，便于与 B 并行开发 UI。但中期以后，占位时钟不能继续充当“已能播放”的证据。当前最明显的缺口是：**控制链与混音参数已通，真实解码与声卡输出未通。**

因此下一阶段 A 侧需要与 B 协同：

1. A 提供 WAV 解码或 PCM 读取接口，替换占位轨。
2. A 用 `QAudioSink`（或等价方案）实现真实输出。
3. A 完成分析数据（波形/VU/频谱）与持久化（JSON/SQLite）并合入集成分支，供 B 替换 Mock。
4. B 补充 EQ/Compressor/Bypass 控件，对接 A 侧 Sprint 3 已提供的 Model API。
5. 双方继续交叉测试，避免 UI 把 Mock 描述成真实音频能力。

## 八、个人小结

到中期为止，A 侧完成了 Model/DSP 从测试骨架到播控时钟、再到多轨混音参数链的分层实现，并用 CTest 与架构脚本固化证据。主要贡献集中在 `AudioEngine`、`DspProcessor`、单元测试与对 ViewModel 的稳定契约。当前成果足以支撑中期报告中关于协作、框架与底层过程完整性的要求，但距离真实可用的音频软件仍需要完成解码、输出、真实分析与持久化合入。

---

成员 A 主责：`Model / DSP / Persistence / 底层测试`。

## 负责模块

- `include/DSP/`、`src/DSP/`
- `include/Model/`、`src/Model/`
- 音频导入、多轨加载、播放状态、Seek/Loop、主音量等 Model 接口
- 混音、Pan、EQ、Compressor、Limiter 等 DSP 算法
- `tests/` 单元测试与 `scripts/` 验证脚本
- 波形/VU/频谱、工程保存、素材库、WAV 导出（中期后继续）

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
2. 阶段 2：跑通导入、多轨、播放状态、Seek、Loop、主音量底层接口。
3. 阶段 3：实现多轨线性混音、主输出限幅、轨道音量/Pan/EQ/压缩等底层处理。
4. 阶段 4（中期后）：波形降采样、VU、频谱、峰值/削波；SQLite 素材库与 JSON 工程保存/加载。
5. 阶段 5：WAV 导出验收、测试补齐与底层架构检查清单。

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
