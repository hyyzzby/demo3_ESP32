#include "robot.h"
#include "config.h"
#include <WiFi.h>
#include <time.h>

void Robot::begin() {
    Serial.begin(115200);
    Serial.println("\n[Robot] booting...");
    pinMode(PIN_STATUS_LED, OUTPUT);
    _setStatusLed(true);

    _sound.begin();
    _motion.begin();
    _eyes.begin();

    // ---- 触摸基线校准（校准时不要碰 GPIO4）----
    _eyes.bootScreen("Calibrating", "keep hands off");
    uint32_t acc = 0;
    for (int i = 0; i < 32; i++) { acc += touchRead(PIN_TOUCH); delay(2); }
    _touchBaseline = acc / 32.0f;
    Serial.printf("[Robot] touch baseline = %.0f\n", _touchBaseline);

    // ---- WiFi（阻塞最多 10s，失败进离线模式后台重试）----
    _eyes.bootScreen("Connecting WiFi", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) delay(100);
    _wifiOk = (WiFi.status() == WL_CONNECTED);
    if (_wifiOk) {
        Serial.printf("[Robot] WiFi OK  IP=%s\n", WiFi.localIP().toString().c_str());
        setenv("TZ", TZ_INFO, 1);
        tzset();
        configTime(8 * 3600, 0, NTP_SERVER_1, NTP_SERVER_2);
        _syncTime();
    } else {
        Serial.println("[Robot] WiFi failed, offline mode (will retry)");
    }

    // ---- 开机表演：欢叫 + 摆头 ----
    _eyes.bootScreen("I'm alive!", "");
    _sound.playBoot();
    _motion.playHello();
    while (_motion.busy()) { _motion.update(millis()); delay(5); }

#if ROBOT_IS_S3
    // ---- 语音链路自测（非阻塞，在待机中完成）：滴→录3s→嘟→回放 ----
    _voice.begin(&Audio());
    _eyes.bootScreen("Voice self-test", "say anything...");
    _voice.start();
#endif

    _enterState(State::Idle);
    Serial.println("[Robot] ready.");
}

void Robot::_enterState(State s) {
    _state = s;
    _stateStart = millis();
    switch (s) {
        case State::Idle:
            Serial.println("[Robot] -> Idle");
            _motion.center();
            break;
        case State::Happy:
            Serial.println("[Robot] -> Happy");
            _sound.playHappy();
            _motion.playNod();
            break;
        case State::Sleep:
            Serial.println("[Robot] -> Sleep");
            _sound.playYawn();
            _motion.rest();
#if ROBOT_IS_S3
            _voice.cancel();  // 语音交互随睡眠中止
#endif
            break;
    }
}

// 被摸头：睡着先醒再开心；醒着直接开心（可连续摸连续爽）
// S3 语音版：开心之外同时触发一次语音交互
void Robot::_onPet() {
    Serial.println("[Robot] pet!");
    if (_state == State::Sleep) {
        _sound.playWake();
        _enterState(State::Idle);
        return;
    }
    _enterState(State::Happy);
#if ROBOT_IS_S3
    if (!_voice.busy()) _voice.start();
#endif
}

void Robot::_syncTime() {
    _lastNtpSync = millis();
    struct tm ti;
    if (getLocalTime(&ti, 3000)) {
        snprintf(_timeStr, sizeof(_timeStr), "%02d:%02d", ti.tm_hour, ti.tm_min);
        Serial.printf("[Robot] time synced %s\n", _timeStr);
    }
}

void Robot::_setStatusLed(bool on) {
    digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW);
    _ledOn = on;
}

void Robot::update() {
    uint32_t now = millis();
    _motion.update(now);
    _sound.update();
#if ROBOT_IS_S3
    _voice.update(now);
#endif

    // ---- 触摸轮询（20ms，带 ±滞回消抖）----
    if (now - _lastTouchPoll >= 20) {
        _lastTouchPoll = now;
        float v = touchRead(PIN_TOUCH);
        bool pressed = v < _touchBaseline * TOUCH_RATIO;
        if (pressed) _touchCount = _touchCount < 5 ? _touchCount + 1 : 5;
        else         _touchCount = _touchCount > 0 ? _touchCount - 1 : 0;
        bool stable = _touchCount >= 3;
        if (stable && !_touchWasPressed) _onPet();
        _touchWasPressed = stable;
    }

    // ---- 光线轮询（1s）：暗太久睡觉 / 亮起来自动醒 ----
    if (now - _lastLdrPoll >= 1000) {
        _lastLdrPoll = now;
        bool dark = analogRead(PIN_LDR) < LDR_DARK_VALUE;
        if (dark)  { if (!_darkSince)   _darkSince = now;   _brightSince = 0; }
        else       { if (!_brightSince) _brightSince = now; _darkSince = 0; }

        if (_state == State::Idle) {
            if (_darkSince && now - _darkSince >= SLEEP_AFTER_DARK_MS) {
                Serial.println("[Robot] too dark, sleeping");
                _enterState(State::Sleep);
            } else if (now - _stateStart >= SLEEP_AFTER_IDLE_MS) {
                Serial.println("[Robot] nobody plays with me, sleeping");
                _enterState(State::Sleep);
            }
        } else if (_state == State::Sleep && _brightSince &&
                   now - _brightSince >= WAKE_AFTER_LIGHT_MS) {
            Serial.println("[Robot] light is back, waking");
            _sound.playWake();
            _enterState(State::Idle);
        }
    }

    // ---- WiFi 保活 + 定时校时 ----
    bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected != _wifiOk) {
        _wifiOk = connected;
        Serial.printf("[Robot] WiFi %s\n", connected ? "connected" : "lost");
        if (connected) _syncTime();
    }
    if (!connected && now - _lastWifiRetry >= 30000) {
        _lastWifiRetry = now;
        WiFi.reconnect();
    }
    if (_wifiOk && now - _lastNtpSync >= 1800000UL) _syncTime();

    // ---- 环视（仅待机、动作不忙时）----
    if (_state == State::Idle && !_motion.busy() && now >= _nextGlance) {
        _nextGlance = now + 3500 + random(4000);
        _lookXTgt = random(-10, 11);   // 眼睛像素偏移
        _lookYTgt = random(-5, 6);
        _motion.look(SERVO_PAN_CENTER + _lookXTgt * 3,    // 头随眼动
                     SERVO_TILT_CENTER + _lookYTgt * 2);
    }
    _lookXF += (_lookXTgt - _lookXF) * 0.12f;   // 视线缓动
    _lookYF += (_lookYTgt - _lookYF) * 0.12f;
    _lookX = (int)_lookXF;
    _lookY = (int)_lookYF;

    // ---- 状态超时 ----
    if (_state == State::Happy && now - _stateStart >= PET_DURATION_MS) {
        _enterState(State::Idle);
    }

    // ---- 渲染 ----
    Expression expr = Expression::Neutral;
    if (_state == State::Happy)      expr = Expression::Happy;
    else if (_state == State::Sleep) expr = Expression::Sleepy;
#if ROBOT_IS_S3
    else if (_voice.phase() == RobotVoice::Recording) expr = Expression::Surprised;  // 录音中瞪大眼
#endif
    _eyes.render(expr, _lookX, _lookY, _wifiOk, _timeStr);

    // ---- 状态 LED：待机慢闪 / 开心常亮 / 睡觉熄灭 ----
    switch (_state) {
        case State::Idle:
            if (now - _lastLedToggle >= 1500) {
                _lastLedToggle = now;
                _setStatusLed(!_ledOn);
            }
            break;
        case State::Happy: _setStatusLed(true);  break;
        case State::Sleep: _setStatusLed(false); break;
    }
}
