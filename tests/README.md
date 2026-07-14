# Tests

## 当前测试

| 测试目标 | 源文件 | 覆盖范围 |
| :------- | :----- | :------- |
| `dsp_processor` | `test_dsp_processor.cpp` | `clampSample`、`applyGain`、`panLeftGain`、`panRightGain` |
| `audio_engine` | `test_audio_engine.cpp` | 导入、多轨、播放定时器、暂停、Seek、Loop、主音量 clamp、clearTracks |

## 一键运行

```powershell
.\scripts\run_tests.ps1 -WithApp
```

## 手动运行

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Debug
$env:PATH = "D:\Qt\6.5.3\msvc2019_64\bin;$env:PATH"
ctest --test-dir build -C Debug --output-on-failure
```

当前应通过：

- `dsp_processor`
- `audio_engine`
