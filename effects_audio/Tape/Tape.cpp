//
// Created by ferluht on 10/07/2022.
//

#include "Tape.h"

MIDISTATUS Tape::midiIn(MData &cmd) {
    MIDISTATUS ret = MIDISTATUS::DONE;
    if (cmd.status == CC_HEADER &&
        cmd.data1 > MIDICC::TAPE &&
        cmd.data1 < MIDICC::TAPE_END &&
        cmd.data2 > 0) {

        switch (cmd.data1) {
            case MIDICC::TAPE_TRIG:
                ret = trig();
                break;
            case MIDICC::TAPE_CLEAR:
                ret = clear();
                break;
            case MIDICC::TAPE_DOUBLE:
                ret = double_loop();
                break;
            case MIDICC::TAPE_STOP:
                looper_state = STOP;
                position = 0;
                ret = MIDISTATUS::DONE;
                break;
            default:
                break;
        }
    }
    return ret;
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
