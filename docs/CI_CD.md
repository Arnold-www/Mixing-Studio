# GitHub CI/CD

## CI

`.github/workflows/ci.yml` 在以下场景运行：

- 向 `main`、`ci/**` 或 `feature/**` 推送提交；
- 创建或更新 Pull Request；
- 在 GitHub Actions 页面手动触发。

每次运行都会在 Ubuntu 24.04、Windows Server 2022 和 macOS 14 上安装 Qt 6.8.3，完成 Release 构建，并运行全部 CTest。CI 默认只有 `contents: read` 权限。

建议把三个 `CI / Linux`、`CI / Windows`、`CI / macOS` check 配置为 `main` 的 required status checks。

## CD

`.github/workflows/release.yml` 由 `v*` 标签触发。三个系统分别执行构建、测试和 `cmake --install`，然后生成 `.tar.gz` 安装包。所有平台成功后，工作流会用 GitHub 自动提供的 `GITHUB_TOKEN` 创建 Release 并上传安装包。

发布示例：

```bash
git switch main
git pull --ff-only
git tag -a v0.1.0 -m "Mixing Studio v0.1.0"
git push origin v0.1.0
```

仅 `publish` job 拥有 `contents: write`，构建 job 保持只读权限。发布失败时不要重复创建同名标签；先在 Actions 日志中修复问题，再重新运行失败的 jobs。

## 本地等价验证

```bash
cmake -S . -B build-ci -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci --config Release --parallel 2
ctest --test-dir build-ci -C Release --output-on-failure
cmake --install build-ci --config Release --prefix stage
```
