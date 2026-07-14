# 大模型使用记录

最终报告必须保留大模型使用记录和采用模式。本文件用于持续记录。

| 日期 | 成员 | 阶段/模块 | 使用模型 | 采用模式 | 提示词摘要 | AI 输出 | 人工修改 | 验收结果 | 对应提交 |
| :--- | :--- | :-------- | :------- | :------- | :--------- | :------ | :------- | :------- | :------- |
| 2026-07-10 | B | 阶段 0 / ViewModel、View、Report 规划 | Codex | AI 辅助规划，人工确认执行 | 在 A 尚未提交底层代码前，规划 B 如何先做可回退的小步开发 | 拆分 B 侧阶段、明确 Mock 先行和接口隔离策略 | 人工确认在当前分支渐进开发并逐次 commit | 规划已记录，待后续构建验证 | `d7704c2` |
| 2026-07-10 | B | 阶段 1 / Mock 播放状态与 QML 骨架 | Codex | AI 主导代码生成，人工确认验证边界 | 先实现 B 侧播放进度状态和基础调音台界面，不等待 A 的真实音频实现 | 增加 Mock 播放进度属性、计时器和 QML 调音台布局 | 人工确认先记录 Qt6 环境缺口，是否安装取决于本机验收需求 | `git diff --check` 通过；CMake 因缺少 Qt6 未完成配置 | `f7245af` |
| 2026-07-10 | B | 阶段 2 / 轨道 Solo、Mute 和参数回显 | Codex | AI 主导代码生成，人工构建验证 | 实现 B 侧轨道控制状态机，补充 Solo/Mute 有效监听状态和 UI 回显 | 增加 `audible`、`volumeText`、`panText`、`anySolo` 和 QML 状态显示 | 人工确认状态机留在 ViewModel，QML 只做绑定 | `git diff --check` 通过；`cmake --build build-qt` 通过 | `7a25348` |
| 2026-07-10 | B | 阶段 3 / Mock 波形、频谱和电平显示 | Codex | AI 主导代码生成，人工构建验证 | 先完成 B 侧分析数据消费和可视化，不等待 A 的真实波形/VU/频谱实现 | 增加 Mock 分析数据、单轨电平属性、Canvas 波形和频谱渲染 | 人工确认 Mock 数据来源可后续替换，QML 接口保持稳定 | `git diff --check` 通过；`cmake --build build-qt` 通过 | `10f525b` |
| 2026-07-10 | B | 阶段 4 / 素材库和工程入口 | Codex | AI 主导代码生成，人工构建验证 | 先完成 B 侧素材搜索、导入入口、最近工程和恢复入口，不等待 A 的持久化实现 | 增加素材/工程 QStringList 属性、搜索过滤、导入/恢复/保存 stub 和 QML 左侧面板 | 人工确认当前为 Mock/stub，后续由 A 的 SQLite/JSON 接口替换 | `git diff --check` 通过；`cmake --build build-qt` 通过 | `60e47c7` |
| 2026-07-10 | B | 阶段 4.1 / UI 自我 Review 修正 | Codex | AI 自我 Review，人工确认方案后修改 | 用户指出界面排布、Spectrum 坐标和主次层级问题，要求先反思后按方案改动 | 重构为 Transport、Analysis、Mixer、Library 四区；补充波形/频谱轴线和横向 channel strip | 人工确认修改范围限定在 B 侧 QML，不新增 A 侧底层依赖 | `git diff --check` 通过；`cmake --build build-qt` 通过；GUI 启动无错误输出 | `f055073` |
| 2026-07-11 | A | `chai/feat` 交叉测试 / 验证脚本 | Codex | AI 生成测试脚本，人工运行并记录结果 | 根据 docs 中 MVVM 分层、B 侧分支范围和互测要求，为当前 feature 编写可重复运行的测试脚本 | 新增 `scripts/validate_feature.ps1`，检查分支、头文件目录、QML 越界、Model/DSP 依赖、报告证据和 CMake include 配置 | 人工运行脚本并补充互测记录；另运行 CMake 配置确认 Windows Qt6 环境缺口 | 静态验证 13 项通过；CMake 因本机未配置 Qt6 未通过，记录为环境问题 | `4fcbeaa` |
| 2026-07-11 | A | Windows Qt 环境 / 构建脚本 | Codex | AI 辅助环境配置，人工运行验证 | 为 Windows 本机安装 Qt 并保留可复用构建脚本，确保 `chai/feat` 可在 Windows 构建 | 使用 `aqtinstall` 安装 Qt 6.5.3；新增 `scripts/configure_qt_windows.ps1` 一键配置和构建 | 人工运行安装、配置和构建命令；确认生成 `build/bin/Debug/MixingStudio.exe` | `configure_qt_windows.ps1` 运行通过；Windows Debug 构建通过 | `dbe9213` |
| 2026-07-11 | A | 阶段 1 / Model/DSP 基建与测试骨架 | Codex | AI 主导代码生成，人工审查与合并验证 | 实现阶段一基建，并与 B feat 本地 merge 测试 | `DspProcessor`、`AudioEngine` getter/clamp、DSP 测试、CMake CTest、`run_tests.ps1` | 从 main 基线开发；merge `chai/feat`；运行架构检查 | CTest 1/1；`validate_feature` 13/13 | `2654de0` |
| 2026-07-11 | A | 阶段 1.1 / A+B 本地合并交叉测试 | Codex | AI 主导合并与验证，人工确认接口边界 | 将 `origin/chai/feat` 合并到 A 侧阶段一分支，验证 MVVM 边界和 VM→Model 调用链 | 本地 merge 成功；更新 `CROSS_TEST_LOG.md`、`TEST_AND_TOOLCHAIN.md` | 人工确认 QML 不越层、播放/导入命令正确转发 | 合并后全量构建通过；A 侧本地互测通过 | `9cac45c` |
| 2026-07-11 | A | 阶段 2 / 播放闭环底层接口 | Codex | AI 主导代码生成，人工构建与测试验证 | 构建阶段 2：导入、多轨、播放、Seek、Loop、主音量 | `AudioEngine` 播放定时器、Seek/Loop、`test_audio_engine`；VM 进度对接 Model 时钟 | 默认不自动开启 Loop；保留 B 侧 Mock 分析刷新 | CTest 2/2 通过；App 构建通过 | 待提交 |

## 采用模式示例

- AI 主导代码生成，人工审查和修改。
- AI 辅助调试，人工复现和确认。
- AI 生成测试，人工补充边界用例。
- AI 审查 MVVM 架构边界，人工决定是否修复。
- AI 辅助报告撰写，人工补充真实截图和提交证据。

## 一键测试

```powershell
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

```bash
WITH_APP=1 ./scripts/run_tests.sh
```
