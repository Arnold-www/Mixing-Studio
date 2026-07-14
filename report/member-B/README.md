# 成员 B 中期分报告

> 成员 B：ViewModel / View / Report / 集成验证  
> 对应当前集成分支：`release/midterm-integration`  
> 最新集成提交：`194d823 merge: integrate sprint3 mixing`  
> 截止日期：2026-07-14

## 一、个人负责内容概述

本人在本项目中主要负责 ViewModel、QML View、报告材料和中期集成验证。具体包括：

1. 设计并实现 `MixerViewModel` 与 `TrackViewModel`，为 QML 暴露播放状态、轨道状态、素材列表、波形/频谱/电平数据和用户操作入口。
2. 设计并实现 `src/View/Main.qml` 调音台界面，包括 Transport、播放进度、Master 音量、Waveform、Spectrum、素材库、最近工程和横向 mixer channel strip。
3. 在成员 A 的 `AudioEngine` 接口逐步完善后，将 ViewModel 的播放进度、Seek、Master、Volume、Pan、Mute、Solo 同步到底层 Model。
4. 维护报告目录结构，记录 B 侧过程、AI 使用、构建测试、交叉测试和中期整体报告。
5. 创建并推送 `release/midterm-integration` 分支，将成员 A 的 Sprint 2 和 Sprint 3 分支按顺序集成，作为 GitHub PR 使用的中期发布分支。

## 二、与评分要求的对应

| 评分项 | B 侧贡献 |
| :--- | :--- |
| 成员协作与有效提交 | B 侧在 `chai/feat` 上完成多阶段 ViewModel/View 提交；后续在 `release/midterm-integration` 上完成 Sprint 2、Sprint 3 两次集成 merge。 |
| 先进框架开发 | B 侧使用 Qt 6 QML + C++17 + MVVM。QML 只绑定 ViewModel，不直接访问 `AudioEngine` 或 `DspProcessor`。 |
| 完整报告 | B 侧负责整理 `report/README.md` 整体报告、`report/member-B/README.md` 分报告，并维护共享测试记录。 |

## 三、有效提交记录

| 提交 | 类型 | 内容 |
| :--- | :--- | :--- |
| `d7704c2` | 规划 | 规划 B 侧分阶段实现方案 |
| `f7245af` | ViewModel | 暴露 mock 播放 transport 状态 |
| `f109d01` | 报告 | 记录 B 侧阶段一验证 |
| `7739f16` | 构建 | 分离可执行输出和 QML module 输出 |
| `7a25348` | ViewModel | 建立轨道 Solo 和 Audible 状态 |
| `85ebefd` | 报告 | 记录轨道状态验证 |
| `10f525b` | View/QML | 渲染 mock waveform、spectrum 和 meters |
| `db1fecd` | 报告 | 记录 mock 可视化验证 |
| `60e47c7` | View/QML | 增加 mock 素材库和项目面板 |
| `66c4564` | 报告 | 记录素材库验证 |
| `8680017` | View/QML | 收紧 mixer panel 布局 |
| `f055073` | View/QML | 重构 mixer workspace 布局 |
| `d450e1c` | 报告 | 记录 UI review redesign |
| `8dda167` | View/QML | 保证启动时 mixer 可见 |
| `4fcbeaa` | View/QML | 将项目素材库面板移动到左侧 |
| `b28ea5c` | 集成 | 合入 Sprint 2 playback |
| `194d823` | 集成 | 合入 Sprint 3 mixing |

## 四、B 侧技术实现

### 1. ViewModel 状态层

B 侧实现的 `MixerViewModel` 负责把底层 Model 能力转换为 QML 易绑定的属性和命令。主要属性包括：

- `tracks`
- `playing`
- `statusMessage`
- `masterVolume`
- `positionSeconds`
- `durationSeconds`
- `playbackProgress`
- `playbackTimeText`
- `anySolo`
- `waveformPoints`
- `spectrumLevels`
- `assetSearchText`
- `filteredAssetNames`
- `recentProjectNames`

主要槽函数包括：

- `importMockTrack()`
- `importAssetByName()`
- `restoreRecentProject()`
- `saveMockProject()`
- `play()`
- `pause()`
- `stop()`
- `seekToProgress()`
- `setMasterVolume()`
- `setAssetSearchText()`

在 Sprint 2 集成后，播放位置、时长和 Seek 由 `AudioEngine` 提供，不再只由 ViewModel 自己维护秒表。  
在 Sprint 3 集成后，轨道 Volume、Pan、Mute、Solo 会通过 `syncTrackToEngine()` 同步到底层 `AudioEngine`，使 UI 上的 channel strip 控件能够驱动 Model 参数。

### 2. TrackViewModel 单轨状态

`TrackViewModel` 负责单条轨道的界面状态，包括：

- 轨道名 `name`
- 音量 `volume`
- 声像 `pan`
- 静音 `muted`
- 独奏 `solo`
- 是否可听 `audible`
- 电平 `meterLevel`
- 文本回显 `volumeText`、`panText`、`meterText`

其中 `audible` 由 Mute/Solo 规则共同决定，避免 QML 自行判断复杂业务逻辑。

### 3. QML 界面

`Main.qml` 当前形成了一个可操作的调音台界面：

- 顶部：标题、Import、Play/Pause、Stop、时间码、播放进度、Master 音量和状态消息。
- 中部：Waveform 与 Spectrum 两个分析面板，带坐标网格和播放位置线。
- 左侧：最近工程、恢复入口、保存快照、素材库搜索、选中素材导入。
- 主区域：横向 mixer channel strip，每条轨道包含电平条、音量推子、Pan、Mute、Solo 和电平文本。

### 4. 报告与证据整理

B 侧负责把开发过程转化为可检查的报告证据，包括：

- 在 `report/member-B/README.md` 中记录每个阶段的 AI 使用、实现内容、人工修改、自测结果和对应提交。
- 在 `report/shared/TEST_AND_TOOLCHAIN.md` 中记录 macOS/Windows 工具链、构建命令和测试结果。
- 在 `report/shared/CROSS_TEST_LOG.md` 中记录交叉测试和阶段检查项。
- 在 `report/README.md` 中补充中期整体报告，说明项目现状、分工、架构、测试和后续计划。

## 五、当前实现边界

需要明确的是，B 侧当前实现的是可交互调音台界面和接口绑定链路，不是完整真实音频播放链路。

当前能用的部分：

- 可以启动 Qt/QML 应用。
- 可以通过 Import 或素材库导入入口创建轨道。
- 可以操作 Play/Pause/Stop、Seek、Master。
- 可以操作单轨 Volume、Pan、Mute、Solo。
- 可以看到 waveform、spectrum、meter 的动态变化。
- UI 控件能够通过 ViewModel 同步到 `AudioEngine` 的对应接口。

当前仍是 mock 或 stub 的部分：

- 素材库是内置 mock 名称，不是磁盘文件浏览。
- `AudioEngine::play()` 当前推进播放时钟，不向声卡输出真实声音。
- waveform、spectrum、meter 当前是 mock/确定性模拟数据，不是从真实 PCM 计算。
- 最近工程和保存快照是界面入口，未实现真实 JSON/数据库持久化。

这个边界是中期报告中必须诚实说明的内容。下一阶段需要优先把 mock 文件导入替换为真实 WAV 导入和 `QAudioSink` 输出。

## 六、自测与集成验证

B 侧在 macOS 上对当前中期集成分支完成了构建和测试：

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

测试结果：

```text
1/2 Test #1: dsp_processor .................... Passed
2/2 Test #2: audio_engine ..................... Passed
100% tests passed, 0 tests failed out of 2
```

应用启动检查结果：

- `MixingStudio` 可执行文件构建成功。
- 本地启动后未出现终端 QML/runtime 错误。
- 由于当前未接真实声卡输出，启动验证仅能证明 GUI 和状态链路可运行，不能证明真实音频播放完成。

## 七、问题反思

B 侧前期采用 mock 数据推进 UI 是合理的，因为成员 A 的底层 Model/DSP 接口尚未稳定，如果等待底层完全完成会阻塞前端开发。但中期以后，mock 不能继续作为主要功能实现。当前项目最明显的问题是“界面像真实调音台，但音频链路还没有真实发声”，这会影响最终验收。

因此，下一阶段 B 侧需要和 A 侧协同推进真实链路：

1. B 侧在 QML 增加真实文件选择入口。
2. A 侧提供 WAV 解码或 PCM 数据读取接口。
3. B 侧将素材库导入入口从 mock 名称改为真实文件路径。
4. A 侧用 `QAudioSink` 实现真实输出。
5. B 侧将 waveform/spectrum/meter 从 mock 数据切换为 Model 提供的真实分析数据。
6. B 侧补充 EQ/Compressor/Bypass 控件，对接 A 侧 Sprint 3 已提供的接口。

## 八、个人小结

到中期为止，B 侧完成了从零到可运行调音台界面的搭建，并在 A 侧底层接口逐步完善后完成了部分真实接口对接。我的主要贡献集中在 ViewModel 状态设计、QML 界面实现、交互流程、报告证据和集成发布分支。当前成果足以支撑中期报告中关于协作、框架和过程完整性的要求，但距离真实可用的音频软件仍需要完成真实文件导入、音频输出和真实分析数据替换。

成员 B 主责：`ViewModel / View / Report`。

## 负责模块

- `src/ViewModel/`
- `src/View/`
- `Q_PROPERTY` 属性绑定
- 轨道状态机
- QML 主界面和控件
- 波形、VU、频谱显示
- 报告结构、截图证据和总报告整合

## 阶段记录模板

### 阶段：

- 日期：
- 使用的大模型：
- 采用模式：
- 提示词摘要：
- AI 输出内容：
- 人工修改内容：
- 自测结果：
- 成员 A 交叉测试结果：
- 对应提交：
- 可放入报告的证据：

## 阶段 0：B 侧渐进开发计划

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 辅助规划，人工确认后按小步提交实施。
- 提示词摘要：在成员 A 尚未提交底层实现前，规划成员 B 如何先推进 ViewModel、QML 和报告，并保持可回退的 commit 历史。
- AI 输出内容：
  - 先完成 B 侧计划和 AI 使用记录，建立报告证据。
  - 先使用 Mock 数据推进 UI 和 ViewModel，不阻塞等待 Model/DSP。
  - 每个阶段拆成可构建的小提交，避免一次性大提交。
  - 后续等成员 A 提供稳定接口后，再将 Mock 路径切换到真实 `AudioEngine` 数据。
- 人工修改内容：确认当前分支作为 B 侧开发分支继续推进。
- 自测结果：规划阶段不涉及构建。
- 成员 A 交叉测试结果：待成员 A 后续检查 B 侧接口边界。
- 对应提交：`d7704c2`
- 可放入报告的证据：本文件、`report/shared/AI_USAGE_LOG.md`、后续 git 提交历史。

## B 侧阶段拆分

1.  阶段 1：完善基础播放状态和调音台主界面骨架。
2.  阶段 2：完善轨道控制状态机，包括音量、Pan、Mute、Solo 的边界和回显。
3.  阶段 3：加入 Mock 波形、VU、频谱数据和可视化消费路径。
4.  阶段 4：加入素材库搜索、最近工程和工程恢复入口。
5.  阶段 5：在成员 A 接口稳定后，对接真实播放、导入、分析数据和工程数据。

## 阶段 1：基础播放状态与调音台界面骨架

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，当前环境尚未配置 Qt6，暂以静态检查和提交差异审查作为阶段验证方式。
- 提示词摘要：先实现 B 侧可独立推进的播放状态、Mock 进度和 QML 基础调音台布局，并逐次 commit。
- AI 输出内容：
  - `MixerViewModel` 增加 Mock 播放位置、总时长、进度比例和时间文本。
  - `MixerViewModel` 使用 `QTimer` 在播放时推进 Mock 播放位置。
  - `Main.qml` 从简单骨架调整为顶部状态区、播放控制区和轨道列表区。
  - 空轨道状态改用 `ListView.count` 判断，避免依赖 `QQmlListProperty.length`。
- 人工修改内容：确认当前阶段先记录 Qt6 环境缺口，是否安装 Qt 由后续本机验收需求决定。
- 自测结果：
  - 已运行 `git diff --check`，未发现空白或补丁格式问题。
  - 已尝试 `cmake -S . -B build`，当前环境缺少 Qt6 CMake 配置，因此无法完成构建验证。
  - 尚未配置 Qt6；如果需要本机运行、截图和完整验收，应安装 Qt6 或使用团队统一的 Qt6 构建环境。
- 成员 A 交叉测试结果：待成员 A 后续检查 QML 是否只通过 ViewModel 绑定。
- 对应提交：`f7245af`
- 可放入报告的证据：`MixerViewModel` 播放状态属性、`Main.qml` 调音台骨架、`git diff --check` 结果、Qt6 缺失的 CMake 输出摘要。

## 阶段 2：轨道控制状态机

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，人工通过构建结果和差异审查确认。
- 提示词摘要：继续推进 B 侧 TrackViewModel/MixerViewModel，实现更完整的轨道控制状态机和 QML 回显。
- AI 输出内容：
  - `TrackViewModel` 增加 `audible`、`volumeText`、`panText`，用于 UI 回显和状态表达。
  - `TrackViewModel` 在 Mute 状态变化时同步发出 `audibleChanged`。
  - `MixerViewModel` 增加 `anySolo` 和 Solo 刷新逻辑，实现 Solo 轨道监听时自动屏蔽非 Solo 轨道。
  - `Main.qml` 根据 `audible` 和 `solo` 调整轨道视觉状态，并显示音量百分比和 Pan 文本。
- 人工修改内容：确认 Solo 规则放在 MixerViewModel，单轨参数显示放在 TrackViewModel，QML 只做绑定和渲染。
- 自测结果：
  - 已运行 `git diff --check`，未发现空白或补丁格式问题。
  - 已运行 `cmake --build build-qt`，构建通过。
- 成员 A 交叉测试结果：待成员 A 检查 ViewModel/QML 是否越过 Model，Solo/Mute 规则是否符合接口预期。
- 对应提交：`7a25348`
- 可放入报告的证据：`TrackViewModel::audible()`、`MixerViewModel::refreshSoloState()`、QML 轨道状态回显、构建通过记录。

## 阶段 3：Mock 波形、频谱和电平可视化

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，人工通过 Qt 构建和差异审查确认。
- 提示词摘要：在成员 A 尚未提供真实分析数据前，由 B 侧先完成波形、频谱、轨道电平的数据消费路径和 QML 可视化。
- AI 输出内容：
  - `MixerViewModel` 增加 `waveformPoints` 和 `spectrumLevels`，使用确定性 Mock 数据模拟分析结果。
  - `TrackViewModel` 增加 `meterLevel` 和 `meterText`，用于单轨电平显示。
  - 播放计时器推进时刷新 Mock 波形、频谱和每轨电平。
  - `Main.qml` 增加波形 Canvas、频谱 Canvas 和单轨电平条。
- 人工修改内容：确认当前数据为 Mock，后续由成员 A 的真实波形/VU/频谱接口替换数据来源，QML 绑定接口保持稳定。
- 自测结果：
  - 已运行 `git diff --check`，未发现空白或补丁格式问题。
  - 已运行 `cmake --build build-qt`，构建通过。
- 成员 A 交叉测试结果：待成员 A 检查可视化数据是否只来自 ViewModel，后续对接真实分析数据。
- 对应提交：`10f525b`
- 可放入报告的证据：`MixerViewModel::updateMockAnalysisData()`、`TrackViewModel::meterLevel()`、QML Canvas 渲染、构建通过记录。

## 阶段 4：素材库搜索与工程入口

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 主导代码生成，人工通过 Qt 构建和差异审查确认。
- 提示词摘要：在成员 A 尚未提供 SQLite 素材库和 JSON 工程保存接口前，B 侧先完成素材搜索、导入入口、最近工程和工程恢复入口。
- AI 输出内容：
  - `MixerViewModel` 增加 `assetSearchText`、`filteredAssetNames`、`recentProjectNames`。
  - `MixerViewModel` 增加 `importAssetByName()`、`restoreRecentProject()`、`saveMockProject()` stub。
  - `Main.qml` 增加左侧素材库搜索列表、选中导入、最近工程恢复和保存快照入口。
  - 轨道区保留原有 ViewModel 绑定，导入素材后仍进入当前 Mock 轨道列表。
- 人工修改内容：确认素材和工程数据当前均为 Mock/stub，不直接实现 SQLite/JSON，也不越过 ViewModel 调用 Model。
- 自测结果：
  - 已运行 `git diff --check`，未发现空白或补丁格式问题。
  - 已运行 `cmake --build build-qt`，构建通过。
- 成员 A 交叉测试结果：待成员 A 检查后续素材库、工程保存/加载接口是否能替换当前 stub。
- 对应提交：`60e47c7`
- 可放入报告的证据：`MixerViewModel::refreshFilteredAssetNames()`、`MixerViewModel::saveMockProject()`、QML 素材/最近工程入口、构建通过记录。

## 阶段 4.1：自我 Review 后的界面重构

- 日期：2026-07-10
- 使用的大模型：Codex
- 采用模式：AI 自我 Review，人工指出界面层级问题后确认方案，再由 AI 修改实现。
- 提示词摘要：当前窗口排布不够像调音台，素材选择占据过多空间，Spectrum/Waveform 缺少坐标说明且尺寸不平衡，需要先反思再按确认方案改动。
- AI 输出内容：
  - 将窗口重构为顶部 Transport、中部 Analysis、底部 Mixer 主区和右侧 Library 侧栏。
  - 将素材库和最近工程入口压缩为右侧窄栏，避免占据主混音空间。
  - 将轨道列表改成横向 channel strip，使音量推子、电平、Pan、Mute、Solo 成为主体。
  - Waveform 和 Spectrum 改为等宽分析面板，并增加网格、时间/频率横轴、幅度/dB 纵轴和播放位置线。
- 人工修改内容：确认本次只重构 B 侧 QML 视觉和交互排布，不引入新的 A 侧底层依赖。
- 自测结果：
  - 已运行 `git diff --check`，未发现空白或补丁格式问题。
  - 已运行 `cmake --build build-qt`，构建通过。
  - 已启动 `build-qt/bin/MixingStudio`，无 QML 运行时错误输出。
- 成员 A 交叉测试结果：待成员 A 后续检查 QML 是否仍只通过 ViewModel 消费数据。
- 对应提交：`f055073`
- 可放入报告的证据：新主界面截图、Spectrum/Waveform 坐标轴截图、横向 channel strip 截图、构建通过记录。

## 阶段 5：中期接口集成与发布分支

- 日期：2026-07-14
- 使用的大模型：Codex
- 采用模式：AI 辅助分支检查、合并、构建验证和报告整理，人工确认走 GitHub PR 流程。
- 提示词摘要：检查当前仓库分支，理解合作者“中期弄好了”的含义；按 Sprint 2、Sprint 3 顺序打通接口并准备发布到主分支；后续改为推送集成分支，便于在 GitHub 上开 PR。
- AI 输出内容：
  - 检查本地与远程分支，确认 `origin/feature/A-model-dsp-sprint2-playback` 和 `origin/feature/A-model-dsp-sprint3-mixing` 尚未进入 `main`。
  - 创建 `release/midterm-integration` 本地集成分支。
  - 先合入 Sprint 2，生成 `b28ea5c merge: integrate sprint2 playback`。
  - 再合入 Sprint 3，生成 `194d823 merge: integrate sprint3 mixing`。
  - 验证 Sprint 2/3 后 `MixerViewModel` 已经将播放时钟、Seek、Master、Volume、Pan、Mute、Solo 等接口连接到 `AudioEngine`。
  - 推送远程分支 `origin/release/midterm-integration`，供后续 GitHub PR 使用。
- 人工修改内容：
  - 确认不直接推送或快进 `main`，而是保留 GitHub PR 协作流程。
  - 明确当前版本仍未实现真实音频解码和声卡输出，报告中不得将 mock 功能描述为真实播放。
- 自测结果：
  - macOS 上使用 Homebrew Qt 6.11.1 显式 CMake 路径完成配置。
  - `cmake --build build-midterm --config Debug --target MixingStudio` 构建通过。
  - `ctest --test-dir build-midterm -C Debug --output-on-failure` 通过，`dsp_processor` 和 `audio_engine` 两个测试均成功。
  - 本地启动 `build-midterm/bin/MixingStudio`，未出现终端 QML/runtime 错误。
- 成员 A 交叉测试结果：待成员 A 在 GitHub PR 中 review 集成分支；A 侧底层 Sprint 2/3 自测记录见 `report/member-A/README.md`。
- 对应提交：`b28ea5c`、`194d823`
- 可放入报告的证据：
  - GitHub 远程分支 `origin/release/midterm-integration`
  - CTest 2/2 通过输出
  - `git log --graph` 显示两次集成 merge
  - 本文件和 `report/README.md` 的中期报告内容
