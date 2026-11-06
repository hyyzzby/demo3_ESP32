# 立创EDA 文件（可生成PCB）

## ⭐ 推荐文件（v0.5 芯片级载板原理图）

- `demo3_robot_s3_schematic.json` — **载板式原理图**（参考梁山派瓦力机器人的拓展板思路）：
  - **① 电源**：DC5V 座 → AMS1117-3.3（C6186）+ 输入输出电容 → 3V3 轨，带电源指示 LED
  - **② MCU**：J1 = ESP32-S3-DevKitC-1 插板主头（19 引脚，功能分组排序）
  - **③ 音频**：J3 INMP441（含 100nF 去耦）+ J4 MAX98357A + J5 喇叭座
  - **④ 显示**：J6 OLED + R2/R3 I2C 上拉
  - **⑤ 传感器**：J7 光敏 + R1 分压、J8 触摸焊盘
  - **⑥ 舵机**：J15 外部 5V 端子 + J9/J10 舵机座（⚠️ 舵机独立供电，与板共地）
  - **按键**：SW1 BOOT / SW2 RST + R5/R6 上拉；**J14** UART 编程口
- 导入后：**设计 → 原理图转PCB** → 给 J1 选 2.54 排针封装 → 布线打板
- 布线提示：I2C 两根线短、I2S 六根线远离舵机信号、电源 5V/3V3 加粗到 30mil

### 配套 PDF
`docs/schematic_pdf/demo3_robot_s3_schematic.pdf` — 工程图样式的 PDF 版（同布局），
由 `docs/schematic_pdf/demo3_robot_s3_schematic.svg` / `.html` 生成。

### 重新生成 / 调整
`docs/lceda/gen_real_schematic.ps1` — 单一数据源（组件引脚表）同时产出 JSON + SVG + PDF。

## 旧版：模块级转接图（已保留备查）

`demo3_robot_s3_schematic.json` 已被上面的载板版**覆盖**；旧的模块级设计可从 git 历史
（`d9cd75e`~`6f4e514`）找回。导入方式不变：**文件 → 导入 → 立创EDA标准版**。

## 为什么不是 SVG / .epro2？

- **SVG** 只是绘图格式，任何 EDA 软件都无法把 SVG 转成 PCB
- **`.epro2`** 是立创EDA专业版私有格式（解剖过官方瓦力机器人样例：9.8 万行私有 JSON，
  含矢量化字体缓存，手工生成无法保证可打开）
- **能导入的是**：立创EDA标准版工程 JSON ✅（本目录）· KiCad 工程 · Altium 工程

## 标准版工程 JSON 结构（解析官方样例得出的结论）

```
{ editorVersion, docType:5, title, colors,
  schematics: [ { docType:1, title, dataStr:{ head(11字段), canvas(CA~...), shape[字符串数组], BBox, colors } } ] }
```
shape 字符串语法见官方文档 docs.easyeda.com → DocumentFormat → 2-Schematic-File-Format：
`R~x~y~rx~ry~w~h~stroke~width~style~fill~id` · `T~L~x~y~0~color~~size~bold~...~text~1~start~id` ·
`P~show~elec~num~x~y~rot~id^^dot^^path~color^^name段^^number段^^反相段^^时钟段` ·
`N~x~y~0~color~NET~id~anchor~tx~ty~font~` · `W~pts~color~width~style~fill~id~lock`

## 元件采购建议（BOM，立创商城关键词）

| 位号 | 内容 | 采购 |
|------|------|------|
| U1 | AMS1117-3.3 SOT-223 | C6186 |
| J3/J4/J6 | INMP441 模块 / MAX98357A 模块 / 0.96 OLED | 成品模块焊排针 |
| J12/J15 | 5.08mm 端子 2P | 端子 5.08 2P |
| J1 | 2.54 排母 1×19（DevKitC 插板）| 排母 2.54 1x19 |
| 其余 J | 2.54 排针/排母按位号 | XH2.54 或直排 |
| SW1/SW2 | 轻触开关 | C318884 |
| R/C | 按位号值 0603 | 10K=C25804 4.7K=C25999 1K=C21190 100nF=C14663 10uF=C15850 22uF=C21379 |
| LED1 | 0603 绿 | C72043 |
| LDR1 | GL5528 | 光敏电阻 GL5528 |

## 重新生成 / 调整
`gen_real_schematic.ps1`（载板版）· `gen_lceda_json.ps1`（旧模块级）— 引脚表在脚本里，改完重跑。
