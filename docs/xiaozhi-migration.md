# 🎙 小智AI (xiaozhi-esp32) 迁移路线

> 目标：把本项目从「回声自测」升级为「云端 AI 语音对话」。
> 本文档是续接指南——按顺序执行即可，不需要重新摸索。

## 一、为什么不能直接"塞进"本项目

[xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 是**完整独立固件**（ESP-IDF 构建），
包含：WebSocket/MQTT 通信协议、Opus 音频编解码、唤醒词、OTA、板级抽象层等 100+ 文件。
它的正确用法是 **以它为底座**（选一块官方支持的开发板），而不是把它的代码拷进 PlatformIO 工程。

硬件上也绕不开：**语音对话强烈依赖 ESP32-S3 + PSRAM**（Opus 解码 + 音频缓冲 + WiFi 并发），
经典 ESP32-WROOM-32 官方不支持。这就是 v0.5 把电路图全部改画成 S3 的原因。

## 二、硬件清单（已按 v0.5 电路图备好）

| 器件 | 作用 | 参考价 |
|------|------|--------|
| ESP32-S3-DevKitC-1 (N8R8) | 主控，PSRAM 必须 | ¥40 |
| INMP441 | I2S 全向麦克风 | ¥6 |
| MAX98357A + 8Ω3W喇叭 | I2S 功放+喇叭 | ¥11 |
| 0.96" OLED、SG90×2、LDR | 沿用现有 | — |

接线与 `docs/schematic_esp32s3.svg` 一致；引脚宏在 `src/config.h` 的 `ROBOT_IS_S3` 区块。

## 三、迁移步骤（三条路线，选一条）

### 路线 A：官方固件直接刷（最快获得真语音对话，1 小时）
1. 按官方 README 用 ESP-IDF 5.3+/idf.py 或在线 [xiaozhi.me](https://xiaozhi.me) 控制台激活设备
2. 选 `boards/esp32s3` 系列板级配置（我们的引脚与其默认板不同，
   需复制 `esp32-s3-devkitc-1` 板级目录，改 `config.json` 里 I2S/I2C GPIO 为我们的映射）
3. 注册 xiaozhi.me → 添加设备 → 输入验证码 → 对话即通
4. **代价**：我们的表情/舵机代码不在此固件里（只有语音）

### 路线 B：xiaozhi 源码二次开发（推荐终点）——✅ 成品已就绪

**迁移工作已经完成**！官方源码已并入本仓库 `xiaozhi-esp32/`，且
[`main/boards/demo3-robot-s3/`](../xiaozhi-esp32/main/boards/demo3-robot-s3/) 板级包已拷入、
CMakeLists/Kconfig 两处注册已改好（见 `xiaozhi-esp32/UPSTREAM.md`）——装好 ESP-IDF 即可构建：

1. 按官方 README 安装 **ESP-IDF v6.0.1+**（推荐 release/v6.1，v5.x 不再支持）
2. `cd xiaozhi-esp32 && idf.py set-target esp32s3`
3. `idf.py menuconfig` → Xiaozhi Assistant → Target Board 选 **demo3 robot**
4. `idf.py build flash monitor` → xiaozhi.me 激活 → **开聊**
5. 对机器人说「点一下头」「看左边」—— 舵机经 MCP 工具响应语音指令 ✨

已在板级包里实现的定制：
- 电容触摸 T14（自研基线校准按钮，按住=说话）
- 头部双舵机 MCP 工具（`self.robot.set_head / nod / center`）
- OLED 8/9 引脚 + 128x64 分辨率
- 移除官方面包板的音量键/灯控 GPIO

### 路线 C：本仓库内自研精简协议（学习向）
不用 xiaozhi 固件，直接对接兼容其 WebSocket 协议的服务端：
- `src/voice.cpp` 的状态机已预留接口：`Recording` 结束后取 `recordData()` 送 ASR，TTS 返回后 `playStart()`
- 缺 Opus 编解码（需移植 esp32-opus 组件，S3+PSRAM 可跑）和流式协议栈
- 适合想吃透原理的场景，工作量最大

## 四、本仓库已有的「语音骨架」清单

| 文件 | 内容 |
|------|------|
| `src/audio.h/.cpp` | I2S0 收麦(INMP441 32bit→16bit) + I2S1 功放(MAX98357A) + 正弦提示音合成，全非阻塞 |
| `src/voice.h/.cpp` | 语音交互状态机：滴→录3s→嘟→回放（回声自测），`Phase` 可直接替换为云端对话流程 |
| `src/sound.cpp` S3 分支 | 原蜂鸣器旋律 → I2S 功放合成，上层 API 零改动 |
| `src/config.h` S3 区块 | 全部引脚宏，与电路图一一对应 |
| `platformio.ini [env:esp32s3]` | 一键构建烧录（首次拉 ~130MB S3 工具链） |

## 五、验收清单

- [ ] `pio run -e esp32s3` 编译通过
- [ ] 开机自测：摸头 → "滴" → 对麦克风说话 3 秒 → "嘟" → 喇叭回放出你的声音
- [ ] 路线 A/B 任一条：实现真实云端对话
- [ ] （加分）对话时 OLED 嘴巴动画 + 舵机点头
