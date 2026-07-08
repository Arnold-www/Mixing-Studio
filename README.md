# Mixing Studio

基于 Qt 6、C++17、QML 和严格 MVVM 架构的多轨调音项目。

本仓库当前处于基础框架阶段，已经预留：

- `src/Model/`：音频导入、播放、工程保存、WAV 导出等 Model 层。
- `src/DSP/`：混音、Pan、EQ、Compressor、Limiter、波形/VU/频谱等 DSP 算法。
- `src/ViewModel/`：`Q_PROPERTY`、轨道状态机、命令转发和数据绑定。
- `src/View/`：QML 页面和控件，只绑定 ViewModel，不直接访问 Model/DSP。
- `include/`：所有 `.h` 头文件，按 `Model`、`DSP`、`ViewModel` 分类。
- `docs/planning/`：项目规划书和协同规范。
- `report/`：总报告、分报告、大模型使用记录、互测记录和提交证据。

## 协作模式

两名成员分别操控 AI 进行开发：

- 成员 A 主责 `Model / DSP / Persistence`。
- 成员 B 主责 `ViewModel / View / Report`。
- 每个阶段必须交叉测试对方实现，通过后才能合入主分支。

## 构建方式

需要 Qt 6.5+ 和 CMake 3.21+。

```powershell
cmake -S . -B build
cmake --build build
```

## 截止时间

最终提交截止时间为 7 月 18 日。7 月 17 日进入冻结期，只修复稳定性、测试、报告材料和大模型使用记录。
