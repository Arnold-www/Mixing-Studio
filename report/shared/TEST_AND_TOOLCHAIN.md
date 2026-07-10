# 测试与工具链记录

本文件用于记录报告中需要体现的单元测试、其他测试、构建工具和持续集成情况。

## 构建环境

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

## 测试记录

| 日期 | 测试类型 | 测试对象 | 命令/步骤 | 结果 | 问题 | 修复提交 |
| :--- | :------- | :------- | :-------- | :--- | :--- | :------- |
| 2026-07-10 | 构建测试 | Qt/CMake 工具链 | `cmake -S . -B build-qt -DCMAKE_PREFIX_PATH=/opt/homebrew` | 通过 | 初始环境缺少 Qt6；安装 `qtdeclarative` 后通过 | `7739f16` |
| 2026-07-10 | 构建测试 | B 侧阶段 2 ViewModel/QML | `cmake --build build-qt` | 通过 | 无 | `7a25348` |
| 2026-07-10 | 构建测试 | B 侧阶段 3 可视化 | `cmake --build build-qt` | 通过 | 无 | `10f525b` |
| 2026-07-10 | 构建测试 | B 侧阶段 4 素材/工程入口 | `cmake --build build-qt` | 通过 | 无 | `60e47c7` |

## 报告截图清单

- 构建成功截图：
- 单元测试截图：
- 主界面运行截图：
- 交叉测试记录截图：
- Git 提交记录截图：
