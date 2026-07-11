# 测试与工具链记录

## 构建环境

- 操作系统：Windows 10.0.26200
- Qt 版本：6.5.3 (msvc2019_64)
- CMake 版本：3.31.3
- 分支：`feature/A-model-dsp-sprint1-infra`（已 merge `origin/chai/feat`）

## 阶段一一键测试

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp
.\scripts\validate_feature.ps1 -BaseBranch main
```

## 测试记录

| 日期 | 测试类型 | 测试对象 | 命令/步骤 | 结果 | 问题 | 修复提交 |
| :--- | :------- | :------- | :-------- | :--- | :--- | :------- |
| 2026-07-11 | 单元测试 | Sprint 1 DSP 骨架 | `run_tests.ps1 -WithApp` | 通过，1/1 | 无 | `2654de0` |
| 2026-07-11 | 架构检查 | Sprint 1 MVVM 边界 | `validate_feature.ps1` | 通过，13/13 | 无 | merge commit |
| 2026-07-11 | 集成构建 | A+B 阶段一合并 | merge `chai/feat` 后全量构建 | 通过 | 无 | merge commit |

## 最近一次 CTest 输出

```text
1/1 Test #1: dsp_processor ....................   Passed    0.01 sec
100% tests passed, 0 tests failed out of 1
```

## 报告截图清单

- [ ] `run_tests.ps1` 通过截图
- [ ] `validate_feature.ps1` 13/13 截图
- [ ] 合并后 `MixingStudio.exe` 运行截图
- [ ] Git 合并提交记录截图
