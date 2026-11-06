#pragma once
#include <Arduino.h>
#include "eyes.h"
#include "motion.h"
#include "sound.h"
#include "config.h"
#if ROBOT_IS_S3
#include "voice.h"  // S3 语音交互（audio.h 由其内部引入）
#endif

// 桌面陪伴机器人状态机
// Idle：眨眼环视 + 显示时钟；被摸头 → Happy；暗光太久/没人理 → Sleep
// Happy：眯眼点头 + 欢叫（被摸头触发，可刷新时长）
// Sleep：闭眼低头 + Zzz；触摸或光线变亮唤醒
class Robot {
public:
    void begin();
    void update();

private:
    enum class State : uint8_t { Idle, Happy, Sleep };

    void _enterState(State s);
    void _onPet();
    void _syncTime();
    void _setStatusLed(bool on);

    RobotEyes   _eyes;
    RobotMotion _motion;
    RobotSound  _sound;
#if ROBOT_IS_S3
    RobotVoice  _voice;   // S3：摸头触发语音交互
#endif

    State    _state = State::Idle;
    uint32_t _stateStart = 0;

    // 触摸（软件消抖 + 开机基线校准）
    float    _touchBaseline = 0;
    bool     _touchWasPressed = false;
    uint8_t  _touchCount = 0;
    uint32_t _lastTouchPoll = 0;

    // 光线
    uint32_t _darkSince = 0;
    uint32_t _brightSince = 0;
    uint32_t _lastLdrPoll = 0;

    // 环视（眼睛像素偏移 + 舵机随动）
    int      _lookX = 0, _lookY = 0;
    float    _lookXF = 0, _lookYF = 0;
    int      _lookXTgt = 0, _lookYTgt = 0;
    uint32_t _nextGlance = 0;

    // WiFi / NTP
    bool     _wifiOk = false;
    uint32_t _lastWifiRetry = 0;
    uint32_t _lastNtpSync = 0;
    char     _timeStr[8] = "--:--";

    // 状态 LED 心跳
    bool     _ledOn = false;
    uint32_t _lastLedToggle = 0;
};
