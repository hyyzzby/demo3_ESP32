#include "voice.h"

#if ROBOT_IS_S3

#include "config.h"

void RobotVoice::begin(RobotAudio* audio) {
    _audio = audio;
    _phase = Idle;
}

void RobotVoice::_goto(Phase p, uint32_t now) {
    _phase = p;
    _phaseStart = now;
}

void RobotVoice::start() {
    if (busy() || !_audio) return;
    _audio->silence();
    _goto(CueIn, millis());
}

void RobotVoice::cancel() {
    if (!_audio) return;
    if (_audio->recording()) _audio->recordDiscard();
    if (_audio->playing())   _audio->playStop();
    _audio->silence();
    _phase = Idle;
}

void RobotVoice::update(uint32_t now) {
    if (!_audio || _phase == Idle) return;
    uint32_t dt = now - _phaseStart;

    switch (_phase) {
        case CueIn:  // "滴"提示 + 起录
            if (dt == 0) _audio->tone(1047, 150);
            if (dt >= 200) {
                if (_audio->recordStart(AUDIO_RECORD_MS)) _goto(Recording, now);
                else _phase = Idle;
            }
            break;

        case Recording:  // 录音中……
            if (_audio->recordDone()) {
                _goto(CueOut, now);
            }
            break;

        case CueOut:  // "嘟"提示 + 播放回声
            if (dt == 0) _audio->tone(1319, 150);
            if (dt >= 200) {
                _audio->playStart(_audio->recordData(), _audio->recordSamples());
                _goto(Playing, now);
            }
            break;

        case Playing:  // 播放中……
            if (!_audio->playing() && dt >= 100) {
                _audio->recordDiscard();
                _phase = Idle;
            }
            break;

        default:
            _phase = Idle;
            break;
    }
}

#endif // ROBOT_IS_S3
