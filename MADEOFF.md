# 📋 MADEOFF — 工作交接文档

> 更新时间：2026-11-06 晚（收工）
> **当前状态：v0.5.1 已发布。代码/文档/板级包全部就绪，等待硬件采购到货 + 立创EDA导入验证。**

---

## 一、项目概述

**ESP32 桌面陪伴机器人**（双硬件版本）：

| 版本 | 主控 | 能力 |
|------|------|------|
| v0.4 经典版 | ESP32-WROOM-32 | 表情/舵机/触摸/蜂鸣器/WiFi时钟 ✅ 已编译通过 |
| v0.5.1 语音版 | ESP32-S3-DevKitC-1 | 上述全部 + INMP441麦克风 + MAX98357A功放（蜂鸣器退役），回声自测就绪，小智AI迁移路线完成 |

- 仓库：https://github.com/hyyzzby/demo3_ESP32 （public, main 分支, 12 个提交）
- 本地：`D:\project-base\demo3_ESP32`
- 小智官方源码（路线B参考）：`D:\project-base\xiaozhi-esp32`（shallow clone，勿删）
- GitHub 账号：hyyzzby（gh CLI + classic token 在 keyring，git 代理已配 127.0.0.1:18899）

## 二、已完成 ✅

### 版本线
- ✅ `v0.4.0`（a0fa628）：经典版首次编译通过（RAM 14.6% / Flash 61.7%）
- ✅ `v0.5.0`（83cbf15）：语音版 — I2S 音频驱动、回声自测状态机、S3电路图/渲染图、双板条件编译（esp32dev 仍编译通过 21s）
- ✅ `v0.5.1`（1002f07）：**路线B完成** — xiaozhi 板级包 + 可交互仿真电路图
- ✅ `d9cd75e`→`a68d4a6`：立创EDA JSON（导入格式修复，**待用户验证导入**）

### 关键产物速查
| 文件 | 用途 |
|------|------|
| `docs/schematic_esp32s3_sim.svg` | 🎮 可交互仿真电路图（**浏览器打开可玩**：电源/摸头/说话/光照滑块驱动状态机）。GitHub 预览不执行脚本，必须下载后本地打开 |
| `docs/schematic_esp32s3.svg` / `render_robot.svg` | 静态电路图 / 渲染效果图 |
| `docs/lceda/demo3_robot_s3_schematic.json` | 立创EDA标准版原理图（**a68d4a6 修复为官方编辑器字符串语法**，待导入验证） |
| `docs/lceda/gen_lceda_json.ps1` | JSON 生成器（引脚表在脚本里，可改可重生成）|
| `docs/lceda/README.md` | 导入步骤 + 元件采购关键词 |
| `xiaozhi-board/demo3-robot-s3/` | **路线B成品**：小智官方仓库自定义板级包（config.h/config.json/板级实现/舵机MCP工具）|
| `xiaozhi-board/INSTALL.md` | 移植指南（拷贝命令 + CMakeLists/Kconfig 两处注册改法 + 构建/激活步骤）|
| `docs/xiaozhi-migration.md` | 三条迁移路线总览（路线B章节已更新为"成品就绪"）|
| `docs/hardware.md` | 双版本接线表 |

### 代码模块（src/）
`config.h`（双板引脚宏 ROBOT_IS_S3）· `eyes`（表情）· `motion`（舵机）· `sound`（双后端：蜂鸣器/I2S合成）· `audio`（S3 I2S mic+spk）· `voice`（语音状态机）· `robot`（状态机+语音触发）

## 三、未完成 ⏳（明天的活）

1. **立创EDA导入验证**（用户侧）：`a68d4a6` 修复后的 JSON 还没导入过。失败的话把报错截图给我——手里有官方格式文档（`$TEMP` 已清理，需要重抓 docs.easyeda.com/assets/en_DocumentFormat_2-...index.md.CB0u3sfi.js）
2. **硬件采购**（用户侧）：ESP32-S3-DevKitC-1 N8R8(¥40) + INMP441(¥6) + MAX98357A(¥6) + 8Ω3W喇叭(¥5)
3. **到手后**：`pio run -e esp32s3`（首拉 ~130MB S3 工具链，走代理慢但能成，参考中午的经历别误杀）→ 回声自测 → 走 `xiaozhi-board/INSTALL.md` 刷小智
4. **路线B后续**：表情移植（eyes.cpp → 小智 OledDisplay 空闲帧钩子）、光敏自动睡觉（MCP工具或低电任务）
5. **WiFi 配置**：`src/config.h` 占位符（⚠️ 真实密码别提交）
6. **安全收尾**：⚠️ 对话中暴露过 2 个 fine-grained token（已无权限，仍建议 Revoke）+ 1 个 classic token（建议轮换）

## 四、问题 & 解决方法档案 🔧（累计 12 条）

#1-#8 见 git 历史 v0.4.0 版本 MADEOFF.md；今日新增：

| # | 问题 | 根因 | 解决 |
|---|------|------|------|
| 9 | framework 235MB 反复下载失败 | 代理对大文件掉链子；registry API 400 因 framework 在 registry 里是 **tool** 类型 | API 拿 URL+sha256 → curl 直连（`--retry 30 -C -`）→ 读 PIO 源码发现缓存 key=`sha1(url+sha256)`，种进 `.cache/downloads/` 秒过 |
| 10 | platformio.ini 未锁版本拉了 8.0.0 工具链 | `platform = espressif32` 解析到最新 | 锁 `espressif32@6.12.0`（8bc19d4）|
| 11 | 编译错 SERVO_PAN_CENTER 未定义 | motion.h 没包含 config.h | 补 include（唯一代码 bug）|
| 12 | **SVG 无法导入 EDA 生成 PCB**；`.epro2` 手工生成不可行 | SVG 是绘图格式；`.epro2` 内部是 9.8 万行私有 JSON（矢量化字体缓存），无公开规范 | 走官方通道：**立创EDA标准版 JSON**。语法从 docs.easyeda.com 官方文档 + LCSC API 真实器件数据（`easyeda.com/api/products/{id}/components` 的 dataStr）双重对齐。第一版对象式结构导入失败 → 第二版字符串语法（a68d4a6）待验证 |

### 今日关键配方
```powershell
# 立创EDA格式参考（重新获取官方文档）
curl.exe --proxy http://127.0.0.1:18899 -L "https://docs.easyeda.com/assets/en_DocumentFormat_2-EasyEDA-Schematic-File-Format_index.md.CB0u3sfi.js"
# 真实器件格式基准（dataStr.SYMBOL.shape[] 即编辑器字符串语法）
curl.exe --proxy http://127.0.0.1:18899 "https://easyeda.com/api/products/C16581/components"
```

## 五、明日开工顺序 📌

1. 问用户：立创EDA 导入成功没？成功 → 协助选封装转 PCB；失败 → 按报错修 JSON（对账官方语法）
2. 用户 WiFi 给了的话 → 改 config.h → 提交（注意别提交真实密码，或用 skip-worktree）
3. 硬件到货 → S3 工具链下载（`$env:HTTP(S)_PROXY=http://127.0.0.1:18899`）→ 回声自测 → 小智刷机
4. 可选：把 MAEDOFF 过程整理成博客/论坛帖（oshwhub 社区风格，像瓦力机器人那样）

## 六、常用命令速查

```bash
# 构建（⚠️ 必须带 -e，否则会构建全部环境并拉 S3 工具链）
py -m platformio run -e esp32dev        # 经典版
py -m platformio run -e esp32s3         # 语音版（首次拉工具链）
py -m platformio run -e esp32s3 -t upload
py -m platformio device monitor          # 115200

# git（代理已配 18899；若换网端口变了记得 git config --global http.proxy 更新）
git add -A; git commit -m "..."; git push

# gh CLI
& "C:\Program Files\GitHub CLI\gh.exe" <cmd>
```

---
*项目仓库含 12 提交 / 2 tag（v0.4.0, v0.5.1）/ MIT License。由 OpenCode (GLM) 协助完成。*
