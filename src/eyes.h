#pragma once
#include <U8g2lib.h>

// 机器人表情
enum class Expression : uint8_t {
    Neutral,    // 平静（会眨眼、随视线偏移）
    Happy,      // 开心眯眼 + 腮红
    Sleepy,     // 睡觉 Zzz
    Surprised,  // 惊讶圆眼
};

// OLED 眼睛渲染器（128x64 SSD1306, I2C 硬件接口）
class RobotEyes {
public:
    void begin();
    // 每帧调用：expr 表情 / lookX lookY 视线像素偏移 / 底部状态栏
    void render(Expression expr, int lookX, int lookY, bool wifiOk, const char* timeStr);
    // 开机文字屏
    void bootScreen(const char* line1, const char* line2);
    void setContrast(uint8_t v) { _u8g2.setContrast(v); }

private:
    void _drawNeutral(uint32_t now, int lookX, int lookY);
    void _drawHappy();
    void _drawSleepy(uint32_t now);
    void _drawSurprised();
    void _drawEye(int cx, int cy, int w, int h);
    void _drawStatusBar(bool wifiOk, const char* timeStr);

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{U8G2_R0, U8X8_PIN_NONE};
    bool     _blinkActive = false;
    uint32_t _blinkStart = 0;
    uint32_t _nextBlinkAt = 0;
};
