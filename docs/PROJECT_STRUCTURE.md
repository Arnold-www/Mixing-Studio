# 项目结构说明

本项目采用 **五层 MVVM**（`App / View / ViewModel / Model / Common`），View 为纯 QML，由 **MixingStudioViewBinder** 装配与 ViewModel 的连接（无 ContextProperty / setInitialProperties 注入）。

架构真源见：[架构关系与数据流向图](../1.%20架构关系与数据流向图%20(Mermaid%20格式).md)

## 源码目录

```text
include/ / src/
  Common/
    MixerTypes.h
  DSP/
    DspProcessor.* / DspAnalysis.*
  Model/
    AudioEngine.* / …
  ViewModel/
    RealMixerViewModel.*   # 仅业务 state / slots / NOTIFY（无 View）
    TrackViewModel.*
  App/
    MixingStudioApp.*      # 启动 + ViewBinder（View 根 ↔ ViewModel）
  View/
    Main.qml               # 根 property + signal（纯 View）
    子面板
```

## 分层约束（红线）

1. **纯 View**：全 QML 不得出现 ViewModel 类型名；禁止 ContextProperty / setInitialProperties。
2. **ViewModel**：仅 `QObject` + slots + NOTIFY；禁止持有 View 根、禁止 `bindView` / `QQuickItem` / `findChild`。
3. **App**：对象创建与 View↔ViewModel 装配（`MixingStudioViewBinder`）。
4. Model/DSP 不依赖 View / ViewModel / App。

## 数据流

```text
Main 根 property/signal（纯 View）
        ↕  MixingStudioViewBinder（connect / setProperty）
RealMixerViewModel → AudioEngine → DSP
```

## 后期分工

- **成员 A**：架构优化与实现（Common / RealVM / Model / App 装配）。
- **成员 B**：测试与审核（CTest、`ui_binder` UI 自动化、validate_feature、合入验收），不主责改架构。
