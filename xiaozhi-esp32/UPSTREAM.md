# ⚠️ Vendored 上游源码说明

本目录是 [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 的**完整快照**
（2026-11-06 shallow clone，commit 时无本地改动），已并入 demo3_ESP32 仓库统一版本管理。

## 已施加的本地修改（相对上游）

1. **新增板级** `main/boards/demo3-robot-s3/` —— 来自本仓库 `xiaozhi-board/demo3-robot-s3/`
   （本仓库根目录的 `xiaozhi-board/` 是源模板，**两边同步改**，或以后只改这里并反向同步）
2. `main/CMakeLists.txt` —— 注册 `CONFIG_BOARD_TYPE_DEMO3_ROBOT_S3` → BOARD_DIR
3. `main/Kconfig.projbuild` —— 新增板型选项 `demo3 ESP32-S3 desktop robot`

## 更新上游的方法

```bash
# 在临时目录重新 shallow clone，替换除 demo3-robot-s3 外的内容，
# 然后重做上面 2、3 两处注册（详见 ../../xiaozhi-board/INSTALL.md）
```

## 构建

需要 ESP-IDF v6.0.1+（官方已不支持 5.x）：

```bash
cd xiaozhi-esp32
idf.py set-target esp32s3
idf.py menuconfig      # Xiaozhi Assistant → Target Board 选 demo3 robot
idf.py build flash monitor
```

详细步骤与验收清单：[../xiaozhi-board/INSTALL.md](../xiaozhi-board/INSTALL.md)
