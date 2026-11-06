#pragma once
#include <ESP32Servo.h>
#include "config.h"

// 双舵机头部动作（水平 pan / 垂直 tilt）
// look() 设定目标角度，update() 中按 SERVO_SPEED_DPS 平滑趋近；
// 点头/打招呼为内置时间轴动画，同样非阻塞执行。
class RobotMotion {
public:
    void begin();
    void update(uint32_t now);
    void look(int pan, int tilt);   // 目标角度（自动限幅到 config.h 范围）
    void center();                  // 回正
    void rest();                    // 休息位（低头）
    void playNod();                 // 点头动画
    void playHello();               // 开机左右摆头
    bool busy() const { return _action != Action::None; }
    int  panDeg() const { return _panCur; }

private:
    enum class Action : uint8_t { None, Nod, Hello };
    static int _clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
    void _apply();

    Servo _pan, _tilt;
    int _panCur = SERVO_PAN_CENTER, _tiltCur = SERVO_TILT_CENTER;
    int _panTgt = SERVO_PAN_CENTER, _tiltTgt = SERVO_TILT_CENTER;
    Action _action = Action::None;
    uint32_t _actionStart = 0;
    uint32_t _lastUpdate = 0;
};
