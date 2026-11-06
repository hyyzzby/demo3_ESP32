#pragma once
#include "config.h"

#if ROBOT_IS_S3

#include <Arduino.h>
#include <driver/i2s.h>

// S3 双 I2S 音频：I2S0 收（INMP441 麦克风）/ I2S1 发（MAX98357A 功放）
// 16kHz 单声道；录音/播放/提示音全部非阻塞分段推进，每帧调用 update()
class RobotAudio {
public:
    bool begin();
    void update();

    // ---- 麦克风录音 ----
    bool recordStart(uint32_t ms = AUDIO_RECORD_MS);
    bool recording()  const { return _recActive; }
    bool recordDone() const { return _recDone; }
    const int16_t* recordData()    const { return _recBuf; }
    size_t         recordSamples() const { return _recSamples; }
    void           recordDiscard();

    // ---- 功放播放 ----
    bool playStart(const int16_t* data, size_t samples);
    bool playing() const { return _playIdx < _playLen; }
    void playStop();

    // ---- 提示音（正弦合成，蜂鸣器的替代品）----
    void tone(uint16_t freq, uint16_t ms);
    void silence();
    bool toneActive() const { return _toneActive; }

private:
    void _pollMic();
    void _feedSpeaker();

    // 录音
    int16_t* _recBuf    = nullptr;
    size_t   _recCap    = 0;
    size_t   _recPos    = 0;
    bool     _recActive = false;
    bool     _recDone   = false;

    // 播放
    const int16_t* _playData = nullptr;
    size_t         _playLen  = 0;
    size_t         _playIdx  = 0;

    // 提示音合成
    bool     _toneActive = false;
    uint16_t _toneFreq   = 0;
    uint32_t _toneEnd    = 0;
    float    _phase      = 0.0f;

    int32_t  _raw[256];   // 32bit 原始采样暂存
    int16_t  _out[256];   // 输出混音/合成缓冲
};

// 全局单例（sound.cpp 的 S3 后端也走它）
RobotAudio& Audio();

#endif // ROBOT_IS_S3
