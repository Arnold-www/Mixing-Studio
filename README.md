# Mixing Studio

基于 Qt 6、C++17、QML 和严格 MVVM 架构的多轨调音项目。

## 当前进度

**阶段一完成；阶段二播放闭环已提交（`b244414`）；阶段三 A 侧混音/DSP 在 `feature/A-model-dsp-sprint3-mixing`（轨参/EQ/压缩/Bypass 底层 + 离线混音限幅 + CTest）。** B 侧 EQ/Comp UI 与真 PCM 输出仍待后续。

## 目录结构

- `include/Model/`、`src/Model/` — 音频引擎（成员 A）
- `include/DSP/`、`src/DSP/` — DSP 算法（成员 A）
- `include/ViewModel/`、`src/ViewModel/` — 状态与命令转发（成员 B）
- `src/View/` — QML 界面（成员 B）
- `tests/` — 单元测试（`test_dsp_processor`、`test_audio_engine`）
- `scripts/` — 构建与验证脚本
- `docs/planning/` — 项目规划书
- `report/` — 过程记录与报告证据

## 协作模式

- **成员 A：** Model / DSP / Persistence
- **成员 B：** ViewModel / View / Report
- 每阶段交叉测试通过后合入 `main`

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