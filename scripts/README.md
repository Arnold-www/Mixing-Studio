# Scripts

`scripts/` 目录下的脚本按用途分类。Qt 路径由 `Resolve-QtPath.ps1` **自动探测**（`QT_PATH` → 常见 `D:\Qt\…` / `C:\Qt\…`），日常无需传 `-QtPath`。

## 一、启动应用（推荐）

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `run_app.ps1` | Windows | 自动探测 Qt → CMake 配置/构建 → 设 PATH → 启动 |

```powershell
# 零参数即可
.\scripts\run_app.ps1

# 已构建过，只启动
.\scripts\run_app.ps1 -SkipBuild

# 把 Qt 运行时部署到 exe 旁（之后可直接双击 MixingStudio.exe）
.\scripts\run_app.ps1 -Deploy
```

不要直接双击/裸跑 `.\build\bin\Debug\MixingStudio.exe`（除非已 `-Deploy` 或 PATH 已含 Qt `bin`），否则会因缺少 DLL 立刻退出。

---

## 二、环境配置

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `configure_qt_windows.ps1` | Windows | 自动找 Qt，执行 `cmake` 配置 + 全量构建 |
| `Resolve-QtPath.ps1` | Windows | 公共探测逻辑（被其它脚本 `.` 引用） |

```powershell
.\scripts\configure_qt_windows.ps1
# 仅在自动探测失败时才需要指定
.\scripts\configure_qt_windows.ps1 -QtPrefix "D:\Qt\6.8.3\msvc2022_64"
```

若本机尚未安装 Qt：

```powershell
python -m pip install --user aqtinstall
python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O "D:\Qt"
```

---

## 三、构建与测试

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `run_tests.ps1` | Windows | 一键 CMake 配置 → 构建 → 运行 CTest |
| `run_tests.sh` | macOS / Linux | 同上（B 同学 macOS 使用） |

```powershell
.\scripts\run_tests.ps1
.\scripts\run_tests.ps1 -WithApp
.\scripts\run_tests.ps1 -SkipConfigure -WithApp
```

```bash
chmod +x scripts/run_tests.sh
./scripts/run_tests.sh
WITH_APP=1 ./scripts/run_tests.sh
QT_PATH="$HOME/Qt/6.5.3/macos" ./scripts/run_tests.sh
```

**参数说明（`run_tests.ps1`）：**

| 参数 | 说明 |
| :--- | :--- |
| `-QtPath` | 可选；覆盖自动探测 |
| `-WithApp` | 同时构建 `MixingStudio` |
| `-SkipConfigure` | 跳过 `cmake -S . -B build` |
| `-BuildDir` | 构建目录，默认 `build` |
| `-Config` | `Debug` 或 `Release`，默认 `Debug` |

---

## 四、质量与交叉测试

| 脚本 | 平台 | 作用 |
| :--- | :--- | :--- |
| `validate_feature.ps1` | Windows | 13 项静态检查：头文件目录、QML 越层、Model/DSP 依赖、报告文件等 |

```powershell
.\scripts\validate_feature.ps1
.\scripts\validate_feature.ps1 -BaseBranch main
```

---

## 推荐工作流

### 成员 A（Windows）

```powershell
.\scripts\run_app.ps1          # 日常启动
.\scripts\run_tests.ps1 -WithApp
.\scripts\validate_feature.ps1
```

### 成员 B（macOS）

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
| `QT_PATH` | 可选；优先于目录扫描 |
| `CMAKE_PREFIX_PATH` | 可选；探测候选之一 |
| `BUILD_DIR` | 构建目录（`run_tests.sh`） |
| `WITH_APP` | 设为 `1` 时同时构建 App（`run_tests.sh`） |
