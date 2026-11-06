#include "eyes.h"
#include "config.h"
#include <Arduino.h>

// ---- 屏幕几何（128x64）----
static constexpr int EYE_L_CX = 44, EYE_R_CX = 84;  // 左右眼中心
static constexpr int EYE_CY   = 28;                 // 眼睛垂直中心
static constexpr int EYE_W    = 22, EYE_H = 28;     // 眼睛宽高

void RobotEyes::begin() {
    _u8g2.begin();
    _u8g2.setBusClock(400000);
    _nextBlinkAt = millis() + 2000;
    bootScreen("Robot Booting...", "");
}

void RobotEyes::bootScreen(const char* line1, const char* line2) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_9x15_tr);
    _u8g2.drawStr(10, 28, line1);
    if (line2 && line2[0]) _u8g2.drawStr(10, 44, line2);
    _u8g2.sendBuffer();
}

void RobotEyes::render(Expression expr, int lookX, int lookY, bool wifiOk, const char* timeStr) {
    _u8g2.clearBuffer();
    switch (expr) {
        case Expression::Neutral:   _drawNeutral(millis(), lookX, lookY); break;
        case Expression::Happy:     _drawHappy();            break;
        case Expression::Sleepy:    _drawSleepy(millis());   break;
        case Expression::Surprised: _drawSurprised();        break;
    }
    _drawStatusBar(wifiOk, timeStr);
    _u8g2.sendBuffer();
}

// 圆角矩形眼睛 + 高光；眨眼时 h 变小、眼皮从上往下合
void RobotEyes::_drawEye(int cx, int cy, int w, int h) {
    int r = (w / 2 < 7) ? w / 2 : 7;
    if (h < 2 * r) r = h / 2;
    if (h < 2) h = 2;
    _u8g2.drawRBox(cx - w / 2, cy - h / 2, w, h, r);
    if (h >= 14) {  // 睁眼时才画高光
        _u8g2.setDrawColor(0);
        _u8g2.drawBox(cx - w / 2 + 3, cy - h / 2 + 3, 5, 4);
        _u8g2.setDrawColor(1);
    }
}

void RobotEyes::_drawNeutral(uint32_t now, int lookX, int lookY) {
    // 眨眼状态机：随机间隔触发，180ms 三角波（闭合→睁开）
    int h = EYE_H;
    if (!_blinkActive && (int32_t)(now - _nextBlinkAt) >= 0) {
        _blinkActive = true;
        _blinkStart = now;
    }
    if (_blinkActive) {
        uint32_t t = now - _blinkStart;
        if (t >= 180) {
            _blinkActive = false;
            _nextBlinkAt = now + BLINK_PERIOD_MIN_MS + random(BLINK_PERIOD_MAX_MS - BLINK_PERIOD_MIN_MS);
        } else {
            float p = t / 180.0f;
            float close = (p < 0.5f) ? p * 2.0f : (1.0f - p) * 2.0f;  // 0→1→0
            h = (int)(EYE_H * (1.0f - close * 0.92f));
        }
    }
    int cy = EYE_CY + (EYE_H - h) / 2;  // 中心随高度下移 → 眼皮上合效果
    _drawEye(EYE_L_CX + lookX, cy + lookY, EYE_W, h);
    _drawEye(EYE_R_CX + lookX, cy + lookY, EYE_W, h);
}

// 开心 ∩∩：上半圆 + 抹掉下半 + 腮红
void RobotEyes::_drawHappy() {
    for (int cx : {EYE_L_CX, EYE_R_CX}) {
        _u8g2.drawDisc(cx, EYE_CY + 6, 11);
        _u8g2.setDrawColor(0);
        _u8g2.drawBox(cx - 12, EYE_CY + 6, 24, 14);
        _u8g2.setDrawColor(1);
    }
    _u8g2.drawBox(EYE_L_CX - 16, EYE_CY + 12, 8, 3);
    _u8g2.drawBox(EYE_R_CX + 8,  EYE_CY + 12, 8, 3);
}

// 睡觉：闭眼横线 + 飘动 Zzz
void RobotEyes::_drawSleepy(uint32_t now) {
    _u8g2.drawBox(EYE_L_CX - 11, EYE_CY + 4, 22, 3);
    _u8g2.drawBox(EYE_R_CX - 11, EYE_CY + 4, 22, 3);
    int bob = (int)((now / 600) % 2);  // 0/1 交替上下飘
    _u8g2.setFont(u8g2_font_9x15_tr);
    _u8g2.drawStr(102, 18 - bob * 2, "Z");
    _u8g2.setFont(u8g2_font_6x12_tr);
    _u8g2.drawStr(116, 28 + bob * 2, "z");
}

// 惊讶：大圆眼 + O 型嘴
void RobotEyes::_drawSurprised() {
    _drawEye(EYE_L_CX, EYE_CY, 26, 30);
    _drawEye(EYE_R_CX, EYE_CY, 26, 30);
    _u8g2.drawDisc(64, 50, 5);
}

// 底部状态栏：左 WiFi 图标（圆心贴近屏底 → 只露上半弧），右侧时间
void RobotEyes::_drawStatusBar(bool wifiOk, const char* timeStr) {
    if (wifiOk) {
        _u8g2.drawDisc(8, 62, 2);
        _u8g2.drawCircle(8, 62, 5);
        _u8g2.drawCircle(8, 62, 9);
    } else {
        _u8g2.setFont(u8g2_font_6x12_tr);
        _u8g2.drawStr(4, 63, "x");
    }
    _u8g2.setFont(u8g2_font_6x12_tr);
    _u8g2.drawStr(128 - 6 * (int)strlen(timeStr) - 2, 63, timeStr);
}
