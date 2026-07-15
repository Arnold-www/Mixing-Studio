# Mixing Studio

基于 Qt 6、C++17、QML 与五层 MVVM（`App / View / ViewModel / Model / Common`）的多轨调音项目。

View 通过 **接口面向** 解耦：QML 只绑定 `IMixerViewModel*`，由 App 注入；实现类为 `RealMixerViewModel`（聚合 `AudioEngine`）。

架构说明见 [`docs/PROJECT_STRUCTURE.md`](docs/PROJECT_STRUCTURE.md) 与 [`1. 架构关系与数据流向图 (Mermaid 格式).md`](1.%20架构关系与数据流向图%20(Mermaid%20格式).md)。

## 目录结构

- `include/Common/` — `ICommandBase`、`MixerTypes`
- `include/Model/`、`src/Model/` — `AudioEngine` 等
- `include/DSP/`、`src/DSP/` — DSP 处理与分析
- `include/ViewModel/` — `IMixerViewModel` / `ITrackViewModel` 契约 + `RealMixerViewModel` / `TrackViewModel`
- `include/Command/` — 作用于 `AudioEngine` 的命令
- `include/App/` — `MixingStudioApp` 装配（注入接口指针）
- `src/View/` — QML
- `tests/`、`scripts/`、`docs/`、`report/`

## 协作模式（后期）

- **成员 A（彭）：** 架构优化与实现（Common / 契约 / RealVM / Model / App 装配）
- **成员 B（张）：** 测试与审核（CTest、`validate_feature`、合入验收）
- 调用链：`QML → IMixerViewModel → RealMixerViewModel → Command/Model → DSP`

## 脚本

详见 [`scripts/README.md`](scripts/README.md)。脚本会自动探测本机 Qt，**无需手写路径**。

```powershell
# 一键：探测 Qt → 配置/构建 → 启动（推荐）
.\scripts\run_app.ps1

# 测试
.\scripts\run_tests.ps1 -WithApp
```

若需把 Qt DLL 拷到 exe 旁以便直接双击运行：`.\scripts\run_app.ps1 -Deploy`。
## 截止时间

最终提交：7 月 18 日；7 月 17 日进入冻结期。
