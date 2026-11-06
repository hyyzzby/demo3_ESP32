#pragma once
#include <Arduino.h>

// 无源蜂鸣器旋律播放器（LEDC 通道，非阻塞）
// play() 后每帧调用 update() 推进音符序列
class RobotSound {
public:
    struct Note { uint16_t freq; uint16_t ms; };  // freq=0 表示休止符

    void begin();
    void update();
    void playBoot();    // 开机三连音 ↑
    void playHappy();   // 开心两声上扬
    void playYawn();    // 困倦哈欠 ↓
    void playWake();    // 苏醒
    void stop();
    bool playing() const { return _playing; }

private:
    void _play(const Note* seq, uint8_t len);

    const Note* _seq = nullptr;
    uint8_t _len = 0, _idx = 0;
    bool _playing = false;
    uint32_t _noteStart = 0;
};
