# 交叉测试记录

每个阶段合入主分支前，必须由非主责成员测试通过并记录。

| 日期 | 阶段 | 实现成员 | 测试成员 | 测试范围 | 发现问题 | 修复结果 | 是否允许合入主分支 | 对应提交 |
| :--- | :--- | :------- | :------- | :------- | :------- | :------- | :----------------- | :------- |
|      |      |          |          |          |          |          |                    |          |
| 2026-07-11 | B 侧 feature / `chai/feat` | 成员 B | 成员 A | 按规划检查 ViewModel/QML 分层、头文件目录、B 侧分支范围、报告证据文件和 CMake 头文件配置；运行 `scripts/validate_feature.ps1` 和 Windows Qt 构建脚本 | 首次 CMake 配置因 Windows 未配置 Qt6 失败；安装 Qt 6.5.3 后复测通过 | 保留 `scripts/validate_feature.ps1` 和 `scripts/configure_qt_windows.ps1`；静态验证 13 项通过；`cmake --build build --config Debug` 通过并生成 `MixingStudio.exe` | 是，静态验证和 Windows Qt 构建均通过 | `4fcbeaa` |

## 检查重点

- QML 是否直接访问 Model 或 DSP。
- ViewModel 是否正确转发命令和状态。
- Model/DSP 是否按接口返回可消费数据。
- 音频参数是否生效。
- 工程保存、加载和导出是否正确。
- 测试结果是否能进入报告。
