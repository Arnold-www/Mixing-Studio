# Mixing Studio

基于 Qt 6、C++17、QML 和严格 MVVM 架构的多轨调音项目。

## 当前进度

**阶段一～三 A 侧已推送；阶段四 A 侧分析/持久化在 `feature/A-model-dsp-sprint4-analysis`（波形/VU/频谱/削波 + SQLite/JSON，CTest 4/4）。** B 侧仍用 Mock 可视化，改绑待交叉。

## 目录结构

- `include/Model/`、`src/Model/` — 音频引擎（按 Playback/Tracks/Analysis/Persistence 拆分）
- `include/DSP/`、`src/DSP/` — `DspProcessor`（处理）+ `DspAnalysis`（分析）
- `include/App/`、`src/App/` — MixerApp 用例（Playback/Project/Tracks 拆分）
- `include/Command/`、`src/Command/` — Playback / Project / TrackDsp 命令
- `include/ViewModel/`、`src/ViewModel/` — UI 状态（含 Sync 拆分）
- `src/View/` — Main 组装 + TransportBar / Waveform / Spectrum / Library / Mixer 组件
- `tests/` — 单元测试（含 `test_mixer_app`、`test_commands`）
- `scripts/` — 构建与验证脚本
- `docs/planning/` — 项目规划书
- `report/` — 过程记录与报告证据

## 协作模式

- **成员 A：** Model / DSP / Persistence
- **成员 B：** Command / App 对接、ViewModel / View / Report
- 每阶段交叉测试通过后合入 `main`
- 调用链：`QML → ViewModel → Command → App → Model/DSP`

## 脚本分类

详见 `[scripts/README.md](scripts/README.md)`：


| 类别    | 脚本                               | 用途                  |
| ----- | -------------------------------- | ------------------- |
| 环境配置  | `configure_qt_windows.ps1`       | 首次 Windows Qt 配置与构建 |
| 构建与测试 | `run_tests.ps1` / `run_tests.sh` | 一键构建 + CTest        |
| 质量互测  | `validate_feature.ps1`           | MVVM 架构边界检查         |




## 本地运行（Windows）

**前置条件：** Qt 6.5+、CMake 3.21+、Visual Studio 2022（MSVC）。首次环境搭建见 `[scripts/README.md](scripts/README.md)`。

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp
.\build\bin\Debug\MixingStudio.exe
```



## 截止时间

最终提交：7 月 18 日；7 月 17 日进入冻结期。