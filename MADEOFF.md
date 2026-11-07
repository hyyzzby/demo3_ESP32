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

- 仓库：https://github.com/hyyzzby/demo3_ESP32 （public, main 分支）
- 本地：`D:\project-base\demo3_ESP32`
- 小智官方源码：**已并入本仓库** `xiaozhi-esp32/`（vendored 快照，含已就位的 demo3 板级 + 两处注册，见其 `UPSTREAM.md`）
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

---

# 📌 附录：2026-11-06 深夜 · 立创EDA软件内自动化进展

## 已完成 ✅
- 安装 easyeda-agent 全家桶：CLI v1.16.0（`%USERPROFILE%\.local\bin\easyeda.exe`）+ daemon（端口60832）+ Pro客户端连接器扩展（用户已导入并勾选允许外部交互）
- 客户端工程：`New Project_2026-11-06_23-14-14`（Schematic1/P1 + P2 + PCB 页）
- **P1 已放置**：MCU块（U1=ESP32-S3-WROOM-1 C9900163599 + C1 10uF + C2 1uF + C3 100nF + R1/R2 10K + 电源地标签）、AMS1117电源块（U2 C6186 + C4/C5/C6 + +5V/+3V3/GND标签）、按键块（SW1/SW2 C720477）
- **P2 已放置**：USB-C电源块（J2 TYPE-C-31-M-12 C9900012665 + D4 SS34 + D5 USBLC6 + R5/R6 5.1K CC下拉）
- B7(DN2) 漏线已补、SBU1/2 已打 NC、USB_VBUS 岛屿已并网、图签已填、两页已保存(saved:true)

## 待办 ⏳（明天从这里继续）
1. P1：12 个 out-of-sheet 图元挪回（`sch layout-lint`）、61 悬空脚处理（未用 GPIO 批量 NC：`sch no-connect`；已用脚打标签：`sch connect --pin U1:IO4 --kind net_label --net GPIO4` 逐脚）
2. P1：外设连接器 ~10 个（`easyeda lib search` 找 2.54 排针 → `sch place` → `sch connect` 绑 GPIO4/5/6/7/8/9/11/12/2/14/47 网络名）+ I2C 上拉 4.7K×2 + LDR 分压 R
3. P2：destagger --apply 再跑一次清余留重叠；peripheral-not-direct WARN 已用 USB_VBUS 标签并网（电气正确，保留 WARN 说明即可）
4. **原理图转 PCB**：查 `easyeda board`/`pcb` 命令组 → 板框 → 布局（WROOM 天线禁布区规则已内置于块）→ 布线 → 铺铜 → DRC → 保存
5. LED 块 CTRL 已绑 GPIO47；注意 U1 引脚用「名字或编号」都可（sch read 可查）

## 环境备忘
- CLI：`& "$env:USERPROFILE\.local\bin\easyeda.exe" <cmd> --project "New Project_2026-11-06_23-14-14"`
- 网络又变：git 代理已改 **127.0.0.1:21956**；直连 GitHub 目前也通
- 官方文档：docs.easyeda.com → DocumentFormat/2-Schematic-File-Format（shape 语法）

## 📌 附录2：立创EDA交互操作当前状态（2026-11-07 凌晨暂停）

### 当前页面状态（混浊，建议明早清页重来）
- P1：MCU块+AMS块+按键块完好；但**5个孤儿未删净**（D1/D2/J1/R3/R4，usbc首次失败残留，prim-delete 显示组注册移除但 sch list 仍见）+ LED块第一次落错位置(1070,870 界外)已删
- P2：USB-C块完好；**LED块误落P2**（LED1+R7，R7已删LED1还在）+ 我加的2个 USB_VBUS 标签与块内 D4_N1 命名冲突（net-marker-mismatch ×2）
- 两页均已保存(saved:true)， Autosave 也在

### 明早干净重来方案（推荐，~15分钟）
1. `easyeda sch clear --doc P1 --project "New Project_2026-11-06_23-14-14"` 清空P1，同法清P2
2. P1 顺序：block-apply esp32s3_wroom1_module --at 600,400 → tactile_boot_reset --at 150,700 → led_indicator_gpio --at 430,660 --bind CTRL=GPIO47
3. P2（page-new 后）：block-apply usbc_ufp_power_or --at 200,250 → ams1117_ldo_3v3 --at 200,600
4. U1 引脚网络标签15个（connect --pin U1:IO4 --kind net_label --net MIC_SCK …清单见下）
5. 外设连接器：lib search 排针 → sch place → sch connect
6. 每页 sch check → sch gate → sch save（必须 saved:true）

### U1 引脚网络标签清单（已验证可用的命令模式）
IO4=MIC_SCK IO5=MIC_WS IO6=MIC_SD IO7=SPK_DIN IO15=SPK_BCLK IO16=SPK_LRC IO8=SDA IO9=SCL IO11=SERVO_PAN IO12=SERVO_TILT IO2=LDR_ADC IO14=TOUCH IO47=GPIO47 IO19=USB_DP IO20=USB_DM
（NC：IO1/3/10/13/17/18/21/35/36/37/38/39/40/41/42/45/46/48；注意 EPAD 接 GND 不 NC）

### 经验教训
- block-apply 落块前**必须确认活动页**（--doc P1 有时未生效，LED块落到了P2）；apply 后立即 sch list 验证
- prim-delete 显示"组注册将移除"不等于已删——删后必须 sch list 复读
- U1 右缘引脚 stub 出线 20px 即 665 列，与块布局的器件体冲突时会静默漂浮——用 --offset 40 或先挪遮挡器件

## 📌 附录3：立创EDA PCB 设计进展（2026-11-07）

### 已完成 ✅
- 原理图 3 页全部完成（0 悬浮脚，网表验证 15 个信号网络全对）：P1=MCU+按键+LED，P2=USB-C电源+AMS1117，P3=外设接口（J21麦克风 J22功放 J23喇叭 J24 OLED J25/J26舵机 J27舵机电源 J28触摸 LDR1 R4）
- 关键修复：IO7/IO8 曾被短路到 3V3 轨（已解除）；IO19/20 网名统一为 USB_HOST_DP/DM（USB 烧录链路打通）；3V3/+3V3 轨合并
- PCB：30 器件导入 ✓、板框 60×50mm 圆角 ✓、约束布局（0 短路 0 重叠 0 出板）✓、天线禁布区 ✓、官方路由器布线（28+5/32 网）✓、双面 GND 铺铜 ✓
- 工程文件已入 git：projects_pcb/New Project_2026-11-06_23-14-14/

### DRC 剩余 20 项违规（下一段工作）
- 13× 间距错误：主要是 **+5V_SERVO 走线穿过 J23(喇叭座) SPK− 焊盘** + 少量丝印压焊盘 → 在 Pro GUI 里把 J23 挪开重新布该线，或删该线手动重布
- 10× 连接错误：未布线的网络（MCU_TX/RX 单端需补 UART 座或接受、D1_N4 孤儿线删除）
- 1× 网表错误：board rebind 检查（board list 显示绑定正常，可能为陈旧提示）
- 处理路径：GUI 里跑 DRC 面板逐条 goto 修复，或继续 CLI 迭代（`pcb drc` + 定点 `pcb modify`/`prim-delete`）

### 环境（重启电脑后恢复顺序）
1. 打开立创EDA专业版 + 打开工程 New Project_2026-11-06_23-14-14
2. `$cli = 'C:\Users\14363\.local\bin\easyeda.exe'; Start-Process $cli -ArgumentList daemon,start -WindowStyleHidden`（或任意命令自动拉起）
3. 双窗口注意：electron=原理图 / browser=PCB，命令用 --window 或 --project 路由
4. git 代理当前 127.0.0.1:21956（换网记得改）
