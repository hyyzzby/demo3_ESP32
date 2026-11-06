#include "motion.h"
#include "config.h"
#include <Arduino.h>

// 动作时间轴：{累计ms → 该时刻应到达的角度}，段间线性插值
struct Seg { uint32_t ms; int deg; };
static const Seg NOD_SEGS[]   = {{100, 75}, {250, 100}, {380, 80}, {500, SERVO_TILT_CENTER}};
static const Seg HELLO_SEGS[] = {{300, 55}, {650, 125}, {1000, SERVO_PAN_CENTER}};
static constexpr size_t NOD_N = 4, HELLO_N = 3;

void RobotMotion::begin() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    _pan.setPeriodHertz(50);
    _tilt.setPeriodHertz(50);
    _pan.attach(PIN_SERVO_PAN, 500, 2400);
    _tilt.attach(PIN_SERVO_TILT, 500, 2400);
    _apply();
    _lastUpdate = millis();
}

void RobotMotion::look(int pan, int tilt) {
    _panTgt  = _clamp(pan,  SERVO_PAN_MIN,  SERVO_PAN_MAX);
    _tiltTgt = _clamp(tilt, SERVO_TILT_MIN, SERVO_TILT_MAX);
}

void RobotMotion::center() { look(SERVO_PAN_CENTER, SERVO_TILT_CENTER); }
void RobotMotion::rest()   { look(SERVO_PAN_CENTER, SERVO_TILT_MAX); }  // 大角度 = 低头

void RobotMotion::playNod()   { _action = Action::Nod;   _actionStart = millis(); }
void RobotMotion::playHello() { _action = Action::Hello; _actionStart = millis(); }

void RobotMotion::update(uint32_t now) {
    if (_action != Action::None) {
        const Seg* segs = (_action == Action::Nod) ? NOD_SEGS : HELLO_SEGS;
        const size_t n = (_action == Action::Nod) ? NOD_N : HELLO_N;
        uint32_t t = now - _actionStart;
        int prevDeg = (_action == Action::Nod) ? SERVO_TILT_CENTER : SERVO_PAN_CENTER;
        uint32_t prevT = 0;
        int deg = prevDeg;
        bool done = true;
        for (size_t i = 0; i < n; i++) {
            if (t < segs[i].ms) {
                float k = (float)(t - prevT) / (float)(segs[i].ms - prevT);
                deg = prevDeg + (int)((segs[i].deg - prevDeg) * k);
                done = false;
                break;
            }
            prevDeg = segs[i].deg;
            prevT = segs[i].ms;
            deg = segs[i].deg;
        }
        if (_action == Action::Nod) _tiltCur = deg;
        else                        _panCur = deg;
        if (done) {
            if (_action == Action::Nod) _tiltTgt = SERVO_TILT_CENTER;
            else                        _panTgt = SERVO_PAN_CENTER;
            _action = Action::None;
        }
    } else {
        // 平滑趋近目标角度（限速）
        float dt = (now - _lastUpdate) / 1000.0f;
        int step = (int)(SERVO_SPEED_DPS * dt);
        int dPan = _panTgt - _panCur, dTilt = _tiltTgt - _tiltCur;
        if (dPan)  _panCur  += (abs(dPan)  < step) ? dPan  : (dPan  > 0 ? step : -step);
        if (dTilt) _tiltCur += (abs(dTilt) < step) ? dTilt : (dTilt > 0 ? step : -step);
    }
    _apply();
    _lastUpdate = now;
}

void RobotMotion::_apply() {
    _pan.write(_panCur);
    _tilt.write(_tiltCur);
}
