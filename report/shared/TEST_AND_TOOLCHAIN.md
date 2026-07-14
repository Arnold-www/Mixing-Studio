# 测试与工具链记录

本文件用于记录报告中需要体现的单元测试、其他测试、构建工具和持续集成情况。

## 构建环境

### macOS（成员 B）

- 操作系统：macOS 15.7.4
- Qt 版本：Qt 6.11.1，通过 Homebrew `qtdeclarative` 安装，包含本项目需要的 Qt QML、Qt Quick、Qt Quick Controls 2。
- CMake 版本：4.3.4
- 编译器：Apple clang 17.0.0
- 构建命令：

```bash
cmake -S . -B build-qt -DCMAKE_PREFIX_PATH=/opt/homebrew
cmake --build build-qt
```

- 说明：未安装完整 `qt` 元包；当前最小可用路径为 `brew install qtdeclarative`。该命令会安装 `qtbase`、`qtsvg` 等必要依赖。

### Windows（成员 A）

- 操作系统：Windows 10.0.26200
- Qt 版本：6.5.3 (msvc2019_64)，通过 `aqtinstall` 安装于 `D:\Qt\6.5.3\msvc2019_64`
- CMake 版本：3.31.3
- 编译器：MSVC 17.14 (Visual Studio 2022)
- 当前分支：`feature/A-model-dsp-sprint2-playback`（基于阶段 1 + `chai/feat`）
- 构建命令：

```powershell
.\scripts\configure_qt_windows.ps1
# 或
cmake -S . -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Debug
```

## 一键测试

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp
.\scripts\validate_feature.ps1 -BaseBranch main
```

```bash
WITH_APP=1 ./scripts/run_tests.sh
```

## 测试记录

| 日期 | 测试类型 | 测试对象 | 命令/步骤 | 结果 | 问题 | 修复提交 |
| :--- | :------- | :------- | :-------- | :--- | :--- | :------- |
| 2026-07-10 | 构建测试 | Qt/CMake 工具链（macOS） | `cmake -S . -B build-qt -DCMAKE_PREFIX_PATH=/opt/homebrew` | 通过 | 初始环境缺少 Qt6；安装 `qtdeclarative` 后通过 | `7739f16` |
| 2026-07-10 | 构建测试 | B 侧阶段 2 ViewModel/QML | `cmake --build build-qt` | 通过 | 无 | `7a25348` |
| 2026-07-10 | 构建测试 | B 侧阶段 3 可视化 | `cmake --build build-qt` | 通过 | 无 | `10f525b` |
| 2026-07-10 | 构建测试 | B 侧阶段 4 素材/工程入口 | `cmake --build build-qt` | 通过 | 无 | `60e47c7` |
| 2026-07-10 | 构建与运行测试 | B 侧阶段 4.1 UI 重构 | `cmake --build build-qt`；启动 `build-qt/bin/MixingStudio` | 通过 | 无 | `f055073` |
| 2026-07-11 | 静态互测脚本 | `chai/feat` MVVM 分层、头文件目录、B 侧分支范围、报告证据文件 | `powershell -ExecutionPolicy Bypass -File scripts\validate_feature.ps1` | 通过，13 项检查全部通过 | 无 | `4fcbeaa` |
| 2026-07-11 | Windows CMake 配置检查 | 本机 Qt/CMake 工具链 | `cmake -S . -B build` | 未通过 | Windows 当前未配置 Qt6，CMake 找不到 `Qt6Config.cmake` | 环境问题，非 feature 代码修复 |
| 2026-07-11 | Windows Qt 环境配置 | Qt 6.5.3 MSVC2019 64-bit | `python -m aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -O "D:\Qt"` | 通过 | 首次尝试单独安装 `qtdeclarative` 模块失败；基础 Qt 安装包已包含 QML/Quick/QuickControls2 | 环境配置 |
| 2026-07-11 | Windows 构建测试 | `chai/feat` 当前工程 | `cmake -S . -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.5.3\msvc2019_64"`；`cmake --build build --config Debug` | 通过，生成 `build/bin/Debug/MixingStudio.exe` | 无 | `dbe9213` |
| 2026-07-11 | 单元测试 | A 侧阶段 1 DSP 骨架 | `.\scripts\run_tests.ps1 -WithApp` | 通过，1/1 | 无 | `2654de0` |
| 2026-07-11 | 架构检查 | A+B 阶段 1 合并后 MVVM 边界 | `.\scripts\validate_feature.ps1` | 通过，13/13 | 无 | `9cac45c` |
| 2026-07-11 | 集成构建 | A+B 阶段 1 本地 merge | merge `chai/feat` 后全量构建 | 通过 | 无 | `9cac45c` |
| 2026-07-11 | 单元测试 | A 侧阶段 2 播放闭环 | `ctest --test-dir build -C Debug --output-on-failure` | 通过，2/2 | 无 | `b244414` |
| 2026-07-11 | 集成构建 | A 侧阶段 2 App + Model 时钟对接 | `cmake --build build --config Debug` | 通过，生成 `MixingStudio.exe` | 无 | `b244414` |
| 2026-07-14 | 单元测试 | A 侧阶段 3 混音/EQ/压缩/限幅 | `.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64"` | 通过，2/2 | 无 | 待提交 |
| 2026-07-14 | 集成构建 | A 侧阶段 3 MixingStudio | `cmake --build build --config Debug --target MixingStudio` | 通过 | 无 | 待提交 |
| 2026-07-14 | 架构检查 | 阶段 3 MVVM 边界 | `.\scripts\validate_feature.ps1` | 通过，13/13 | 无 | 待提交 |

## 最近一次 CTest 输出（A 侧阶段 3）

```text
1/2 Test #1: dsp_processor ....................   Passed    0.01 sec
2/2 Test #2: audio_engine .....................   Passed    0.49 sec
100% tests passed, 0 tests failed out of 2
```

## 报告截图清单

- [ ] macOS 构建成功截图（B）
- [ ] Windows 构建成功截图（A）
- [ ] `validate_feature.ps1` 13/13 通过截图
- [ ] `run_tests.ps1` CTest 通过截图
- [ ] 合并后 `MixingStudio.exe` 运行截图
- [ ] 交叉测试记录截图
- [ ] Git 提交记录截图
