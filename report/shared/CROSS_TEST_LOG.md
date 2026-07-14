# 交叉测试记录

每个阶段合入主分支前，必须由非主责成员测试通过并记录。

| 日期 | 阶段 | 实现成员 | 测试成员 | 测试范围 | 发现问题 | 修复结果 | 是否允许合入主分支 | 对应提交 |
| :--- | :--- | :------- | :------- | :------- | :------- | :------- | :----------------- | :------- |
| 2026-07-11 | B 侧 feature / `chai/feat` | 成员 B | 成员 A | 按规划检查 ViewModel/QML 分层、头文件目录、B 侧分支范围、报告证据文件和 CMake 头文件配置；运行 `scripts/validate_feature.ps1` 和 Windows Qt 构建脚本 | 首次 CMake 配置因 Windows 未配置 Qt6 失败；安装 Qt 6.5.3 后复测通过 | 保留 `scripts/validate_feature.ps1` 和 `scripts/configure_qt_windows.ps1`；静态验证 13 项通过；`cmake --build build --config Debug` 通过并生成 `MixingStudio.exe` | 是，静态验证和 Windows Qt 构建均通过 | `4fcbeaa` |
| 2026-07-11 | 阶段 1：基建与发声（A+B 本地 merge） | 成员 A + 成员 B | 成员 A（本地） | merge `chai/feat` 到 `feature/A-model-dsp-sprint1-infra`；`run_tests.ps1 -WithApp`；`validate_feature.ps1`；审查 VM→Model 调用链 | 无阻塞问题 | 不适用 | 待 B 在 macOS 复测后可合入 | `2654de0`、`9cac45c` |
| 2026-07-11 | 阶段 2：播放闭环 | 成员 A | 成员 A（自测） | `AudioEngine` 导入/播放/Seek/Loop；`test_audio_engine`；VM 进度对接 Model | 无 | 不适用 | 待 B 用 UI 交叉测试后可合入 | 待提交 |

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

## 阶段 1 检查项（待 B 测 A）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| `AudioEngine` 接口可被 VM 调用 | 待填 | B 在 macOS 执行 `./scripts/run_tests.sh` |
| `DspProcessor` 单元测试 | 待填 | CTest `dsp_processor` |
| 主音量 clamp [0,1] | 待填 | `setMasterVolume` 已加 clamp |
| 合并后 macOS 全量构建 | 待填 | `cmake --build build-qt` |

## 阶段 2 检查项（待 B 测 A）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| Import 后 `durationMs` 为 180s 占位 | 待填 | UI 时间码应显示约 3:00 |
| Play 后进度条随 Model 时钟前进 | 待填 | 不再由 VM 独立 Mock 秒表驱动 |
| Seek 拖动进度条调用 `AudioEngine::seek` | 待填 | 位置应立即跳转 |
| Pause/Stop 状态与位置正确 | 待填 | Stop 回零 |
| CTest `audio_engine` 通过 | 待填 | `./scripts/run_tests.sh` 或 Windows 对等脚本 |

## 一键命令

```powershell
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

```bash
WITH_APP=1 ./scripts/run_tests.sh
```
