# 项目结构说明

本项目采用严格 MVVM 分层，并按照两名成员分别操控 AI 的协作方式组织仓库。

## 源码目录

```text
include/
  DSP/           # 成员 A 主责：DSP 对外头文件
  Model/         # 成员 A 主责：Model 对外头文件
  ViewModel/     # 成员 B 主责：ViewModel 对外头文件

src/
  DSP/           # 成员 A 主责：DSP 实现文件
  Model/         # 成员 A 主责：音频引擎、工程保存、素材库、导出实现
  ViewModel/     # 成员 B 主责：Q_PROPERTY、状态机、命令路由实现
  View/          # 成员 B 主责：QML 页面、控件、可视化
```

## 文档目录

```text
docs/
  planning/      # 已确认的规划书、协同规范和附录
```

## 报告目录

```text
report/
  member-A/      # 成员 A 的模块实现过程、提交记录、测试记录
  member-B/      # 成员 B 的模块实现过程、提交记录、测试记录
  shared/        # 总报告材料、大模型使用记录、互测记录、工具链记录
```

## 分层约束

1. `src/View/` 中的 QML 只能绑定 ViewModel。
2. `.h` 头文件统一放在 `include/` 下，并按 `DSP/`、`Model/`、`ViewModel/` 分类。
3. `src/ViewModel/` 负责状态转换和命令转发，不写 DSP 算法。
4. `src/Model/` 和 `src/DSP/` 不依赖 QML。
5. 每个阶段合入主分支前，必须由对方成员交叉测试。
