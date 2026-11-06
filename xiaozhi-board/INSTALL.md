# 🧩 demo3-robot-s3 板级包 · 移植到 xiaozhi-esp32

> 本目录是路线 B 的成品：给官方 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)
> 写的一块自定义板。3 个文件丢进官方仓库、改 2 处注册，即可 `idf.py build`。

## 它是什么

| 文件 | 内容 |
|------|------|
| `config.h` | 引脚映射（OLED 8/9 · 麦 4/5/6 · 功放 7/15/16 · 舵机 11/12 · 触摸 T14）|
| `config.json` | 板型注册（构建目标 esp32s3 + 128x64 OLED）|
| `demo3_robot_board.cc` | Board 子类：SSD1306 显示、音频编解码、**自研电容触摸按钮**（开机基线自校准，按住=说话）|
| `robot_motion.h` | 头部双舵机 LEDC 驱动 + **MCP 工具**（对小智说"点头/抬头/回正"就能控制）|

音频引脚与官方 `bread-compact-wifi` 完全一致（社区标准接线），改动最小。

## 移植步骤

### 1. 准备环境（一次性）
- 安装 **ESP-IDF v6.0.1+**（官方推荐 `release/v6.1`，v5.x 已不被 xiaozhi 支持）
- clone 官方仓库：
  ```bash
  git clone --depth 1 https://github.com/78/xiaozhi-esp32.git
  cd xiaozhi-esp32
  ```

### 2. 拷入板级包
```bash
# Windows (本仓库根目录执行)
xcopy /E /I xiaozhi-board\demo3-robot-s3 xiaozhi-esp32\main\boards\demo3-robot-s3
# Linux/macOS
cp -r xiaozhi-board/demo3-robot-s3 xiaozhi-esp32/main/boards/
```

### 3. 注册板型（2 处编辑，跟着面包板的模式抄）

**① `main/CMakeLists.txt`** — 在 `elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI)` 块**后面**加：
```cmake
elseif(CONFIG_BOARD_TYPE_DEMO3_ROBOT_S3)
    set(BOARD_DIR "demo3-robot-s3")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_14_1)
    set(BUILTIN_ICON_FONT font_material_symbols_14_1)
```

**② `main/Kconfig.projbuild`** — 在 `config BOARD_TYPE_BREAD_COMPACT_WIFI` 条目**后面**加：
```
    config BOARD_TYPE_DEMO3_ROBOT_S3
        bool "demo3 ESP32-S3 desktop robot (面包板改装机器人)"
        depends on IDF_TARGET_ESP32S3
```

### 4. 构建烧录
```bash
idf.py set-target esp32s3
idf.py menuconfig          # Xiaozhi Assistant → Board Type 选 demo3 robot
idf.py build
idf.py -p COMx flash monitor   # 烧不上按住 BOOT 再插 USB
```

### 5. 激活对话
按官方 README：配 WiFi → 登录 xiaozhi.me 添加设备输验证码 → 开聊。

## 验收清单

- [ ] 开机 OLED 显示小智标准界面
- [ ] 网页/终端看到 `cap touch ch=14 baseline=xxxx`（触摸校准成功）
- [ ] 按住头部触摸线说话，松开后 AI 回答（喇叭出声）
- [ ] 对它说「**点一下头**」→ 头部舵机点头（MCP 工具生效）
- [ ] 对它说「**看左边**」→ set_head pan=45

## 已知差异 / 后续路线

- **表情**：官方 OledDisplay 显示文字+表情图标；我们的自定义眼睛动画
  （`src/eyes.cpp`）尚未移植 —— 可后续继承 `OledDisplay` 覆写刷新钩子，把
  U8g2 眼睛画在空闲帧（文档 `docs/xiaozhi-migration.md` 路线 B 第 2 步）
- **自动睡觉**：光敏 LDR(GPIO2) 逻辑不在小智固件里，作为 MCP 工具或低电任务后续加
- **触摸语义**：当前按住说话；要改回"摸头=开心动画+自动对话"，改
  `demo3_robot_board.cc` 里 `OnPressDown` 回调即可
