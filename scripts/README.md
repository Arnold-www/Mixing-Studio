# Scripts

`scripts/` 目录下的脚本按用途分为三类。

## 一、环境配置

用于首次在本机安装/定位 Qt，并完成 CMake 配置与构建。

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `configure_qt_windows.ps1` | Windows | 指定 Qt 路径，执行 `cmake` 配置 + 全量构建 |

**何时使用：** 第一次在 Windows 上搭建环境，或更换 Qt 安装路径后。

```powershell
.\scripts\configure_qt_windows.ps1
# 自定义 Qt 路径
.\scripts\configure_qt_windows.ps1 -QtPrefix "D:\Qt\6.5.3\msvc2019_64"
```

若本机尚未安装 Qt，可先执行：

```powershell
python -m pip install --user aqtinstall
python -m aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -O "D:\Qt"
```

---

## 二、构建与测试

用于日常开发：配置、编译、运行单元测试。

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `run_tests.ps1` | Windows | 一键 CMake 配置 → 构建 → 运行 CTest（`dsp_processor`） |
| `run_tests.sh` | macOS / Linux | 同上（B 同学 macOS 使用） |

**何时使用：** 每次改完 A 侧 DSP/Model 代码后，或合并分支后做回归验证。

```powershell
# 仅构建并测试 DSP（最快）
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64"

# 同时构建完整 App + 运行测试（推荐）
.\scripts\run_tests.ps1 -QtPath "D:\Qt\6.5.3\msvc2019_64" -WithApp

# 已配置过 CMake，跳过配置步骤
.\scripts\run_tests.ps1 -SkipConfigure -WithApp
```

```bash
chmod +x scripts/run_tests.sh
./scripts/run_tests.sh                          # 仅 DSP 测试
WITH_APP=1 ./scripts/run_tests.sh               # App + 测试
QT_PATH="$HOME/Qt/6.5.3/macos" ./scripts/run_tests.sh
```

**参数说明（`run_tests.ps1`）：**

| 参数 | 说明 |
| :--- | :--- |
| `-QtPath` | Qt 安装根目录 |
| `-WithApp` | 同时构建 `MixingStudio` 可执行文件 |
| `-SkipConfigure` | 跳过 `cmake -S . -B build` |
| `-BuildDir` | 构建目录，默认 `build` |
| `-Config` | `Debug` 或 `Release`，默认 `Debug` |

---

## 三、质量与交叉测试

用于阶段合入前检查 MVVM 分层、分支范围和报告证据是否齐全。

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `validate_feature.ps1` | Windows | 13 项静态检查：头文件目录、QML 越层、Model/DSP 依赖、报告文件等 |

**何时使用：** 阶段一交叉测试、合入 `main` 之前。

```powershell
.\scripts\validate_feature.ps1
.\scripts\validate_feature.ps1 -BaseBranch main
```

---

## 推荐工作流

### 成员 A（Windows，日常开发）

```powershell
# 1. 首次环境
.\scripts\configure_qt_windows.ps1

# 2. 改代码后
.\scripts\run_tests.ps1 -WithApp

# 3. 合入前
.\scripts\validate_feature.ps1

# 4. 启动程序
$env:PATH = "D:\Qt\6.5.3\msvc2019_64\bin;$env:PATH"
.\build\bin\Debug\MixingStudio.exe
```

### 成员 B（macOS，交叉测试）

```bash
cmake -S . -B build-qt -DCMAKE_PREFIX_PATH=/opt/homebrew
cmake --build build-qt
./build-qt/bin/MixingStudio

WITH_APP=1 ./scripts/run_tests.sh
```

---

## 环境变量

| 变量 | 作用 |
| :--- | :--- |
| `QT_PATH` | Qt 安装根目录 |
| `CMAKE_PREFIX_PATH` | CMake 查找 Qt 的前缀 |
| `BUILD_DIR` | 构建目录（`run_tests.sh`） |
| `WITH_APP` | 设为 `1` 时同时构建 App（`run_tests.sh`） |
