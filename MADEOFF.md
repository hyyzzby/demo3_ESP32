# 📋 MADEOFF — 交接文档（最终版）

> 更新：2026-11-07 深夜 · 状态：**原理图完成 ✓ PCB 布局完成 ✓ 布线 17/31 ⚠ DRC 待清零 ⚠**
> 仓库：https://github.com/hyyzzby/demo3_ESP32 · 本地：`D:\project-base\demo3_ESP32`

---

## 一、项目全景

ESP32-S3 桌面陪伴机器人（语音版）：WROOM-1 模组 + AMS1117 电源 + USB-C 防倒灌
+ INMP441 麦克风 + MAX98357A 功放 + OLED + 双舵机 + 光敏 + 触摸。

| 产出 | 位置 |
|------|------|
| 固件代码 v0.5.1（可编译） | `src/` · tag v0.4.0 / v0.5.0 / v0.5.1 |
| 立创EDA工程（原理图3页+PCB） | 立创EDA客户端工程 `New Project_2026-11-06_23-14-14`，文件已入 git `projects_pcb/` |
| 载板级 JSON（导入生成PCB 备查） | `docs/lceda/demo3_robot_s3_schematic.json` |
| 可交互仿真电路图 | `docs/schematic_esp32s3_sim.svg`（浏览器打开可玩）|
| 渲染效果图 / 静态电路图 | `docs/render_robot.svg` / `docs/schematic_esp32s3.svg` |
| 接线表 | `docs/hardware.md` |
| 小智AI迁移路线 | `docs/xiaozhi-migration.md` |
| 小智板级包 | `xiaozhi-board/demo3-robot-s3/`（已并入 `xiaozhi-esp32/main/boards/`）|

## 二、明日待办（按优先级）

### PCB 收尾（立创EDA专业版 GUI，预计 30 分钟）
1. 打开 PCB1 → 设计 → 规则设置 → **铺铜间距改 20mil**
2. 右键每块 GND 铺铜 → **重建铜皮**（消除铺铜间距违规）
3. DRC 面板（设计→检查 DRC）逐条处理：
   - J23/J26/J27 与 SW2 间距 → 已由用户拖开大部分，残余逐条 goto 微调
   - **+5V_SERVO 走线穿 J23.2 焊盘** → 删该线，手动重布（从 J27.1 出发避开 J23）
   - R1 距板边 → 已由用户手工挪位 ✓（确认即可）
4. 未布通 4 对连线（均 <2mm 短线，交互布线点击即通）：
   - EN：U1.3 ↔ R1.1
   - 3V3：U1.2 ↔ R1.2
   - USB_HOST_DM：D2.1 ↔ U1.14
   - D1_N4(CC2)：J1.B5 ↔ R5.1
5. 丝印整理：工具 → 丝印对齐（~8 处压焊盘）
6. 全绿后：文件 → 导出 → **PCB 制造文件 Gerber** → 嘉立创下单

### 固件侧（PCB 打板回来前可先做）
1. 改 `src/config.h` WiFi 密码（⚠️ 不提交真实密码）
2. `pio run -e esp32s3` 编译验证（首次拉 S3 工具链，走代理慢但能成）
3. 可选：UART 座（1×4）补装到 PCB 上（预留 UART 烧录备份口）

### 安全
- ⚠️ 对话中暴露过 token → 全部去 GitHub 轮换/Revoke

## 三、踩坑全记录（18 条，按时间线）

### 环境/网络类
| # | 坑 | 根因 | 解法 |
|---|-----|------|------|
| 1 | git push 连接重置 | 国内直连 GitHub 被墙 | `git config --global http.proxy http://127.0.0.1:<代理端口>` |
| 2 | push 403 | fine-grained token 无 Contents 写权限 | 换 classic token（repo scope）一步到位 |
| 3 | 建仓 403 | fine-grained token 无 Administration | 同上 |
| 4 | PIO 下载反复卡 0MB | ①requests 自动读**系统代理**（非 git 配置）②直连 registry 302 目标不可达 | 显式 `$env:HTTP(S)_PROXY`；慢≠死，**别误杀**（曾误杀 40% 进度）|
| 5 | curl 测速 403 | 立创/PIO CDN 校验 UA | 加 `-A "PlatformIO/6.2.0"` |
| 6 | registry API 查包 404/400 | framework 在 registry 注册为 **tool** 类型（PackageType 只有 library/platform/tool）| 读 PIO 源码 `PackageType` 类确认 |
| 7 | CLI 脚本误杀自身 | 进程过滤 `-like "*platformio*"` 匹配到自身命令行里的 URL | 过滤词避开自身参数含的字符串 |
| 8 | PS 管道看似无输出 | `Select-Object -Last N` 吞掉全部流式输出 | 长任务 `Out-File` 实时写日志再 tail |
| 9 | framework 235MB 反复下载失败 | 代理对大文件不稳 | curl `--retry 30 -C -` 断点续传 + **PIO 缓存种子**（sha1(url+sha256) → `.cache/downloads/`）|
| 10 | platformio.ini 未锁版本拉了 8.x | `platform = espressif32` 解析到最新 | 锁 `espressif32@6.12.0` |
| 11 | 编译错 SERVO_PAN_CENTER | motion.h 没包含 config.h | 补 include |
| 12 | SVG 导入 EDA 失败 | SVG 是绘图格式，EDA 无法提取电气网络 | 走标准版 JSON 通道 |

### 立创EDA自动化类（easyeda-agent）
| # | 坑 | 根因 | 解法 |
|---|-----|------|------|
| 13 | `.epro2` 手工生成不可行 | 9.8 万行私有 JSON（含矢量化字体缓存），无公开规范 | 放弃，走标准版 JSON 导入通道 |
| 14 | 标准版 JSON 导入报"格式不正确" | 首版用对象式结构（臆造）| 抓**官方文档** docs.easyeda.com + **LCSC API 真实器件 dataStr** 双重对齐：**工程信封 `{editorVersion, docType:5, schematics:[{docType:1, dataStr:{head,canvas,shape[]}}]}`** + shape 紧凑字符串语法 |
| 15 | registry API 查 framework 400 | framework 在 registry 注册为 tool 类型 | `/v3/packages/platformio/tool/framework-arduinoespressif32` |
| 16 | Electron 桌面窗口 autoroute 连续失败 | **官方路由器只在浏览器宿主 Web 引擎可用** | 明日方案A：浏览器打开 pro.lceda.cn 让连接器连上再跑 |
| 17 | daemon 随 PC 重启消失 | daemon 非系统服务 | `Start-Process easyeda -ArgumentList daemon,start -WindowStyle Hidden` |
| 18 | 双窗口 AMBIGUOUS_WINDOW/PROJECT | 浏览器+桌面各连一个窗口 | 所有命令带 `--project` 或 `--window <id>`；ID 从 `easyeda health` 实时取（重启会变）|

### PowerShell 类
| # | 坑 | 解法 |
|---|-----|------|
| A | `.ps1` 无 BOM → PS5.1 按 GBK 解析中文乱码/报错 | `[System.IO.File]::WriteAllText($p,$c,(New-Object Text.UTF8Encoding $true))` 补 BOM |
| B | 函数内 `$arr +=` 修改的是局部副本 | 用 `$script:arr` 或改用 `[System.Collections.Generic.List[string]]` + `.Add()` |
| C | 变量跨工具调用不保留 | 每次命令自包含（重新赋值所有变量）|
| D | `--%` 停止解析符后不能接管道 | 单独运行，输出验证走文件 |
| E | JSON spec 文件带 BOM → Go 解析报 `invalid character 'ï'` | `UTF8Encoding $false` 无 BOM 写入 |

## 四、立创EDA 自动化命令速查（已验证可用）

```bash
# 环境
easyeda health                          # daemon+connector+工程状态
easyeda daemon start                    # 后台启动（Hidden 窗口）
# 原理图
easyeda sch pages/list/read/check/save  # 读写验证
easyeda sch block-apply <id> --at x,y --bind PORT=NET --project <名>
easyeda sch place --lib <libUuid> --uuid <devUuid> --x --y --designator J21
easyeda sch connect --pin U1:IO4 --kind net_label --net MIC_SCK
easyeda sch autoconnect --spec p1_connect.json --replace   # 批量几何感知连接
easyeda sch no-connect --designator U1 --pin "39,15,18,..."
easyeda sch titleblock --doc P2 --data '{"Name":{"showTitle":true,...}}'  # 嵌套格式
# PCB
easyeda pcb import-changes              # 原理图→PCB 同步
easyeda pcb outline-round --rect 0,0,2362,1969 --radius 80
easyeda pcb arrange --ids <csv> --mode cluster|grid
easyeda pcb place-constrained           # 分层约束布局（边座/锚/卫星合法化）
easyeda pcb antenna-keepout             # WROOM 天线禁布区
easyeda pcb autoroute                   # ⚠️ 仅浏览器宿主可用
easyeda pcb route-short                 # 短网 L 形自布线
easyeda pcb pour-fit --net GND --layer 1|2 --inset 25
easyeda pcb pour-rebuild                # 布线变更后必跑
easyeda pcb modify --id <pid> --center --x --y
easyeda pcb drc / layout-lint           # 检查
# 库搜索
easyeda lib search --query "2.54mm 1x6 排针" --project <名>
easyeda lib by-lcsc C6186              # LCSC 编号直查
```

## 五、关键教训总结

1. **先布线后铺铜**——铜箔会堵死路由器
2. **挪器件后旧走线不跟随**——需 rip-up + 重新布线
3. **EasyEDA 官方路由器只在浏览器宿主可用**（Electron 桌面不行）
4. **PIO 下载器走系统代理而非 git 代理**；大文件用 curl 断点续传
5. **每次 CLI 操作后必须回读验证**（"OK"不等于"生效"）
6. **--designator 传给 sch place 是原子赋值**，但 block-apply 内部件是自动分配的——位号冲突要去 P1 页面看已分配的
7. **立创EDA 的 XH-2P SMD 封装含宽体壳脚**（421×451 mil），布局时按 4 脚算不是 2 脚
