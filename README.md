# 🤖 demo3_ESP32 — ESP32 桌面陪伴机器人

用一块 ESP32 开发板 + 廉价外设，打造一个会眨眼、会转头、能被"摸头"、**还能跟你说话**的桌面小宠物。

![PlatformIO](https://img.shields.io/badge/framework-PlatformIO-orange)
![ESP32](https://img.shields.io/badge/chip-ESP32--S3%20%7C%20WROOM--32-blue)
![voice](https://img.shields.io/badge/voice-小智AI--ready-8b5cf6)

## 🆕 v0.5 语音版亮点

- 🎙 **INMP441 麦克风 + MAX98357A 功放**（I2S 双通道，蜂鸣器光荣退休）
- 🔊 **回声自测**：摸头 → "滴" → 说话 3 秒 → "嘟" → 喇叭回放，一键验证语音链路
- 🗣 **小智AI 迁移路线**：`docs/xiaozhi-migration.md` 三条路线（官方固件直刷 / 源码二开 / 自研协议）
- 📐 **电路图**：`docs/schematic_esp32s3.svg` · **渲染图**：`docs/render_robot.svg`
- 🔀 **双板同源**：一套代码 `ROBOT_IS_S3` 条件编译，经典 WROOM-32 版继续可用

## ✨ 功能特性

| 模块 | 功能 |
|------|------|
| 👀 表情系统 | SSD1306 OLED 眼睛动画：眨眼 / 环视 / 开心眯眼 / 困倦 Zzz / 录音时惊讶瞪眼 |
| 🦾 头部动作 | 双舵机（水平/垂直）平滑转头、点头、开机打招呼 |
| 👆 摸头交互 | 电容触摸，摸一下开心点头 + 触发语音交互 |
| 🎙 语音对话 | v0.5 回声自测就绪 → 小智AI 云端对话（见迁移文档） |
| 🔊 声音反馈 | 经典版：蜂鸣器旋律；S3 版：功放正弦合成提示音 |
| 📶 WiFi 时钟 | 自动连 WiFi，NTP 同步，待机时显示时间 |
| 😴 状态机 | `待机 → 开心 → 睡眠` 自动切换；光线变暗自动睡觉 |

## 🧰 硬件清单（BOM）

| 器件 | 型号 | 数量 | 参考价 |
|------|------|------|--------|
| 主控 | ESP32-S3-DevKitC-1 (N8R8) | 1 | ¥40 |
| 显示 | 0.96" SSD1306 OLED (I2C) | 1 | ¥10 |
| 麦克风 | INMP441 (I2S MEMS) | 1 | ¥6 |
| 功放 | MAX98357A (I2S) + 8Ω3W喇叭 | 1 | ¥11 |
| 舵机 | SG90 9g | 2 | ¥10 |
| 光敏 | LDR + 10K电阻分压 | 1 | ¥1 |
| 其他 | 锡箔纸/触摸线、杜邦线 | 若干 | — |

> 经典 WROOM-32 + 无源蜂鸣器（¥27）仍受支持：`pio run -e esp32dev`

详细接线见 **[docs/hardware.md](docs/hardware.md)** ·
电路图 **[docs/schematic_esp32s3.svg](docs/schematic_esp32s3.svg)** ·
渲染图 **[docs/render_robot.svg](docs/render_robot.svg)**

## 🚀 快速开始

### 方式一：VS Code + PlatformIO 插件（推荐）
1. 安装 [VS Code](https://code.visualstudio.com/) 和 PlatformIO IDE 扩展
2. 打开本文件夹，自动下载工具链
3. 修改 `src/config.h` 中 WiFi 账号密码
4. 底部状态栏切换环境（esp32dev / **esp32s3**），点 **→** 上传

### 方式二：命令行
```bash
pip install platformio
pio run -e esp32s3            # 编译语音版（经典版用 -e esp32dev）
pio run -e esp32s3 -t upload  # 烧录（烧不上按住 BOOT 插 USB）
pio device monitor            # 串口日志 (115200)
```

### 🗣 语音对话升级（小智AI）
硬件装好后先开机验证回声自测，然后按 **[docs/xiaozhi-migration.md](docs/xiaozhi-migration.md)**
三条路线任选其一升级为真正的云端 AI 对话。

## 📁 目录结构

```
demo3_ESP32/
├── platformio.ini         # 双环境：esp32dev / esp32s3
├── docs/
│   ├── hardware.md        # 接线表（S3 + 经典版）
│   ├── schematic_esp32s3.svg  # 电路连接图
│   ├── render_robot.svg   # 机器人渲染效果图
│   └── xiaozhi-migration.md   # 小智AI 迁移路线 ⭐
└── src/
    ├── main.cpp           # 入口
    ├── config.h           # 🔧 双板引脚/WiFi/音频参数
    ├── robot.h/.cpp       # 状态机（待机/开心/睡眠 + 语音触发）
    ├── eyes.h/.cpp        # OLED 眼睛表情动画
    ├── motion.h/.cpp      # 舵机动作
    ├── sound.h/.cpp       # 旋律播放器（蜂鸣器/I2S 双后端）
    ├── audio.h/.cpp       # 🎙 I2S 音频驱动（S3：麦克风+功放）
    └── voice.h/.cpp       # 🎙 语音交互状态机（S3：回声自测→云端对话）
```

## 🗺️ 路线图

- [x] v0.1 项目骨架
- [x] v0.2 OLED 眼睛表情系统
- [x] v0.3 舵机动作 + 蜂鸣器声音
- [x] v0.4 触摸交互 + 状态机 + WiFi 时钟（首个编译通过 tag）
- [x] v0.5 语音硬件底座：I2S 音频驱动 + 回声自测 + 小智迁移文档
- [ ] v0.6 接入小智AI 云端对话（路线 B）
- [ ] v0.7 3D 打印外壳设计

## 📄 License

MIT
