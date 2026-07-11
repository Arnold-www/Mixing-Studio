# Scripts

## 阶段一脚本

| 脚本 | 用途 | 使用者 |
| :--- | :--- | :----- |
| `run_tests.ps1` | 一键配置、构建、运行 CTest | A / B |
| `validate_feature.ps1` | MVVM 架构边界检查 | B（A 亦可复用） |
| `configure_qt_windows.ps1` | Windows Qt 环境配置 | B |

### 阶段一推荐命令（Windows）

```powershell
.\scripts\configure_qt_windows.ps1   # 首次配置 Qt 时
.\scripts\run_tests.ps1 -WithApp     # 构建 App + 运行 DSP 测试
.\scripts\validate_feature.ps1       # 架构边界检查
```

### 参数

`run_tests.ps1`：

- `-QtPath`：Qt 安装路径
- `-WithApp`：同时构建 `MixingStudio` 可执行文件
- `-SkipConfigure`：跳过 CMake 配置
