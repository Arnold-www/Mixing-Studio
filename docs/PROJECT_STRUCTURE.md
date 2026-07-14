# 项目结构说明

本项目采用 **MVVM + App + Command** 分层，并按职责拆分源文件（单一职责）。

## 源码目录

```text
include/ / src/
  DSP/
    DspProcessor.*     # 实时处理链（增益/Pan/EQ/压缩/混音/限幅）
    DspAnalysis.*      # 分析（波形/VU/频谱/峰值/削波）
  Model/
    AudioTrack.h
    AudioEngine.*      # 引擎门面 + 按关注点拆分的实现文件：
      AudioEngine_Playback.cpp
      AudioEngine_Tracks.cpp
      AudioEngine_Analysis.cpp
      AudioEngine_Persistence.cpp
    AssetLibrary.* / ProjectStore.*
  App/
    MixerApp.*         # 用例门面 + Playback / Project / Tracks 拆分实现
  Command/
    ICommand.h
    PlaybackCommands.* / ProjectCommands.* / TrackDspCommands.*
    MixerCommands.h    # 伞头文件，聚合上述命令
  ViewModel/
    MixerViewModel.* + MixerViewModel_Sync.cpp
    TrackViewModel.*
  View/
    Main.qml           # 布局组装
    TransportBar.qml / WaveformPanel.qml / SpectrumPanel.qml
    LibraryPanel.qml / TrackMixerList.qml
```

## 分层约束

1. QML 只能绑定 ViewModel，不得直接访问 Command / App / Model / DSP。
2. Command 调用 App；App 调用 Model；Model/DSP 不依赖上层。
3. 对外 API 仍以门面类为准（`AudioEngine`、`MixerApp`、`MixerViewModel`），拆分的是实现文件。

## 数据流

```text
QML 组件 → ViewModel 槽 → Command::execute → MixerApp → AudioEngine → DSP
                                              ↑ Signal 回传
```
