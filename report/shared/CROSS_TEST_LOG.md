# 交叉测试记录

每个阶段合入主分支前，必须由非主责成员测试通过并记录。

| 日期 | 阶段 | 实现成员 | 测试成员 | 测试范围 | 发现问题 | 修复结果 | 是否允许合入主分支 | 对应提交 |
| :--- | :--- | :------- | :------- | :------- | :------- | :------- | :----------------- | :------- |
| 2026-07-11 | B 侧 feature / `chai/feat` | 成员 B | 成员 A | 按规划检查 ViewModel/QML 分层、头文件目录、B 侧分支范围、报告证据文件和 CMake 头文件配置；运行 `scripts/validate_feature.ps1` 和 Windows Qt 构建脚本 | 首次 CMake 配置因 Windows 未配置 Qt6 失败；安装 Qt 6.5.3 后复测通过 | 保留 `scripts/validate_feature.ps1` 和 `scripts/configure_qt_windows.ps1`；静态验证 13 项通过；`cmake --build build --config Debug` 通过并生成 `MixingStudio.exe` | 是，静态验证和 Windows Qt 构建均通过 | `4fcbeaa` |
| 2026-07-11 | 阶段 1：基建与发声（A+B 本地 merge） | 成员 A + 成员 B | 成员 A（本地）→ 成员 B（macOS 复测） | merge `chai/feat` 到 `feature/A-model-dsp-sprint1-infra`；`run_tests.ps1 -WithApp`；`validate_feature.ps1`；审查 VM→Model 调用链；B 侧 macOS 构建与 CTest | 无阻塞问题 | 不适用 | **是，中期已实现项互相通过** | `2654de0`、`9cac45c`、集成分支复测 |
| 2026-07-11 | 阶段 2：播放闭环 | 成员 A | 成员 A（自测）→ 成员 B（UI/集成分支） | `AudioEngine` 导入/播放/Seek/Loop；`test_audio_engine`；VM 进度对接 Model；B 在集成分支验证播控 UI | 无 | 不适用 | **是，中期已实现项互相通过** | `b244414`、`b28ea5c` |
| 2026-07-14 | 阶段 3：混音与 DSP | 成员 A | 成员 A（自测）→ 成员 B（轨参 UI） | DSP EQ/压缩/混音/限幅；`renderMixFrame` Mute/Solo/Pan；Volume/Pan/Mute/Solo 经 VM 同步；`validate_feature` | EQ/Comp UI 不在中期交付 | 不适用 | **是：轨参/混音底层与已挂 UI 互相通过**；EQ/Comp 控件属后期 | `7291c53`、`194d823` |
| 2026-07-14 | 阶段 4：分析与持久化（仅 A） | 成员 A | 成员 A（自测） | 波形/VU/频谱/削波；JSON 工程；SQLite 素材库；CTest 4/4 | 无阻塞；B 仍用 Mock 可视化 | 不适用 | 未纳入中期集成分支；待后期 B 改绑后再交叉 | 待提交 |
| 2026-07-16 | 阶段 4：RealVM 改绑闭环 | 成员彭 | 待成员张审核 | RealVM 接 `refreshAnalysis`/`ProjectStore`/`AssetLibrary`；QML VU；CTest 6/6 | 无 | 不适用 | 待审核通过后合入 | `37f8b8c` |
| 2026-07-16 | 阶段 5：WAV 导出与交付清单 | 成员彭 | 待成员张审核 | `WavExporter`/`exportMixToWav`、Export UI、DSP 补强、架构清单；CTest 含 `wav_export` | 无 | 不适用 | 待审核通过后合入 | 待提交 |
| 2026-07-16 | 真实音频链路 | 成员彭 | 待成员张审核 | `WavDecoder`、FileDialog 导入、`QAudioSink`、真实分析/导出、EQ/Comp UI；CTest 8/8 | 无 | 不适用 | 待审核 | 待提交 |
| 2026-07-16 | 四项缺口 + MP3 | 成员彭 | 待成员张审核 | Loop UI、最近工程扫描、样例工程、Mock 模式、MP3 解码 | 无 | 不适用 | 待审核 | 待提交 |
| 2026-07-17 | 后期集成分支独立 Review 与回归 | 成员 A（后期实现） | 成员 B | 相对 `origin/release/midterm-integration` 的完整差异；macOS 干净构建；CTest；应用/QML 启动冒烟；WAV→工程→导出 E2E；仓库样例工程 E2E；QML lint | 发现音频输出格式错配、测试脚本吞错/漏测、WAV 非法位深假成功、WAV fixture 假阳性、QML undefined 颜色、SQLite 连接释放、工程同名覆盖；另确认十段 EQ 只是整体增益代理 | 已修复可局部确认的缺陷并补回归；CTest **11/11**、完整构建、app smoke、样例工程导出与 `git diff --check` 通过；真实 EQ 缺口保留 | **有条件允许**：自动化主链通过；真实 EQ 不得按十段频率均衡验收，声卡/MP3/完整 GUI 点击仍需专项人工测试 | 待提交 |

## 检查重点

- QML 是否直接访问 Model 或 DSP。
- ViewModel 是否正确转发命令和状态。
- Model/DSP 是否按接口返回可消费数据。
- 音频参数是否生效。
- 工程保存、加载和导出是否正确。
- 测试结果是否能进入报告。

## 阶段 1 检查项（A 测 B，本地 merge 后）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| QML 不直接访问 Model/DSP | ✅ | `validate_feature.ps1` 通过 |
| Model/DSP 不依赖 View/ViewModel | ✅ | `validate_feature.ps1` 通过 |
| `MixerViewModel` 转发 play/pause/stop | ✅ | 调用 `m_audioEngine->play()` 等 |
| `importTrack` 由 VM 触发 | ✅ | `importMockTrack` / `importAssetByName` |
| 播放状态信号连接 | ✅ | `playbackStateChanged` → `playingChanged` |
| 合并后构建 | ✅ | `MixingStudio.exe` + `test_dsp_processor` |

## 阶段 1 检查项（B 测 A，已通过）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| `AudioEngine` 接口可被 VM 调用 | ✅ | B 在 macOS 集成分支构建并启动验证 |
| `DspProcessor` 单元测试 | ✅ | CTest `dsp_processor` Passed |
| 主音量 clamp [0,1] | ✅ | UI Master 可调；底层 clamp 生效 |
| 合并后 macOS 全量构建 | ✅ | `MixingStudio` + CTest 2/2 |

## 阶段 2 检查项（B 测 A，已通过）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| Import 后 `durationMs` 为 180s 占位 | ✅ | UI 时间码约 3:00 |
| Play 后进度条随 Model 时钟前进 | ✅ | 集成分支上由 Model 时钟驱动 |
| Seek 拖动进度条调用底层 Seek | ✅ | 进度可跳转 |
| Pause/Stop 状态与位置正确 | ✅ | 播控可用 |
| CTest `audio_engine` 通过 | ✅ | macOS / Windows 均为 Passed |

## 阶段 3 检查项（A 自测 + B 对已挂 UI 交叉）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| 三段 EQ / 压缩器 / Bypass DSP | ✅ | `test_dsp_processor`（底层） |
| 多轨线性混音 + 主限幅 | ✅ | `mixLinear` + `applyMasterChain` |
| Mute / Solo 影响 `renderMixFrame` | ✅ | `test_audio_engine` |
| 轨音量 / Pan 进入 Model | ✅ | `setTrackVolume` / `setTrackPan` + VM sync；B UI 交叉通过 |
| Mute / Solo UI → Model | ✅ | B 在集成分支验证 channel strip |
| QML 仅经 ViewModel | ✅ | `validate_feature` 13/13 |
| CTest 全绿 | ✅ | 2/2 |
| EQ/Comp/Bypass UI 交叉 | — | **不在中期交付**；后期 B 挂控件后再测 |

## 阶段 4 检查项（A 自测）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| 波形降采样 / VU / 频谱 / 削波 | ✅ | `test_dsp_processor` |
| `AudioEngine::refreshAnalysis` | ✅ | 64 / 18 bins |
| JSON 工程 round-trip | ✅ | `test_project_store` |
| SQLite 素材 upsert/搜索/recent | ✅ | `test_asset_library` |
| CTest 全绿 | ✅ | 4/4 |
| `validate_feature` | ✅ | 13/13 |
| B 改绑分析/工程 UI | ✅（彭实现，待张审核） | `RealMixerViewModel` 消费 Engine 分析/JSON/SQLite；Transport VU |

## 一键命令

```powershell
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

```bash
WITH_APP=1 ./scripts/run_tests.sh
```
