# 📋 MADEOFF — 工作交接文档

> 更新时间：2026-11-06 中午（暂停前）
> 用途：记录 demo3_ESP32 项目当前状态，方便下午（或任何人）无缝续接

---

## 一、项目概述

**ESP32 桌面陪伴机器人**：ESP32-WROOM-32 + SSD1306 OLED（眼睛表情）+ 双 SG90 舵机（头部动作）
+ 电容触摸（摸头）+ 无源蜂鸣器（叫声）+ 光敏（天黑睡觉）+ WiFi/NTP（时钟）。

- 仓库：https://github.com/hyyzzby/demo3_ESP32 （public, main 分支）
- 本地：`D:\project-base\demo3_ESP32`
- 框架：PlatformIO + Arduino（平台 espressif32@6.12.0 = Arduino core 2.0.17）
- 账号：GitHub **hyyzzby**（gh CLI 已登录，classic token 存于 keyring）

## 二、已完成 ✅

### 代码（4 个提交，已全部推送 GitHub）
| 提交 | 内容 |
|------|------|
| `4d83f38` | v0.1 骨架：platformio.ini、README、docs/hardware.md（接线图）、.gitignore |
| `6a6bb44` | v0.2 表情：config.h（全部引脚/阈值）、eyes.h/.cpp（U8g2 眨眼/环视/开心/睡觉/惊讶） |
| `bd11702` | v0.3 动作声音：motion.h/.cpp（舵机 look/center/rest/nod/hello 平滑+时间轴）、sound.h/.cpp（LEDC 非阻塞旋律） |
| `7e48e27` | v0.4 整合：robot.h/.cpp（Idle/Happy/Sleep 状态机、触摸消抖、暗光入睡、WiFi保活、NTP）、main.cpp |

### 环境
- ✅ Git 2.56、Python 3.12（`py` 启动器）、PlatformIO Core 6.2.0（`py -m platformio`）
- ✅ gh CLI 2.103.3（`C:\Program Files\GitHub CLI\gh.exe`），classic token 已入库
- ✅ git 全局代理已配：`http.proxy = http://127.0.0.1:16470`（push 正常）
- ✅ 系统代理确认：`127.0.0.1:16470`（约 2.8MB/s 吞吐）
- ✅ 本地仓库 git 身份（占位符）：demo3-dev / demo3-dev@users.noreply.github.com

### 工具链下载（进行中，暂停时状态）
- ✅ 平台 espressif32@6.12.0 → **已装好**
- ⏳ toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5（~140MB，正在下载）
- ⏳ tool-esptoolpy（~2MB，排队）
- 日志：`D:\pio_pkgs\install612.log`

## 三、未完成 ⏳

1. **编译验证**：`pio run` 从未成功跑完（一直卡在下载阶段），代码有编译错误的可能
2. **库安装**：lib_deps（U8g2、ESP32Servo）尚未安装，`pio run` 会自动装（小文件走 python 下载器一般没问题，不行就手动 curl）
3. **修编译错误 → commit + push**
4. **打 tag**：`v0.4.0` 并推送（首个正式版）
5. **WiFi 配置**：`src/config.h` 第 26-27 行占位符，需用户真实 WiFi（⚠️ 不要把真实密码提交，建议改成编译时 `-DWIFI_SSID=...` 注入或 git update-index --skip-worktree）
6. **硬件实测**：用户侧烧录 + 接线验证（docs/hardware.md）
7. **安全收尾**：⚠️ 对话中暴露过 2 个 fine-grained token（均权限不全）→ 应去 github.com/settings/personal-access-tokens **Revoke**；classic token（ghp_YiHS...）也建议下午轮换

## 四、遇到的问题 & 解决方法 🔧

| # | 问题 | 根因 | 解决 / 备注 |
|---|------|------|------------|
| 1 | git push `Connection reset` | 国内直连 github.com 被墙 | `git config --global http.proxy http://127.0.0.1:16470` ✅ |
| 2 | push 403 denied | fine-grained token 无 Contents 写权限 | 先试补权限未果 → **换 classic token（repo scope）** ✅ |
| 3 | 建仓失败 `Resource not accessible` | fine-grained token 无 Administration 建仓权限 | 用户换 token 解决 |
| 4 | PIO 下载卡 0MB（两轮） | ① requests 自动读**系统代理**（不是 git 配置）→ 走代理但隧道慢；② 直连则 registry 302 跳转目标不可达 | 显式 `$env:HTTP(S)_PROXY` 走代理；慢≠死，别轻易杀（我曾误杀 40% 进度） |
| 5 | curl 测速 403 | registry CDN 校验 UA | 加 `-A "PlatformIO/6.2.0"` 即通 ✅（这是保底手动下载配方） |
| 6 | registry API 查包 404/400 | **owner 猜错了**：工具链在 `espressif` 名下，工具在 `platformio` 名下，框架类型路径带 framework 会 400 | 6.12.0 工具链真实版本：`espressif/toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5`（从安装日志拿到） |
| 7 | 误杀自己的 shell（exit 255） | 进程过滤 `-like "*platformio*"` 匹配到了自己命令行里的 URL | 过滤词避开自身命令行包含的字符串 |
| 8 | PowerShell 管道看似无输出 | `Select-Object -Last N` 会吞掉全部流式输出直到结束 | 长任务用 `Out-File` 实时写日志再 tail |

**关键配方（保底，已验证）**：
```powershell
# 手动下载任意 PIO 包（UA + 系统代理 16470）
curl.exe -A "PlatformIO/6.2.0" --proxy http://127.0.0.1:16470 -L -o pkg.tar.gz "<registry URL>"
# 然后离线安装
py -m platformio pkg install --global --tool "file://D:\pio_pkgs\pkg.tar.gz"
```

## 五、下一步计划 📌（按序执行）

1. `Get-Content D:\pio_pkgs\install612.log -Tail 15` — 看工具链装完没
   - 装完 → 直接第 3 步
   - 卡住 → 看日志百分比：有增长就等；0 动就杀进程，用「关键配方」手动下载后离线装
2. 库安装：`pio run` 首次会装 U8g2/ESP32Servo（小文件，大概率直接成功）
3. `py -m platformio run` 编译 → 有报错就修（重点检查：U8g2 `drawRBox` 参数、ESP32Servo `allocateTimer`、core 2.0.17 的 `ledcSetup/ledcWriteTone` 通道 API、`touchRead` 值域）→ commit + push
4. `git tag v0.4.0 && git push origin v0.4.0`
5. 提醒用户改 `config.h` WiFi → 用户烧录实测
6. （可选）token 轮换 + revoke 暴露过的 token
7. （远期 roadmap 在 README）：v0.5 小智AI 语音、v0.6 3D 外壳

## 六、常用命令速查

```bash
# 编译 / 烧录 / 串口
py -m platformio run
py -m platformio run -t upload
py -m platformio device monitor        # 115200

# 提交推送（git 代理已配好）
git add -A; git commit -m "..."; git push

# gh CLI（完整路径，PATH 可能没刷新）
& "C:\Program Files\GitHub CLI\gh.exe" <cmd>
```

---
*由 OpenCode (GLM) 协助编写。硬件接线见 docs/hardware.md，参数调整见 src/config.h。*
