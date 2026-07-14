# Tests

## 当前测试

| 测试目标 | 源文件 | 覆盖范围 |
| :------- | :----- | :------- |
| `dsp_processor` | `test_dsp_processor.cpp` | clamp/gain/pan、`dbToLinear`、三段 EQ、压缩器、轨处理、线性混音、主总线限幅 |
| `audio_engine` | `test_audio_engine.cpp` | 导入/播放/Seek/Loop/主音量；轨音量/Pan/Mute/Solo/EQ/压缩/Bypass；`renderMixFrame` 离线混音 |

## 阶段三测试计划（自拟）

1. **DSP 单元**：平坦 EQ 透传；正增益抬升；压缩器按 ratio 压峰值；不可听轨输出静音；混音线性相加后主限幅夹到 ±1。
2. **Engine 混音**：左右硬分轨；Mute 静音单轨；Solo 屏蔽非 Solo；EQ/压缩在 Bypass 关闭时生效；双轨满幅和经限幅。
3. **回归**：阶段二播放时钟用例保持通过。
4. **架构**：`validate_feature.ps1` 确认 QML 不碰 Model/DSP。
5. **UI（非本阶段 A 必做）**：EQ/Comp/Bypass 控件留给成员 B；现有 Volume/Pan/Mute/Solo 已可同步到 Model。真声卡发声仍非本阶段目标。

## 一键运行

```powershell
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp
.\scripts\validate_feature.ps1
```

## 手动运行

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="D:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

当前应通过：

- `dsp_processor`
- `audio_engine`
