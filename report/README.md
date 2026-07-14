# Mixing Studio 中期整体报告

> 本报告对应当前集成分支 `release/midterm-integration`，提交 `194d823`。  
> 截止日期：2026-07-14。  
> 说明：本项目中期版本已完成 Qt/QML 调音台交互、ViewModel 与 Model 接口对接、Model/DSP 单元测试和报告证据整理；真实音频文件解码与声卡输出仍属于下一阶段任务，本文不夸大为已完成。

## 一、项目概况

Mixing Studio 是一个基于 Qt 6、C++17、QML、CMake 和 MVVM 架构开发的多轨调音项目。项目目标是逐步实现多轨音频导入、播放控制、音量与声像调节、Mute/Solo、基础 DSP 处理、工程保存与导出等功能。

中期阶段的核心目标不是一次性完成完整 DAW，而是先建立可持续协作的工程结构、可运行的 Qt 前端、清晰的 Model/ViewModel/View 边界、可测试的 DSP/Model 底层接口，以及完整的过程报告证据。当前版本已经形成可交互的调音台原型，并完成播放时钟、轨道参数、DSP 单帧混音链和 UI 绑定链路的初步打通。

## 二、中期评分要求对应情况

| 评分要素 | 分值 | 当前完成情况 |
| :--- | ---: | :--- |
| 成员协作紧密，且在版本控制系统上都有有效的提交 | 30 | 已完成。成员 A 负责 Model/DSP/测试，成员 B 负责 ViewModel/View/报告和接口对接。双方均有独立 feature 分支、有效提交、交叉测试记录和集成分支。 |
| 完整地按照最先进的框架开发 | 20 | 已完成中期要求。项目采用 Qt 6 + QML + C++17 + CMake + CTest，按照 MVVM 分层组织。QML 不直接访问 Model/DSP，ViewModel 作为界面状态和命令转发层。 |
| 完整的整体报告和分报告 | 50 | 已补齐。整体报告见本文件；成员 B 分报告见 `report/member-B/README.md`；成员 A 过程记录见 `report/member-A/README.md`；共享测试与 AI 使用记录见 `report/shared/`。 |

## 三、成员分工

| 成员 | 负责范围 | 主要产出 |
| :--- | :--- | :--- |
| 成员 A | Model / DSP / 测试脚本 / 底层接口 | `AudioEngine` 播放状态、轨道参数、Seek/Loop、`DspProcessor` 增益/Pan/EQ/压缩/混音/限幅、CTest 单元测试 |
| 成员 B | ViewModel / View / 报告 / 集成验证 | `MixerViewModel`、`TrackViewModel`、`Main.qml` 调音台界面、波形/频谱/电平可视化、素材库与工程入口、B 分报告、集成分支 |

## 四、版本控制与协作证据

项目使用 Git 和 GitHub 进行协作。开发过程按功能分支推进，再通过本地 merge 和远程 PR 分支进行集成。

### 关键分支

| 分支 | 说明 | 当前状态 |
| :--- | :--- | :--- |
| `main` | 主分支，当前远程主分支停在 Sprint 1 合并点 | `f715d01` |
| `feature/A-model-dsp-sprint1-infra` | A 侧 Sprint 1 Model/DSP 基建 | 已合入 main |
| `feature/A-model-dsp-sprint2-playback` | A 侧 Sprint 2 播放时钟、Seek、Loop、测试 | 已进入集成分支 |
| `feature/A-model-dsp-sprint3-mixing` | A 侧 Sprint 3 混音链、轨道 DSP 参数、测试 | 已进入集成分支 |
| `chai/feat` | B 侧 ViewModel/View 阶段开发分支 | 远程已删除，本地保留提交证据 |
| `release/midterm-integration` | 中期集成分支，包含 Sprint 2 和 Sprint 3 两次 merge | 已推送到 GitHub，可用于创建 PR |

### 有效提交摘录

| 提交 | 成员 | 内容 |
| :--- | :--- | :--- |
| `2654de0` | A | 增加 Sprint 1 DSP 测试骨架和 `AudioEngine` getter |
| `9cac45c` | A+B | 本地合并 B 侧 ViewModel/View，完成阶段一交叉测试 |
| `b244414` | A | 增加 Sprint 2 播放时钟、Seek、Loop 和 `AudioEngine` 测试 |
| `7291c53` | A | 增加 Sprint 3 混音链、轨道 DSP 参数和测试 |
| `d7704c2` | B | 规划 B 侧分阶段实现 |
| `f7245af` | B | 暴露播放 transport 状态到 ViewModel |
| `7a25348` | B | 建立轨道 Solo/Audible 状态机 |
| `10f525b` | B | 渲染 Mock 波形、电平和频谱 |
| `60e47c7` | B | 增加 Mock 素材库和项目面板 |
| `f055073` | B | 重构调音台工作区布局 |
| `4fcbeaa` | B | 调整项目素材面板位置 |
| `b28ea5c` | B | 集成 Sprint 2 playback 到中期发布分支 |
| `194d823` | B | 集成 Sprint 3 mixing 到中期发布分支 |

## 五、技术架构

项目采用 MVVM 架构，避免界面层直接依赖底层音频处理。

```text
QML View
  src/View/Main.qml
        |
        v
ViewModel
  MixerViewModel / TrackViewModel
        |
        v
Model
  AudioEngine / AudioTrack
        |
        v
DSP
  DspProcessor
```

### View 层

`src/View/Main.qml` 负责界面布局和用户交互，包括顶部 Transport、播放进度、Master 音量、中部 Waveform/Spectrum、左侧项目和素材库入口、底部 Mixer channel strip。

### ViewModel 层

`MixerViewModel` 提供 QML 可绑定属性和槽函数，包括轨道列表、播放状态、时间文本、播放进度、主音量、素材搜索、最近项目、波形点、频谱点等。  
`TrackViewModel` 管理单轨状态，包括音量、Pan、Mute、Solo、Audible、电平和文本回显。

Sprint 2/3 集成后，ViewModel 已将播放位置、时长、Seek、Master、Volume、Pan、Mute、Solo 等接口同步到 `AudioEngine`。

### Model 层

`AudioEngine` 管理播放状态、导入轨道元数据、播放位置、时长、Loop、主音量和轨道参数。当前播放时钟由 `QTimer` 驱动，导入后使用 180 秒占位时长，为后续真实解码和音频输出保留接口。

### DSP 层

`DspProcessor` 提供纯 C++ DSP 工具函数，包含 sample clamp、gain、pan、dB 转线性、三段 EQ 代理、压缩器、单轨处理、线性混音和 master 限幅。该层不依赖 Qt UI，便于单元测试。

## 六、当前功能完成情况

### 已完成

| 功能 | 完成情况 |
| :--- | :--- |
| Qt/QML 应用启动 | 已完成，`MixingStudio` 可构建并运行 |
| MVVM 工程结构 | 已完成，View、ViewModel、Model、DSP 分层明确 |
| 轨道导入入口 | 已完成中期入口，当前导入的是 mock/占位轨道 |
| 播放控制 | 已完成 Play/Pause/Stop、播放位置、Seek、时长显示 |
| 主音量 | 已完成 UI 到 Model 的同步 |
| 轨道 Volume/Pan | 已完成 UI 到 ViewModel，再同步到 Model |
| Mute/Solo | 已完成 UI 状态、Audible 规则和 Model 参数同步 |
| 波形/频谱/电平显示 | 已完成 mock 数据可视化消费路径 |
| DSP 单帧处理链 | 已完成并有单元测试 |
| CMake/CTest | 已完成基础测试目标和脚本 |
| 报告证据 | 已完成整体报告、B 分报告、共享测试记录 |

### 当前边界

| 尚未完成 | 说明 |
| :--- | :--- |
| 真实音频文件解码 | 当前没有读取真实 wav/mp3/flac PCM 数据 |
| 声卡输出 | 当前 `AudioEngine::play()` 只推进时钟，不向 `QAudioSink` 输出声音 |
| 真实波形/频谱 | 当前为 mock 分析数据，未从真实音频采样计算 |
| 工程持久化 | 当前项目恢复和保存快照为 mock/stub |
| WAV 导出 | 尚未实现真实混音导出 |

该边界在报告中需要明确说明：当前中期版本能展示架构、交互和接口打通，但不是完整真实音频工作站。

## 七、测试与验证

### macOS 集成验证（成员 B）

环境：macOS、Homebrew Qt 6.11.1、CMake 4.3.4、Apple clang 17。

由于本机 Qt 为 Homebrew 拆包安装，集成测试使用显式 Qt CMake 路径：

```bash
cmake -S . -B build-midterm \
  -DCMAKE_PREFIX_PATH=/opt/homebrew \
  -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6 \
  -DQt6Qml_DIR=/opt/homebrew/lib/cmake/Qt6Qml \
  -DQt6Quick_DIR=/opt/homebrew/lib/cmake/Qt6Quick \
  -DQt6QuickControls2_DIR=/opt/homebrew/lib/cmake/Qt6QuickControls2

cmake --build build-midterm --config Debug --target MixingStudio
ctest --test-dir build-midterm -C Debug --output-on-failure
```

最近一次集成测试结果：

```text
1/2 Test #1: dsp_processor .................... Passed
2/2 Test #2: audio_engine ..................... Passed
100% tests passed, 0 tests failed out of 2
```

### Windows 验证（成员 A）

A 侧在 Windows + Qt 6.5.3 MSVC 环境下完成 Sprint 1/2/3 构建和 CTest。阶段 3 记录显示 `test_dsp_processor` 和 `test_audio_engine` 均通过，`validate_feature.ps1` 架构检查 13/13 通过。

## 八、报告与证据文件

| 文件 | 内容 |
| :--- | :--- |
| `report/README.md` | 中期整体报告 |
| `report/member-A/README.md` | 成员 A Model/DSP 实现过程记录 |
| `report/member-B/README.md` | 成员 B ViewModel/View/Report 分报告 |
| `report/shared/AI_USAGE_LOG.md` | AI 使用记录 |
| `report/shared/CROSS_TEST_LOG.md` | 交叉测试记录 |
| `report/shared/TEST_AND_TOOLCHAIN.md` | 测试与工具链记录 |
| `scripts/run_tests.sh` / `scripts/run_tests.ps1` | 构建与测试脚本 |
| `scripts/validate_feature.ps1` | MVVM 架构边界检查脚本 |

## 九、问题反思与下一阶段计划

中期阶段最大的不足是前端交互和底层 DSP 测试已经推进，但真实音频链路尚未接入。当前 mock 方案有利于先建立 ViewModel/QML 的绑定和团队协作边界，但后续必须尽快替换为真实音频数据，否则项目会停留在演示原型。

下一阶段建议按最小真实可用路径推进：

1. 增加 QML `FileDialog`，允许选择本地 `.wav` 文件。
2. 在 Model 层实现 PCM WAV 解析，先不引入复杂第三方库。
3. 使用 Qt Multimedia 的 `QAudioSink` 输出真实音频。
4. 将多轨 PCM 数据接入现有 Volume/Pan/Mute/Solo/Master 混音链。
5. 从真实 PCM 计算 waveform、VU 和简易频谱，替换当前 mock 可视化。
6. 在真实音频链路稳定后，再扩展工程保存、WAV 导出、MP3/FLAC 支持。

## 十、中期结论

截至当前集成分支，项目满足中期“协作、框架、报告”三项核心要求：成员 A/B 均有有效提交和明确分工；项目采用 Qt 6、QML、C++17、CMake、CTest 和 MVVM 架构开发；整体报告、成员分报告和测试记录已经形成。当前版本可作为中期展示和后续真实音频链路开发的基础，但仍需在下一阶段完成真实文件解码和音频输出，才能达到真实可用的调音软件标准。
