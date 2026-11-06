# 📋 MADEOFF — 工作交接文档

> 更新时间：2026-11-06 下午（v0.4.0 发布）
> **当前状态：✅ 编译通过，v0.4.0 已打标签发布。剩余工作只有硬件实测。**

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

### 工具链下载（已完成 ✅ 2026-11-06 暂停后自动跑完）
- ✅ 平台 espressif32@6.12.0 → 已装好
- ✅ toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5（397MB）→ 已装好
- ✅ tool-esptoolpy @ 2.40900.250804 → 已装好
- 结论：**编译环境完全就绪，续接时直接从「五、下一步计划」第 2 步开始**

## 三、未完成 ⏳

1. ~~编译验证~~ ✅ 2026-11-06 下午完成：RAM 14.6% / Flash 61.7%，tag `v0.4.0` 已推送
2. ~~库安装~~ ✅ U8g2 / ESP32Servo 已随构建自动装好
3. ~~修编译错误~~ ✅ `motion.h` 补 `#include "config.h"`（a0fa628）
4. ~~打 tag~~ ✅ `v0.4.0` 已推送
5. **WiFi 配置**：`src/config.h` 第 26-27 行占位符，烧录前需改成真实 WiFi（⚠️ 不要提交真实密码）
6. **硬件实测**：用户烧录 + 按 docs/hardware.md 接线验证（串口日志 115200 观察状态机流转）
7. **安全收尾**：⚠️ 对话中暴露过 2 个 fine-grained token（均权限不全）→ 去 github.com/settings/personal-access-tokens **Revoke**；classic token（ghp_YiHS...）建议轮换

### 编译环境备忘（换机/重装时用）
- 平台 espressif32@6.12.0（platformio.ini 已锁定版本）、Arduino core 2.0.17
- 已装：toolchain-xtensa-esp32@8.4.0、tool-esptoolpy@2.40900、framework-arduinoespressif32@3.20017.241212
- 框架包 235MB 的离线安装妙招：curl 直连下载 tarball → 算 `sha1(url + sha256)` → 放进 `~/.platformio/.cache/downloads/<sha1>` → `pio run` 自动识别为缓存命中，全程不走网络（详见问题 #9）
- 库 U8g2/ESP32Servo 小文件，python 下载器可正常拉取

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
| 9 | framework 包 235MB 反复下载失败 | ① 代理客户端对大文件掉链子（换网/换端口都一样）；② registry API 一直 400 是因为 **framework 在 registry 里是 tool 类型**（PackageType 只有 library/platform/tool） | 三连招：API 拿精确 URL+sha256 → curl 直连下载（`--retry 30 -C -`）→ **读 PIO 源码**发现缓存 key = `sha1(url+sha256)`，把 tarball 种进 `.cache/downloads/` 秒过 ✅ |
| 10 | `pio pkg install` 没有 `--framework` 选项 | 框架包只能由平台构建时自动安装 | 用问题 #9 的缓存种子方案绕过 |
| 11 | 编译报 `SERVO_PAN_CENTER was not declared` | motion.h 用了 config.h 的宏但没 include | motion.h 顶部加 `#include "config.h"` ✅（唯一一个代码错误！） |

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
