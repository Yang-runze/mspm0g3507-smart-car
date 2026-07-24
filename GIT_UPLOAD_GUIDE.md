# Git 仓库更新与上传指南

本指南适用于当前工程仓库：

```text
E:\TI2026\mspm0_sdk_2_10_00_04\titmx
```

以下命令均在 PowerShell 中执行。尖括号里的示例内容需要换成你自己的信息，命令本身不要带尖括号。

## 一、第一次使用时配置身份

```powershell
git config --global user.name "你的 GitHub 用户名"
```

作用：设置以后创建 Git 提交时记录的作者姓名。`--global` 表示对这台电脑上的所有 Git 仓库生效，通常只需执行一次。

```powershell
git config --global user.email "你的 GitHub 邮箱"
```

作用：设置提交作者邮箱。建议填写 GitHub 账户中已验证的邮箱，或 GitHub 提供的隐私邮箱。

```powershell
git config --global --list
```

作用：查看当前全局 Git 配置，用来确认姓名和邮箱是否设置正确。

## 二、每次更新代码的推荐流程

### 1. 进入仓库根目录

```powershell
Set-Location E:\TI2026\mspm0_sdk_2_10_00_04\titmx
```

作用：把 PowerShell 当前目录切换到 Git 仓库根目录。后续命令都应在此目录执行。

### 2. 查看当前状态

```powershell
git status
```

作用：显示当前分支、修改过的文件、未跟踪文件以及已经加入暂存区的文件。上传前后都建议执行一次。

```powershell
git branch --show-current
```

作用：只显示当前分支名称。本工程正常应为 `main`。

```powershell
git remote -v
```

作用：显示远程仓库名称和地址，确认 `origin` 指向正确的 GitHub 仓库。它不会上传或修改文件。

### 3. 开始修改代码前同步远程更新

```powershell
git pull --rebase origin main
```

作用：从远程仓库 `origin` 下载 `main` 分支的新提交，并把本地尚未上传的提交依次放到其后。这样通常能保持提交历史整洁。

最好在工作区没有未提交修改时执行。如果已经修改了代码，先按后面的步骤提交，再执行本命令。

### 4. 检查自己修改了什么

```powershell
git diff --stat
```

作用：按文件汇总改动行数，快速检查本次修改范围是否符合预期。

```powershell
git diff
```

作用：详细显示尚未加入暂存区的代码差异。提交前应检查是否混入无关修改、调试代码或敏感信息。

### 5. 把本次修改加入暂存区

```powershell
git add -A
```

作用：把仓库中的新增、修改和删除文件全部加入暂存区，准备生成一次提交。该命令还没有上传到 GitHub。

如果只想加入指定文件，可以使用：

```powershell
git add tests/unit_tests/module_test.c
```

作用：只把列出的文件加入暂存区，其他修改保持不动。

### 6. 再次检查待提交内容

```powershell
git status
```

作用：确认 `Changes to be committed` 下只有这次确实要提交的文件。

```powershell
git diff --cached --check
```

作用：检查暂存内容中常见的空白错误。没有输出通常表示检查通过。

```powershell
git diff --cached
```

作用：详细查看已经加入暂存区、即将被提交的最终差异。

如果误把某个文件加入了暂存区：

```powershell
git restore --staged 路径\文件名
```

作用：只把指定文件移出暂存区，不删除文件，也不丢弃该文件的本地修改。

### 7. 创建本地提交

```powershell
git commit -m "说明本次修改内容"
```

作用：把暂存区内容保存为一个本地 Git 提交。提交说明应简短、明确，例如：

```powershell
git commit -m "Add OLED test selection menu"
```

`commit` 只保存在本机，此时还没有上传到 GitHub。

### 8. 上传前再次同步远程分支

```powershell
git pull --rebase origin main
```

作用：检查别人是否在你修改期间上传了新提交。如有新提交，Git 会先接收远程内容，再把你的本地提交接到后面。

### 9. 上传到 GitHub

```powershell
git push origin main
```

作用：把本地 `main` 分支中尚未上传的提交发送到远程仓库 `origin` 的 `main` 分支。

第一次把本地分支关联到远程分支时，可以使用：

```powershell
git push -u origin main
```

作用：上传的同时建立本地 `main` 与 `origin/main` 的跟踪关系。以后可直接执行 `git push`。

GitHub 使用 HTTPS 地址时，登录窗口要求的“密码”通常应使用浏览器授权或 Personal Access Token，而不是 GitHub 登录密码。

### 10. 确认上传结果

```powershell
git status
```

作用：如果显示 `Your branch is up to date with 'origin/main'` 和 `working tree clean`，表示本地、远程一致且没有遗漏修改。

```powershell
git log --oneline --decorate -5
```

作用：显示最近 5 次提交。可检查刚才的提交是否位于 `HEAD`、`main` 和 `origin/main`。

## 三、以后最常用的完整命令

开始修改前：

```powershell
Set-Location E:\TI2026\mspm0_sdk_2_10_00_04\titmx
git status
git pull --rebase origin main
```

完成代码修改并在 Keil 中编译测试后：

```powershell
git diff --stat
git diff
git add -A
git status
git diff --cached --check
git diff --cached
git commit -m "说明本次修改内容"
git pull --rebase origin main
git push origin main
git status
```

不要在没有查看 `git status` 和 `git diff --cached` 的情况下直接提交。

## 四、常见情况处理

### 1. 显示 `nothing to commit`

表示暂存区和已跟踪文件中没有可提交的变化。先执行 `git status`，确认文件是否确实保存、是否被 `.gitignore` 忽略，或是否已经提交过。

### 2. `git push` 被拒绝

通常表示远程 `main` 出现了本地没有的新提交。执行：

```powershell
git pull --rebase origin main
git push origin main
```

第一条命令先同步并重新排列本地提交，成功后第二条命令再上传。

### 3. `pull --rebase` 出现冲突

先查看冲突文件：

```powershell
git status
```

作用：列出发生冲突的文件。

手工编辑文件并删除其中的冲突标记 `<<<<<<<`、`=======`、`>>>>>>>`，保留最终需要的代码。每解决一个文件后执行：

```powershell
git add 路径\冲突文件
```

作用：告诉 Git 该文件的冲突已经解决。

全部解决后执行：

```powershell
git rebase --continue
```

作用：继续完成被冲突暂停的 rebase。

如果发现无法正确处理，想恢复到执行 rebase 之前：

```powershell
git rebase --abort
```

作用：取消本次 rebase，回到开始 rebase 前的状态。

### 4. 查看某条命令是否上传成功

```powershell
git fetch origin
git status
```

`git fetch origin` 只下载远程分支信息，不改动当前工作区；随后 `git status` 可比较本地分支和远程分支。

## 五、安全注意事项

- `project/Keil/Objects`、编译日志、映射文件、HEX/AXF 等构建产物已由 `.gitignore` 排除，通常不需要上传。
- 不要提交账号密码、Token、Wi-Fi 密码或其他敏感信息。
- 不要随意使用 `git reset --hard`，它可能直接丢弃尚未提交的本地修改。
- 不要随意使用 `git push --force`，它可能覆盖远程提交历史。
- 每完成一个独立且已经验证的功能就提交一次，提交说明应描述“改了什么”。
