//
// Created by ferluht on 10/07/2022.
//

#include "Tape.h"

Tape::Tape() : AudioEffect("TAPE") {
    audio.reserve(max_loop_size);

    addMIDIHandler(CC_HEADER, MIDICC::TAPE_TRIG, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return trig();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(CC_HEADER, MIDICC::TAPE_CLEAR, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return clear();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(CC_HEADER, MIDICC::TAPE_DOUBLE, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return double_loop();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(CC_HEADER, MIDICC::TAPE_STOP, [this](MData &cmd) -> MIDISTATUS {
        looper_state = STOP;
        position = 0;
        return MIDISTATUS::DONE;
    });

    clear();
}

MIDISTATUS Tape::trig() {
    switch (looper_state) {
        case STOP:
            if (audio.size() > 0) looper_state = PLAY;
            else looper_state = REC;
            break;
        case REC:
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

void Tape::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {

    float envl, envr;

    for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
        switch (looper_state) {
            case STOP:
                outputBuffer[i + 0] = inputBuffer[i + 0];
                outputBuffer[i + 1] = inputBuffer[i + 1];
                break;
            case REC:
                audio.push_back(inputBuffer[i + 0]);
                audio.push_back(inputBuffer[i + 1]);
                outputBuffer[i + 0] = inputBuffer[i + 0];
                outputBuffer[i + 1] = inputBuffer[i + 1];
                break;
            case OVERDUB:
                audio[position+0] = soft_clip(audio[position+0] + inputBuffer[i + 0]);
                audio[position+1] = soft_clip(audio[position+1] + inputBuffer[i + 1]);
                outputBuffer[i + 0] = audio[position+0];
                outputBuffer[i + 1] = audio[position+1];
                position = (position + 2) % audio.size();
                break;
            case PLAY:
                if (audio.empty()) break;
                outputBuffer[i + 0] = soft_clip(audio[position+0] + inputBuffer[i + 0]);
                outputBuffer[i + 1] = soft_clip(audio[position+1] + inputBuffer[i + 1]);
                position = (position + 2) % audio.size();
                break;
            default:
                break;
        }
    }
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
