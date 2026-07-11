# 交叉测试记录

| 日期 | 阶段 | 实现成员 | 测试成员 | 测试范围 | 发现问题 | 修复结果 | 是否允许合入主分支 | 对应提交 |
| :--- | :--- | :------- | :------- | :------- | :------- | :------- | :----------------- | :------- |
| 2026-07-11 | 阶段一：基建与发声 | A+B | A（本地） | merge `chai/feat` 到 `feature/A-model-dsp-sprint1-infra`；`run_tests.ps1 -WithApp`；`validate_feature.ps1`；审查 VM→Model 调用 | 无阻塞问题 | 不适用 | 待 B 在 macOS 复测后可合入 | `2654de0` + merge |

## 阶段一检查项（A 测 B）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| QML 不直接访问 Model/DSP | ✅ | `validate_feature.ps1` 通过 |
| Model/DSP 不依赖 View/ViewModel | ✅ | `validate_feature.ps1` 通过 |
| `MixerViewModel` 转发 play/pause/stop | ✅ | 调用 `m_audioEngine->play()` 等 |
| `importTrack` 由 VM 触发 | ✅ | `importMockTrack` / `importAssetByName` |
| 播放状态信号连接 | ✅ | `playbackStateChanged` → `playingChanged` |
| 合并后构建 | ✅ | `MixingStudio.exe` + `test_dsp_processor` |

## 阶段一检查项（待 B 测 A）

| 检查项 | 结果 | 说明 |
| :----- | :--- | :--- |
| `AudioEngine` 接口可被 VM 调用 | 待填 | B 在 macOS 执行 `./scripts/run_tests.sh` |
| `DspProcessor` 单元测试 | 待填 | CTest `dsp_processor` |
| 主音量 clamp [0,1] | 待填 | `setMasterVolume` 已加 clamp |

## 一键命令

```powershell
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

```bash
QT_PATH="$HOME/Qt/6.5.3/macos" ./scripts/run_tests.sh
```
