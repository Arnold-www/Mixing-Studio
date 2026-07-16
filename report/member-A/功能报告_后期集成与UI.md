# 成员 A · 功能报告（后期集成与 UI / 播放闭环）

> **角色**：成员 A（Model / DSP / 播放引擎 / 与 View 对接的功能实现）  
> **配套文档**：给成员 B 的测试说明见同目录 [使用说明_成员B测试指南.md](./使用说明_成员B测试指南.md)  
> **日期**：2026-07-17  
> **范围**：本阶段在既有五层 MVVM 骨架上完成的可运行功能（含夜间桌面 UI、双层 Loop、Seek/电平、可编辑参数等）

---

## 1. 报告目的

汇总当前工程中**已落地、可演示、可交叉测试**的功能点，说明设计意图、实现位置、行为约定与已知边界，供中期后集成验收与成员 B 复测使用。

架构约束仍以 [docs/PROJECT_STRUCTURE.md](../../docs/PROJECT_STRUCTURE.md) 与 [docs/ARCHITECTURE_CHECKLIST.md](../../docs/ARCHITECTURE_CHECKLIST.md) 为准：QML 只通过 `mixerViewModel`（`IMixerViewModel*`）交互。

---

## 2. 功能总览

| 编号 | 功能域 | 一句话说明 | 主要落点 |
|------|--------|------------|----------|
| F01 | 夜间桌面壳 | Material Dark + 深色主题，主舞台 + 左右抽屉 | `Main.qml` / `Theme.qml` / `TransportBar.qml` |
| F02 | 运输条 | Play/Pause/Stop、时间、Master 滑块与数值、电平表、导入/工程入口 | `TransportBar.qml` |
| F03 | 波形与 Seek | 总览波形、Seek 条、播放头与 Seek 对齐 | `WaveformPanel.qml` / `TimelineStrip.qml` / `AudioEngine_Playback.cpp` |
| F04 | Song Loop | 全局运输循环：时间轴在 I–O 内回头 | `TimelineStrip.qml` + `setLoopRange` |
| F05 | Clip Loop | 每轨素材循环：仅该轨内容在 I–O 内循环 | 轨条 UI + `AudioTrack::loopEnabled` |
| F06 | 可编辑参数 | Vol/Pan/Speed/Master/EQ/Loop I–O 可键盘输入 | `NumericValueField.qml` |
| F07 | 轨混音条 | M/S/FX、电平表、EQ/压缩推子 | `TrackMixerList.qml` / `ClipLoopToggle.qml` |
| F08 | 电平与过载色 | dB 映射显示；超阈值（0.85）填充变红 | `DspAnalysis` + `LevelMeterBar.qml` |
| F09 | 真实音频 | WAV/MP3 导入、`QAudioSink` 播出、工程存取、导出 | `AudioFileDecoder` / `ProjectStore` / `WavExporter` |
| F10 | 播放时钟 | 以设备 `processedUSecs` + 插值驱动 UI，避免缓冲导致 Seek「晚动」 | `AudioEngine_Playback.cpp` |

---

## 3. 分功能详细说明

### 3.1 应用壳与布局（F01）

**目标**：调音台主界面可读、可操作，避免控件互相遮挡。

**实现要点**：

- 顶栏 `TransportBar`：Library / Projects / Spectrum 抽屉开关；Import / Demo / Sample / Save / Export；运输与 Master。
- 中部舞台：Waveform 标题行 + `TimelineStrip`（Seek / Song Loop）+ `WaveformPanel`。
- 底部：`TrackMixerList` 全宽轨条。
- 左右抽屉：`LibraryPanel` / `ProjectsPanel` / `SpectrumPanel`。
- 布局禁忌：作为 `Layout` 子项的组件**禁止**再 `anchors.fill` 逃逸布局（曾导致 Loop/频谱标题被盖住，已修复）。

**主题**：`Theme.qml` 单例 + Material Dark；工具按钮用 `ToolIconButton.qml`。

---

### 3.2 运输与 Master（F02）

| 控件 | 行为 |
|------|------|
| Play / Pause | 有轨才可播；Pause 停在**可听位置**（非写缓冲超前位置） |
| Stop | 停并归零 |
| 时间文本 | `playbackTimeText`（秒级显示；短素材可能显示 0:00） |
| Master 滑块 + 数值框 | 0–100%，双向绑定 `masterVolume` |
| Master 电平表 | 见 F08 |

---

### 3.3 Seek 与播放头同步（F03 / F10）

**问题背景**：

1. Seek 条与波形区左右边距不一致 → 滑块与播放头视觉错位（已对齐 insets）。  
2. `QAudioSink` 预填缓冲时，写指针超前 → Seek 看起来「声音走完才动」（已改为可听时钟）。  
3. WASAPI `processedUSecs` 块状更新 → Seek 顿挫（已用墙钟插值 + ~120Hz UI tick）。

**当前约定**：

- UI `playbackProgress` / `positionMs()` 读的是**可听位置**（`audiblePositionUSecs`）。
- 内部混音写头仍用 `m_playheadSamples` / `m_positionMs` 成员。
- Seek 行布局：`Seek` 标签 → 滑条 `fillWidth` →（Song Loop 开时）I/O 数值框 → `Song Loop` 按钮；I/O 框**不再叠在**滑条上。
- 滑条 groove 左右留 handle 边距，避免 Out「O」被裁切。

---

### 3.4 双层 Loop（F04 / F05）——方案 C

| 层级 | UI 名称 | 作用 |
|------|---------|------|
| 运输层 | **Song Loop** | 时间轴到 I–O（会话 `loopStartMs`/`loopEndMs`）是否回头 |
| 内容层 | **Clip Loop**（轨条分段开关） | 仅该轨 PCM 在轨 I–O 百分比内循环 |

**混用规则（验收重点）**：

- 仅开 Clip Loop、关 Song Loop：开环轨在会话时长内循环内容；未开环轨播完静音；**到曲末停止**（不回头）。
- 开 Song Loop：时间轴在运输 I–O 内循环；各轨仍按各自 Clip Loop 读数。
- **禁止**「任意轨开 Clip Loop 就强迫时间轴回头」（旧逻辑已去掉）。

**持久化**：轨 `loopEnabled` / `loopStart` / `loopEnd`；会话 `loopStartMs` / `loopEndMs`（见 `ProjectStore`）。

---

### 3.5 可编辑数值（F06）

组件：`NumericValueField.qml`（失焦/回车提交；编辑中不被滑块覆盖）。

| 参数 | 显示 | 范围 |
|------|------|------|
| Master / Vol | 整数 % | 0–100 → 0.0–1.0 |
| Pan | 整数（×100） | -100–100 → -1.0–1.0 |
| Speed | 两位小数 | 0.50–2.00 |
| EQ 1–10 | 一位小数 dB | -12–12 |
| Thr / Rat | 两位小数 | 见引擎 clamp |
| Clip / Song Loop I–O | 整数 % | 0–100 → 0.0–1.0 |

轨条布局：左侧 M/S/FX/`ClipLoopToggle`；中间 Vol/Pan/Speed（加长滑块 + 弹簧均分）；右侧电平表。

---

### 3.6 FX / EQ（F07）

- **FX** 展开：10 段图形 EQ + Thr + Rat 垂直推子。
- 每频段为独立 `ColumnLayout`（上：数值框，中：竖滑块，下：标签），`RowLayout` 均分整行宽度。
- Bypass：关闭 FX 面板时写回 `fxBypass`（与现有逻辑一致）。

---

### 3.7 电平表与过载色（F08）

**引擎侧**：

- `DspAnalysis::computeDisplayMeter(peak, rms)`：约 -42 dBFS～0 dBFS 映射到 0～1，避免线性 RMS「听着很大、条很短」。
- 快攻慢放球istics，更新约 120Hz 分析路径。
- 轨电平**不再**对混音 VU 二次乘 `volume`（混音已含推子）。

**UI 侧**：`LevelMeterBar.qml`

```text
property double threshold: 0.85
color: currentLevel > threshold ? "#ef4444" : "#3fb59f"
```

应用于：Master 电平表、每轨右侧电平表。

---

### 3.8 音频与工程（F09）

- 导入：FileDialog（WAV/MP3）、Demo、Sample 工程、素材库选中后 Import。
- 解码：`AudioFileDecoder` / `WavDecoder`。
- 播出：`QAudioSink` + `MixAudioDevice` 拉流混音；缓冲约 120ms。
- 工程：JSON 保存/加载（含轨参、自动化、Loop）。
- 导出：立体声 WAV（`WavExporter`）。

---

## 4. 关键源码索引

| 类别 | 路径 |
|------|------|
| 播放 / Seek / Song Loop | `src/Model/AudioEngine_Playback.cpp` |
| 轨 Loop / 混音读数 | `include/Model/AudioTrack.h`、`src/Model/AudioEngine_Tracks.cpp` |
| 电平映射 | `src/DSP/DspAnalysis.cpp`（`computeDisplayMeter`） |
| ViewModel | `src/ViewModel/RealMixerViewModel.cpp`、`TrackViewModel.cpp` |
| 主 UI | `src/View/Main.qml`、`TransportBar.qml`、`TimelineStrip.qml`、`TrackMixerList.qml` |
| 组件 | `NumericValueField.qml`、`LevelMeterBar.qml`、`ClipLoopToggle.qml`、`Theme.qml` |
| 工程 | `src/Model/ProjectStore.cpp` |
| 测试 | `tests/test_audio_engine.cpp`、`tests/test_wav_*.cpp`、`scripts/validate_feature.ps1` |

---

## 5. 自测与工具链（A 侧）

- 启动（推荐）：`.\scripts\run_app.ps1`（剥离 Conda `Library\bin`，避免 Qt 插件冲突）。
- 单元测试：`.\scripts\run_tests.ps1` 或构建后运行 `build\Debug\test_*.exe`。
- 架构边界：`.\scripts\validate_feature.ps1`。
- 重建注意：关闭正在运行的 `MixingStudio.exe`，否则 LNK1168 无法写 exe。

---

## 6. 已知限制与说明

1. **时间显示**为整秒：时长 &lt; 1s 的素材可能显示 `0:00 / 0:00`，进度条仍可用。  
2. **轨电平**当前基于**主混音**分析（可听轨显示同一混音能量）；非完全独立的每轨峰值表。  
3. **Mock** 模式用合成分析数据，不代表真实设备电平。  
4. Song Loop 的 I/O 数值框仅在 Song Loop **开启**时显示。  
5. 自动化仍在波形区点击编辑（标题行提示）；与 Clip Loop 独立。

---

## 7. 给成员 B 的交接要点

1. 验收双层 Loop 时务必做「一轨开 Clip、一轨关 + Song 开/关」四象限，见测试指南 §4。  
2. Seek 应与声音同步前进；若回退到旧缓冲写头逻辑会复现「晚动」。  
3. 电平超约 85% 显示单位应变红（`LevelMeterBar`）；低于阈值为青绿。  
4. 交叉测试结果请记入 [report/shared/CROSS_TEST_LOG.md](../shared/CROSS_TEST_LOG.md)。

---

## 8. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-07-17 | 初版：汇总后期集成 UI、双层 Loop、Seek/电平、可编辑参数与测试交接 |
