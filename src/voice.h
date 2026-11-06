#pragma once
#include "config.h"

#if ROBOT_IS_S3

#include <Arduino.h>
#include "audio.h"

// 语音交互状态机（v0.5 回声自测版）：
//   start() → 提示音"滴" → 录音 N 秒 → 提示音"嘟" → 原样播放（验证麦克风+功放链路）
//   将来接入小智AI：Recording 结束后把 recordData() 送云端 ASR，
//   收到 TTS 音频流后 playStart() 播放 —— 状态机骨架不变，见 docs/xiaozhi-migration.md
class RobotVoice {
public:
    enum Phase : uint8_t { Idle, CueIn, Recording, CueOut, Playing };

    void begin(RobotAudio* audio);
    void update(uint32_t now);
    void start();      // 触发一次语音交互（摸头时调用）
    void cancel();     // 睡觉/打断时中止
    bool busy() const  { return _phase != Idle; }
    Phase phase() const { return _phase; }

private:
    void _goto(Phase p, uint32_t now);

    RobotAudio* _audio = nullptr;
    Phase       _phase = Idle;
    uint32_t    _phaseStart = 0;
};

#endif // ROBOT_IS_S3
