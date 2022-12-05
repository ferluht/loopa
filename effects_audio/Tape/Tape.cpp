//
// Created by ferluht on 10/07/2022.
//

#include "Tape.h"

Tape::Tape() : Tape(nullptr){}

Tape::Tape(Sync * sync) : AudioEffect("TAPE") {
    if (sync) this->sync = sync;
    else this->sync = new Sync();

    audio.reserve(max_loop_size);

    sync->attachSyncCallback(this, [this](){ trig(); });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::TRIG, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return trig();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::CLEAR, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return clear();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::DOUBLE, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return double_loop();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::STOP, [this](MData &cmd) -> MIDISTATUS {
//        looper_state = STOP;
        if (cmd.data2 > 0)
            level = std::abs(level - 1.0);
//        position = 0;
        return MIDISTATUS::DONE;
    });

    clear();
}

MIDISTATUS Tape::trig() {
    switch (looper_state) {
        case STOP:
            if (sync->wait(this)) break;
            if (audio.size() > 0) looper_state = PLAY;
            else looper_state = REC;
            break;
        case REC:
            if (sync->wait(this)) break;
            looper_state = OVERDUB;
            break;
        case OVERDUB:
            looper_state = PLAY;
            break;
        case PLAY:
            looper_state = OVERDUB;
            break;
        default:
            break;
    }
    return MIDISTATUS::DONE;
}

MIDISTATUS Tape::clear() {
    looper_state = STOP;
    audio.clear();
    sync->inactivate(this);
    position = 0;
    avg = 0;
    avg_env = 0;
    return MIDISTATUS::DONE;
}

MIDISTATUS Tape::double_loop() {
    if (doubling_progress == 0)
        doubling_size = audio.size();
    if (doubling_size * 2 > max_loop_size) {
        return MIDISTATUS::DONE;
    }
    auto beg = audio.begin() + doubling_progress;
    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
    auto end = beg + step_size;
    std::copy(beg, end, std::back_inserter(audio));
    doubling_progress += step_size;
    if (doubling_progress == doubling_size) {
        doubling_progress = 0;
        return MIDISTATUS::DONE;
    }
    return MIDISTATUS::WAITING;
}

MIDISTATUS Tape::copy(Tape * to) {
    if (doubling_progress == 0)
        doubling_size = audio.size();
    auto beg = audio.begin() + doubling_progress;
    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
    auto end = beg + step_size;
    std::copy(beg, end, std::back_inserter(to->audio));
    doubling_progress += step_size;
    if (doubling_progress == doubling_size) {
        doubling_progress = 0;
        return MIDISTATUS::DONE;
    }
    return MIDISTATUS::WAITING;
}

void Tape::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    unsigned long old_position = position;
    switch (looper_state) {
        case STOP:
            std::copy(inputBuffer, inputBuffer + nBufferFrames * 2, outputBuffer);
            break;
        case REC:
            std::copy(inputBuffer, inputBuffer + nBufferFrames * 2, std::back_inserter(audio));
            std::copy(inputBuffer, inputBuffer + nBufferFrames * 2, outputBuffer);
            break;
        case OVERDUB:
            for (unsigned int i = 0; i < 2*nBufferFrames; i ++) {
                audio[position] = soft_clip(audio[position] + inputBuffer[i]);
                outputBuffer[i] = audio[position] * level;
                position = (position + 1) % audio.size();
            }
            break;
        case PLAY:
            if (audio.empty()) break;
            for (unsigned int i = 0; i < 2*nBufferFrames; i ++) {
                outputBuffer[i] = soft_clip(audio[position] + inputBuffer[i]) * level;
                position = (position + 1) % audio.size();
            }
            break;
    }
    if (position < old_position) sync->send(this);
}

void Tape::draw(GFXcanvas1 * screen) {
//    nvgCircle(vg, 50, 16, 10);
//    nvgCircle(vg, 78, 16, 10);
//
//    nvgCircle(vg, 50, 16, 6);
//    nvgCircle(vg, 78, 16, 6);
//
//    nvgCircle(vg, 50, 16, 2.5);
//    nvgCircle(vg, 78, 16, 2.5);
//
//    nvgMoveTo(vg, 50, 26);
//    nvgLineTo(vg, 78, 26);
//
//    for (int i = 0; i < 5; i++) {
//        nvgMoveTo(vg, 50 + 3 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 3 * std::sin(phase + M_PI * 2 / 5 * i));
//        nvgLineTo(vg, 50 + 10 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 10 * std::sin(phase + M_PI * 2 / 5 * i));
//
//        nvgMoveTo(vg, 78 + 3 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 3 * std::sin(phase + M_PI * 2 / 5 * i));
//        nvgLineTo(vg, 78 + 10 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 10 * std::sin(phase + M_PI * 2 / 5 * i));
//    }
//
//    nvgRoundedRect(vg, 38, 4, 51, 24, 4);
//
//    nvgCircle(vg, 64, 22, 2);
//
//    nvgStrokeWidth(vg, 1);
//    nvgStrokeColor(vg, nvgRGB(255, 255, 255));
//    nvgStroke(vg);
//
//    phase += 0.1;
//    if (phase > M_PI * 2) phase = 0;
}

float Tape::getPosition() {
    return (float)position / (float)audio.size();
}

int Tape::getState() {
    return looper_state;
}
