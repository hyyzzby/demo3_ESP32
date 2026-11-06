# 🤖 demo3_ESP32 — ESP32 桌面陪伴机器人

用一块 ESP32 开发板 + 廉价外设，打造一个会眨眼、会转头、能被"摸头"的桌面小宠物。

![PlatformIO](https://img.shields.io/badge/framework-PlatformIO-orange)
![ESP32](https://img.shields.io/badge/chip-ESP32--WROOM--32-blue)

## ✨ 功能特性

| 模块 | 功能 |
|------|------|
| 👀 表情系统 | SSD1306 OLED 眼睛动画：眨眼 / 环视 / 开心眯眼 / 困倦 Zzz / 惊讶 |
| 🦾 头部动作 | 双舵机（水平/垂直）平滑转头、点头、开机打招呼 |
| 👆 摸头交互 | ESP32 自带电容触摸（免触摸模块），摸一下开心点头+叫两声 |
| 🔊 声音反馈 | 无源蜂鸣器旋律：开机音 / 开心叫 / 困倦哈欠 |
| 📶 WiFi 时钟 | 自动连 WiFi，NTP 同步，待机时显示时间 |
| 😴 状态机 | `待机 → 开心 → 睡眠` 自动切换；光线变暗自动睡觉 |

## 🧰 硬件清单（BOM）

| 器件 | 型号 | 数量 | 参考价 |
|------|------|------|--------|
| 主控 | ESP32 DevKit (WROOM-32) | 1 | ¥15-25 |
| 显示 | 0.96" SSD1306 OLED (I2C) | 1 | ¥8-12 |
| 舵机 | SG90 9g | 2 | ¥5/个 |
| 蜂鸣器 | 无源蜂鸣器 | 1 | ¥1 |
| 光敏 | LDR + 10K电阻分压 | 1 | ¥1 |
| 其他 | 触摸线/锡箔纸、杜邦线 | 若干 | — |

详细接线见 **[docs/hardware.md](docs/hardware.md)**

## 🚀 快速开始

### 方式一：VS Code + PlatformIO 插件（推荐）
1. 安装 [VS Code](https://code.visualstudio.com/) 和 PlatformIO IDE 扩展
2. 用 VS Code 打开本文件夹，PlatformIO 会自动下载工具链和依赖库
3. 修改 `src/config.h` 中的 WiFi 账号密码
4. 连接开发板，点底部状态栏 **→** 上传

### 方式二：命令行
```bash
pip install platformio      # 安装 CLI
pio run                     # 编译
pio run -t upload           # 烧录
pio device monitor          # 看串口日志 (115200)
```

## 📁 目录结构

```
demo3_ESP32/
├── platformio.ini      # PlatformIO 工程配置
├── docs/hardware.md    # 接线图与供电说明
└── src/
    ├── main.cpp        # 入口
    ├── config.h        # 🔧 引脚/WiFi/阈值 全部可调参数
    ├── robot.h/.cpp    # 状态机（待机/开心/睡眠）
    ├── eyes.h/.cpp     # OLED 眼睛表情动画
    ├── motion.h/.cpp   # 舵机动作（转头/点头/打招呼）
    └── sound.h/.cpp    # 蜂鸣器旋律播放器
```

## 🗺️ 路线图

- [x] v0.1 项目骨架 / 构建配置 / 文档
- [x] v0.2 OLED 眼睛表情系统
- [x] v0.3 舵机动作 + 蜂鸣器声音
- [x] v0.4 触摸交互 + 状态机 + WiFi 时钟
- [ ] v0.5 语音对话（移植 [小智AI xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)）
- [ ] v0.6 3D 打印外壳设计

## 📄 License

MIT
