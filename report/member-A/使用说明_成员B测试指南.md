# Mixing Studio · macOS UI 使用说明（成员 B）

> **对象**：成员 B（macOS）  
> **内容**：如何在本机启动应用，以及主界面每个区域/控件做什么、怎么用  
> **配套**：[功能报告_后期集成与UI.md](./功能报告_后期集成与UI.md) · 交叉结果填 [CROSS_TEST_LOG.md](../shared/CROSS_TEST_LOG.md)  
> **日期**：2026-07-17

---

## 1. macOS 环境与启动

与 [TEST_AND_TOOLCHAIN.md](../shared/TEST_AND_TOOLCHAIN.md) 中成员 B 记录一致：

| 项 | 建议 |
|----|------|
| 系统 | macOS（如 15.x） |
| Qt | Homebrew `qtdeclarative`，或官方 Qt；常用前缀 `/opt/homebrew` |
| CMake | 3.21+ |
| 编译器 | Apple clang |

### 1.1 构建

```bash
cd /path/to/c++          # 仓库根目录
cmake -S . -B build-qt -DCMAKE_PREFIX_PATH=/opt/homebrew
cmake --build build-qt
```

若 Qt 装在其它路径：

```bash
export QT_PATH="$HOME/Qt/6.5.3/macos"   # 按本机实际修改
cmake -S . -B build-qt -DCMAKE_PREFIX_PATH="$QT_PATH"
cmake --build build-qt
```

### 1.2 启动应用

```bash
./build-qt/bin/MixingStudio
# 或（视 CMake 输出路径）
./build-qt/bin/Debug/MixingStudio
```

一键构建 + 测试（可选）：

```bash
chmod +x scripts/run_tests.sh
WITH_APP=1 ./scripts/run_tests.sh
```

架构边界检查（仓库根目录）：

```bash
# 若本机有 powershell / pwsh：
pwsh -File scripts/validate_feature.ps1
```

### 1.3 推荐试用素材

| 入口 / 路径 | 作用 |
|-------------|------|
| 顶栏 **Demo** | 快速加一轨演示音 |
| 顶栏 **Sample** | 加载样例工程（多轨） |
| `samples/*.wav` | 用 **Import** 选本地文件 |
| `samples/demo_session.json` | 经 Projects / Sample 相关流程打开 |

请先确认 macOS **系统音量**未静音，耳机/扬声器已选对输出设备。

---

## 2. 主界面总览

窗口大致分为五块（自上而下 + 左右抽屉）：

```
┌──────────────────────────────────────────────────────────────┐
│ ① TransportBar（顶栏）                                        │
│    品牌 | Library Projects Spectrum | Import…Export | 播控…   │
├──────────────────────────────────────────────────────────────┤
│ ② 波形舞台卡                                                   │
│    标题行 → ③ TimelineStrip（Seek / Song Loop）               │
│           → ④ WaveformPanel（波形 + 播放头）                   │
├──────────────────────────────────────────────────────────────┤
│ ⑤ TrackMixerList（每轨一条混音条 + 可展开 FX）                  │
└──────────────────────────────────────────────────────────────┘
        ← 左抽屉 Library / Projects          Spectrum 右抽屉 →
```

所有操作都通过界面绑定的 `mixerViewModel`，**不要**在 QML 里直接碰 Model/DSP。

---

## 3. 顶栏 TransportBar —— 各按钮做什么

文件：`src/View/TransportBar.qml`

### 3.1 抽屉开关

| 控件 | 作用 |
|------|------|
| **Library** | 打开/关闭左侧「素材库」抽屉，浏览可导入素材 |
| **Projects** | 打开/关闭左侧「工程」抽屉，最近工程 / 打开工程（与 Library 互斥，开一个关另一个） |
| **Spectrum** | 打开/关闭右侧「频谱」抽屉，看当前混音频谱 |

### 3.2 工程与素材

| 控件 | 作用 |
|------|------|
| **Import** | 弹出 macOS 文件选择框，选 `.wav` / `.mp3`（可多选）导入为新轨 |
| **Demo** | 导入一轨演示素材，适合快速试播控 |
| **Sample** | 加载内置样例工程（多轨会话），适合测混音/Loop |
| **Save** | 把当前会话存成工程（JSON） |
| **Export** | 把当前混音导出为立体声 WAV |

### 3.3 运输与主电平

| 控件 | 作用 |
|------|------|
| **Play / Pause** | 开始播放；播放中变为 Pause，暂停在当前位置 |
| **Stop** | 停止并回到起点 |
| **时间文字** | 显示当前时间 / 总时长（短素材可能显示 `0:00`，以 Seek 为准） |
| **Master 滑块** | 主输出音量 0–100% |
| **Master 数值框** | 键盘输入主音量（与滑块双向同步；回车或失焦生效） |
| **Master 电平表** | 主混音响度条；过高（约超过 85%）填充变**红**，正常为青绿 |

---

## 4. 波形舞台 —— 看什么、点哪里

### 4.1 标题行

| 元素 | 作用 |
|------|------|
| **Waveform · overview / live** | 提示当前是总览波形还是播放相关状态 |
| **中间 statusMessage** | 引擎/ViewModel 状态提示（导入成功、错误等） |
| **右侧 Track N · click plot…** | 提示已选中哪一轨；可在波形上点选编辑自动化 |

### 4.2 TimelineStrip（Seek + Song Loop）

文件：`src/View/TimelineStrip.qml`

| 控件 | 作用 |
|------|------|
| **Seek 标签 + 滑条** | 拖到任意位置跳转播放头；播放时应与声音同步前进 |
| **I / O 数值框** | 仅在 **Song Loop 开启**时显示；用百分比设置运输循环入点/出点（0–100） |
| **Song Loop 按钮** | 开关「整首歌时间轴循环」：到 O 点后回到 I 点继续播；关闭则播到曲末停止 |

**Song Loop ≠ 轨上的 Clip Loop**（见下一节）。

### 4.3 WaveformPanel

文件：`src/View/WaveformPanel.qml`

| 元素 | 作用 |
|------|------|
| **波形曲线** | 当前会话/选中轨的总览波形 |
| **播放头竖线** | 当前可听位置，应与 Seek 对齐 |
| **点击波形** | 在已选轨上编辑音量等自动化（按标题行提示操作） |

---

## 5. 轨混音条 TrackMixerList —— 每轨控件

文件：`src/View/TrackMixerList.qml`  
底部区域：一行一轨。点某一轨可选中（影响波形侧提示与自动化）。

### 5.1 左侧：静音 / 独奏 / 效果 / 循环模式

| 控件 | 作用 |
|------|------|
| **M（Mute）** | 静音该轨，混音里听不到 |
| **S（Solo）** | 独奏；通常只听 Solo 打开的轨 |
| **FX** | 展开/收起该轨效果条（EQ + 压缩） |
| **Clip \| Loop**（分段开关） | **Clip**：正常播素材一次；**Loop**：该轨内容在轨 I–O 内循环（**不**强迫整曲时间轴回头） |

切到 **Loop** 后，可调该轨自己的 **I / O（%）**，决定素材循环区间。

### 5.2 中部：音量 / 声像 / 速度

| 控件 | 作用 | 范围（界面） |
|------|------|----------------|
| **Vol 滑块 + 数值** | 轨音量 | 0–100（%） |
| **Pan 滑块 + 数值** | 左右声像；0 居中，负偏左，正偏右 | -100–100 |
| **Speed 滑块 + 数值** | 播放速率（变调随实现） | 约 0.50–2.00 |

数值框：点进去用键盘改，**回车或点别处**提交；编辑过程中滑块不会把输入顶掉。

### 5.3 右侧：轨电平表

| 控件 | 作用 |
|------|------|
| **竖条电平** | 显示该轨相关响度（当前与主混音能量相关）；过高变红，同 Master |

### 5.4 FX 展开区（点 FX 后）

| 控件 | 作用 |
|------|------|
| **EQ 1–10** | 十段均衡；每列：上方数值（dB）、中间竖推子、下方频段号。调高/低该频段增益 |
| **Thr** | 压缩器**阈值**：信号超过后才开始压。数值越小越容易触发压缩 |
| **Rat** | 压缩器**压缩比**：超过阈值后压多少（如 3 ≈ 3:1） |

关闭 FX 面板时，效果按现有逻辑旁路（Bypass），接近干声。

---

## 6. 左右抽屉

### 6.1 Library（左）

文件：`src/View/LibraryPanel.qml`

- 浏览素材库列表  
- 选中后导入到当前会话（具体按钮文案以面板为准）  
- 用于不经过系统文件框、从库里加轨  

### 6.2 Projects（左）

文件：`src/View/ProjectsPanel.qml`

- 最近工程、打开已有工程  
- 与顶栏 **Save** 配合：改完参数 → Save → 这里再打开验证是否恢复  

### 6.3 Spectrum（右）

文件：`src/View/SpectrumPanel.qml`

- 显示当前混音频谱  
- 面板内 **Close** 或再点顶栏 Spectrum 可关闭  

---

## 7. 两个 Loop 怎么配合用（必读）

| 名称 | 在哪 | 调什么 |
|------|------|--------|
| **Song Loop** | Seek 行右侧 | **整条时间轴**是否在 I–O 内循环 |
| **Clip Loop** | 每轨 `Clip \| Loop` | **这一轨的素材**是否在轨 I–O 内循环 |

常见用法：

1. 只想某轨鼓循环、整曲播完停 → 该轨 **Loop**，**Song Loop 关**。  
2. 整段副歌区间反复听 → **Song Loop 开**，设好 Seek 旁 I/O。  
3. 两者可同时开：时间轴按 Song 回头，各轨仍按自己的 Clip Loop 读内容。

---

## 8. 推荐操作流程（第一次上手）

1. 启动 `MixingStudio`。  
2. 点 **Sample**（或多 **Import** 几段 WAV）。  
3. 点 **Play**，听声音；看 Seek 与波形播放头是否一起动。  
4. 拖 **Seek**，再调 **Master** / 某轨 **Vol**，确认音量与电平条变化。  
5. 某轨点 **FX**，拧几个 EQ 或 **Thr/Rat**，对比听感。  
6. 试 **Clip Loop** 与 **Song Loop**（先分开试，再一起试）。  
7. **Save** → 退出再开 → **Projects** 打开，确认参数还在。  
8. **Export** 听一下导出的 WAV。  

---

## 9. 交叉验收简表（测完打勾）

在 macOS 上按界面测即可；结果写入 `CROSS_TEST_LOG.md`。

| # | 操作 | 期望 | ☐ |
|---|------|------|---|
| 1 | Demo/Sample/Import + Play | 有声，Seek 跟声音走 | ☐ |
| 2 | Pause / Stop / 拖 Seek | 暂停续播、Stop 归零、Seek 跳转合理 | ☐ |
| 3 | Song Loop 开/关 | 开则区间循环；关则曲末停 | ☐ |
| 4 | 一轨 Clip Loop、一轨不 Loop | 开环轨循环内容；曲末是否回头只看 Song Loop | ☐ |
| 5 | M / S / Pan / Speed / FX | 听感符合控件含义 | ☐ |
| 6 | 电平推高 | Master/轨条过高变红 | ☐ |
| 7 | Save / 再打开 / Export | 工程恢复；WAV 可播 | ☐ |
| 8 | QML 不直连 Model | `validate_feature` 或代码审查通过 | ☐ |

---

## 10. macOS 常见问题

| 现象 | 处理 |
|------|------|
| CMake 找不到 Qt6 | 设 `CMAKE_PREFIX_PATH=/opt/homebrew` 或本机 `QT_PATH` |
| 有界面无声音 | 查系统输出设备、Master、轨 Mute、是否未 Play |
| Clip Loop 开了曲却回头 | 多半同时开了 Song Loop，先关掉 Song Loop 再听 |
| 时间一直 `0:00` | 素材很短时正常；以 Seek 与耳朵为准 |
| 电平一直很矮 | 素材本身轻或推子低；拉高 Vol/Master 再看是否变红 |

Windows 侧细节（`run_app.ps1`、LNK1168 等）由成员 A 自测；B 侧日志请写明 **平台 = macOS** 与 Qt 版本。

---

## 11. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-07-17 | 初版：Windows 向测试清单 |
| 2026-07-17 | 改为 **macOS 启动 + 全 UI 组件作用说明**，验收表精简附后 |
