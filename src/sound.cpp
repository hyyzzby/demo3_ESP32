#include "sound.h"
#include "config.h"

// 旋律（频率 Hz, 时长 ms）
static const RobotSound::Note BOOT_SEQ[]  = {{1047, 80}, {1319, 80}, {1568, 140}};   // C6-E6-G6
static const RobotSound::Note HAPPY_SEQ[] = {{1568, 90}, {0, 40},    {2093, 130}};   // G6-休-C7
static const RobotSound::Note YAWN_SEQ[]  = {{784, 160}, {659, 220}, {523, 320}};    // G5-E5-C5 下滑
static const RobotSound::Note WAKE_SEQ[]  = {{523, 80},  {784, 80},  {1047, 120}};   // C5-G5-C6

void RobotSound::begin() {
    ledcSetup(BUZZER_LEDC_CH, 2000, 10);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CH);
    ledcWriteTone(BUZZER_LEDC_CH, 0);
}

void RobotSound::_play(const Note* seq, uint8_t len) {
    _seq = seq;
    _len = len;
    _idx = 0;
    _playing = true;
    _noteStart = millis();
    ledcWriteTone(BUZZER_LEDC_CH, _seq[0].freq);
}

void RobotSound::update() {
    if (!_playing) return;
    uint32_t t = millis() - _noteStart;
    if (t < _seq[_idx].ms) return;
    _idx++;
    if (_idx >= _len) { stop(); return; }
    _noteStart += _seq[_idx - 1].ms;
    ledcWriteTone(BUZZER_LEDC_CH, _seq[_idx].freq);
}

void RobotSound::stop() {
    _playing = false;
    ledcWriteTone(BUZZER_LEDC_CH, 0);
}

void RobotSound::playBoot()  { _play(BOOT_SEQ,  3); }
void RobotSound::playHappy() { _play(HAPPY_SEQ, 3); }
void RobotSound::playYawn()  { _play(YAWN_SEQ,  3); }
void RobotSound::playWake()  { _play(WAKE_SEQ,  3); }
