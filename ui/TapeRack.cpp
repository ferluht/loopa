//
// Created by ferluht on 22/07/2022.
//

#include "TapeRack.h"

void TapeRack::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    float buf[4][BUF_SIZE * 2];
    float emptybuf[BUF_SIZE * 2];

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        emptybuf[i + 0] = 0;
        emptybuf[i + 1] = 0;
    }

    for (int i = 0; i < 4; i ++) {
        if (i == focus_tape)
            tapes[i]->process(buf[i], inputBuffer, nBufferFrames, 0);
        else
            tapes[i]->process(buf[i], emptybuf, nBufferFrames, 0);
    }

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        outputBuffer[i + 0] = buf[0][i+0] + buf[1][i+0] + buf[2][i+0] + buf[3][i+0];
        outputBuffer[i + 1] = buf[0][i+1] + buf[1][i+1] + buf[2][i+1] + buf[3][i+1];
    }
}

void TapeRack::draw(GFXcanvas1 * screen) {

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


    for (int i = 0; i < 4; i ++) {
        screen->drawRect(111, 11 + 5 * i, 15, 4, 1);
        float width = tapes[i]->getPosition() * 15;
        if (width > 1)
            screen->drawRect(111, 11 + 5 * i + 1, width, 2, 1);

        switch (tapes[i]->getState()) {
            case (Tape::TAPE_STATE::REC):
            case (Tape::TAPE_STATE::OVERDUB):
                screen->drawCircle(103, 11 + 5 * i + 1, 1, 1);
                break;
            case (Tape::TAPE_STATE::STOP):
                screen->drawRect(98, 11 + 5 * i, 3, 3,1);
                break;
            case (Tape::TAPE_STATE::PLAY):
                screen->drawTriangle(106, 11 + 5 * i, 106, 11 + 5 * i + 2, 109, 11 + 5 * i + 1,1);
                break;
            default:
                break;
        }
    }
}