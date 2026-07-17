# Mixing Studio

基于 Qt 6、C++17、QML 与五层 MVVM（`App / View / ViewModel / Model / Common`）的多轨调音项目。

View 为纯 QML（无 ViewModel 注入）。App 加载 Main 后由 `MixingStudioViewBinder` 完成根 signal/property 与 ViewModel slots 的连接；ViewModel 不含任何 View 引用。

架构说明见 [`docs/PROJECT_STRUCTURE.md`](docs/PROJECT_STRUCTURE.md) 与 [`1. 架构关系与数据流向图 (Mermaid 格式).md`](1.%20架构关系与数据流向图%20(Mermaid%20格式).md)。

## 目录结构

- `include/Common/` — `MixerTypes`
- `include/Model/`、`src/Model/` — `AudioEngine` 等
- `include/DSP/`、`src/DSP/` — DSP 处理与分析
- `include/ViewModel/` — `RealMixerViewModel` / `TrackViewModel`（仅业务，无 View）
- `include/App/` — `MixingStudioApp`（启动与 ViewBinder 装配）
- `src/View/` — QML
- `tests/`、`scripts/`、`docs/`、`report/`

## 协作模式（后期）

- **成员 A（彭）：** 架构优化与实现（Common / RealVM / Model / App 装配）
- **成员 B（张）：** 测试与审核（CTest、`validate_feature`、合入验收）
- 调用链：`QML signal → MixingStudioViewBinder → ViewModel slots → Model → DSP`

## 脚本

详见 [`scripts/README.md`](scripts/README.md)。脚本会自动探测本机 Qt，**无需手写路径**。

启动后可用 **Import Audio** 选择本地 `.wav` / `.mp3`；**Load Sample** 加载内置样例；**Loop** / **Mock** 在 Transport 栏。

```powershell
# 一键：探测 Qt → 配置/构建 → 启动（推荐）
.\scripts\run_app.ps1

# 测试
.\scripts\run_tests.ps1 -WithApp
```

若需把 Qt DLL 拷到 exe 旁以便直接双击运行：`.\scripts\run_app.ps1 -Deploy`。
