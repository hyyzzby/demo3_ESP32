#include <Arduino.h>
#include "robot.h"

// ESP32 桌面陪伴机器人 — 入口
// 硬件接线见 docs/hardware.md，所有可调参数集中在 config.h
Robot robot;

void setup() {
    robot.begin();
}

void loop() {
    robot.update();
}
