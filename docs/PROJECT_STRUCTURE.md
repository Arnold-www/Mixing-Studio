# 项目结构说明

本项目采用 **五层 MVVM**（`App / View / ViewModel / Model / Common`），View 为 QML，通过 **接口面向** 与 C++ 解耦。

架构真源见：[架构关系与数据流向图](../1.%20架构关系与数据流向图%20(Mermaid%20格式).md)

## 源码目录

```text
include/ / src/
  Common/
    ICommandBase.h       # 命令契约（ICommand 别名）
    MixerTypes.h         # TrackDspParams / SoloPlan / 素材与工程辅助
  DSP/
    DspProcessor.* / DspAnalysis.*
  Model/
    AudioEngine.* / AudioTrack.h / AssetLibrary.* / ProjectStore.* / WavExporter.*
  ViewModel/
    IMixerViewModel.h / ITrackViewModel.h   # QML 契约（Q_PROPERTY + slots/signals）
    RealMixerViewModel.*                    # 实现；聚合 AudioEngine
    TrackViewModel.*                        # 实现 ITrackViewModel
  Command/
    Playback / Project / TrackDsp 命令（操作 AudioEngine）
  App/
    MixingStudioApp.*    # 仅装配：创建 Model/RealVM，以 IMixerViewModel* 注入 QML
  View/
    Main.qml 及子组件    # 只绑定 mixerViewModel 契约属性/槽
```

最终交付前核对见 [ARCHITECTURE_CHECKLIST.md](ARCHITECTURE_CHECKLIST.md)。

真实音频链路：`AudioFileDecoder`（WAV/MP3）→ 轨 `pcmMono` → `renderMixAtMs` / `QAudioSink`；QML 用 FileDialog / Load Sample / Loop / Mock。

## 分层约束

1. QML 只通过上下文中的 `mixerViewModel`（`IMixerViewModel*`）交互，不得引用 `RealMixerViewModel` / Model / DSP / Command / App。
2. App 创建 `RealMixerViewModel`，但 **`setContextProperty` 必须使用接口指针**。
3. RealVM **直接聚合** `AudioEngine`（Model），不再经旧 `MixerApp` 用例门面。
4. 命令实现 `ICommandBase::execute()`，可由 ViewModel 槽触发（允许 Qt 槽）。
5. Model/DSP 不依赖 View / ViewModel / App / Command。

## 数据流

```text
QML → IMixerViewModel（契约）→ RealMixerViewModel → Command/直接调用 → AudioEngine → DSP
         ↑ App 注入接口指针
```

## 后期分工

- **成员 A**：架构优化与实现（Common / 接口契约 / RealVM / Model / App 装配）。
- **成员 B**：测试与审核（CTest、validate_feature、合入验收），不主责改架构。
