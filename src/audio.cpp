#include "audio.h"

#if ROBOT_IS_S3

#include "config.h"
#include <math.h>

static constexpr i2s_port_t MIC_PORT = I2S_NUM_0;  // INMP441 录音
static constexpr i2s_port_t SPK_PORT = I2S_NUM_1;  // MAX98357A 播放
static constexpr size_t CHUNK = 256;               // 每帧推进的采样数 (16ms @16k)

RobotAudio& Audio() {
    static RobotAudio inst;
    return inst;
}

bool RobotAudio::begin() {
    // ---- 麦克风：32bit 标准I2S，仅左声道（INMP441 L/R 接地）----
    i2s_config_t mic = {};
    mic.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    mic.sample_rate = AUDIO_SAMPLE_RATE;
    mic.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    mic.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    mic.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    mic.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    mic.dma_buf_count = 8;
    mic.dma_buf_len = 256;
    if (i2s_driver_install(MIC_PORT, &mic, 0, nullptr) != ESP_OK) return false;
    i2s_pin_config_t mp = {};
    mp.mck_io_num   = I2S_PIN_NO_CHANGE;
    mp.bck_io_num   = PIN_I2S_MIC_SCK;
    mp.ws_io_num    = PIN_I2S_MIC_WS;
    mp.data_out_num = I2S_PIN_NO_CHANGE;
    mp.data_in_num  = PIN_I2S_MIC_SD;
    if (i2s_set_pin(MIC_PORT, &mp) != ESP_OK) return false;

    // ---- 功放：16bit 标准I2S（SD 接高电平 → (L+R)/2 混音）----
    i2s_config_t spk = {};
    spk.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    spk.sample_rate = AUDIO_SAMPLE_RATE;
    spk.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    spk.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    spk.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    spk.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    spk.dma_buf_count = 8;
    spk.dma_buf_len = 256;
    if (i2s_driver_install(SPK_PORT, &spk, 0, nullptr) != ESP_OK) return false;
    i2s_pin_config_t sp = {};
    sp.mck_io_num   = I2S_PIN_NO_CHANGE;
    sp.bck_io_num   = PIN_I2S_SPK_BCLK;
    sp.ws_io_num    = PIN_I2S_SPK_LRC;
    sp.data_out_num = PIN_I2S_SPK_DIN;
    sp.data_in_num  = I2S_PIN_NO_CHANGE;
    if (i2s_set_pin(SPK_PORT, &sp) != ESP_OK) return false;
    i2s_zero_dma_buffer(SPK_PORT);

    Serial.println("[Audio] I2S mic+spk ready");
    return true;
}

// ---------- 录音 ----------
bool RobotAudio::recordStart(uint32_t ms) {
    if (_recActive) return false;
    _recCap = (size_t)AUDIO_SAMPLE_RATE * ms / 1000;
    _recBuf = (int16_t*)realloc(_recBuf, _recCap * sizeof(int16_t));
    if (!_recBuf) return false;
    _recSamples = 0;
    _recPos = 0;
    _recDone = false;
    _recActive = true;
    return true;
}

void RobotAudio::recordDiscard() {
    _recActive = false;
    _recDone = false;
    _recSamples = 0;
}

void RobotAudio::_pollMic() {
    while (_recPos < _recCap) {
        size_t want = CHUNK;
        if (_recCap - _recPos < want) want = _recCap - _recPos;
        size_t bytes = 0;
        i2s_read(MIC_PORT, _raw, want * sizeof(int32_t), &bytes, 0);  // timeout=0 非阻塞
        size_t n = bytes / sizeof(int32_t);
        for (size_t i = 0; i < n; i++) {
            int32_t s = _raw[i] >> AUDIO_MIC_GAIN_SHIFT;  // 32bit→16bit
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            _recBuf[_recPos++] = (int16_t)s;
        }
        if (n == 0) break;  // DMA 暂无新数据，下帧再续
        if (_recPos >= _recCap) {
            _recSamples = _recPos;
            _recActive = false;
            _recDone = true;
        }
    }
}

// ---------- 播放 ----------
bool RobotAudio::playStart(const int16_t* data, size_t samples) {
    if (_recActive || _playIdx < _playLen) return false;
    _playData = data;
    _playLen = samples;
    _playIdx = 0;
    return true;
}

void RobotAudio::playStop() {
    _playData = nullptr;
    _playLen = 0;
    _playIdx = 0;
}

// ---------- 提示音 ----------
void RobotAudio::tone(uint16_t freq, uint16_t ms) {
    _toneFreq = freq;
    _toneEnd = millis() + ms;
    _toneActive = true;
    if (freq == 0) _toneActive = false;
}

void RobotAudio::silence() {
    _toneActive = false;
    _toneFreq = 0;
}

// 每帧向功放 DMA 填充 CHUNK 个样本：优先播放队列，其次提示音合成，否则静音
void RobotAudio::_feedSpeaker() {
    memset(_out, 0, sizeof(_out));
    size_t filled = 0;

    if (_playIdx < _playLen && _playData) {
        size_t n = _playLen - _playIdx;
        if (n > CHUNK) n = CHUNK;
        for (size_t i = 0; i < n; i++) {
            float v = _playData[_playIdx + i] * AUDIO_SPK_VOLUME;
            _out[i] = (int16_t)constrain(v, -32768.0f, 32767.0f);
        }
        _playIdx += n;
        filled = n;
    } else if (_toneActive) {
        if ((int32_t)(millis() - _toneEnd) >= 0) {
            _toneActive = false;
        } else {
            float step = 2.0f * (float)M_PI * _toneFreq / AUDIO_SAMPLE_RATE;
            for (size_t i = 0; i < CHUNK; i++) {
                _out[i] = (int16_t)(sinf(_phase) * 9000.0f * AUDIO_SPK_VOLUME);
                _phase += step;
            }
            filled = CHUNK;
        }
    }

    if (filled > 0) {
        size_t bytes = 0;
        i2s_write(SPK_PORT, _out, filled * sizeof(int16_t), &bytes, 0);  // timeout=0 非阻塞
        if (bytes < filled * sizeof(int16_t)) {
            // DMA 满了，播放队列回退未写入部分，提示音相位回退半帧避免爆音
            size_t written = bytes / sizeof(int16_t);
            if (_playIdx < _playLen && _playData) _playIdx -= (filled - written);
            else _phase -= (float)(filled - written) * 2.0f * (float)M_PI * _toneFreq / AUDIO_SAMPLE_RATE;
        }
    }
}

void RobotAudio::update() {
    if (_recActive) _pollMic();
    _feedSpeaker();
}

#endif // ROBOT_IS_S3
