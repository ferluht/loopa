//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include <AMG.h>
#include <AudioEffects.h>

class TapeRack : public AMG {

public:

    TapeRack() : AMG("TAPE") {
        for (int i = 0; i < 4; i ++) tapes.push_back(new Tape());
        focus_tape = 0;
        phase = 0;
    }

    void midiIn(MData& cmd) override;

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(GFXcanvas1 * screen) override;

private:

    std::vector<Tape *> tapes;
    uint8_t focus_tape;
    float phase;

    bool cmdpressed = false;
    bool shiftpressed = false;

};
