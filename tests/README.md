# Tests

阶段一（Sprint 1）仅包含 DSP 单元测试骨架。

## 当前测试

- `test_dsp_processor.cpp`：验证 `clampSample`、`applyGain`、`panLeftGain`、`panRightGain`

## 一键运行

```powershell
.\scripts\run_tests.ps1 -WithApp
```

## 手动运行

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

当前应通过：`dsp_processor`
