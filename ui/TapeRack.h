//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include <AMG.h>
#include <AudioEffects.h>

class TapeRack : public AMG {

public:

    TapeRack() : AMG("TAPES") {
        for (int i = 0; i < 4; i ++) tapes.push_back(new Tape());
        focus_tape = 0;
        phase = 0;

        addMIDIHandler(CC_HEADER, CC_HEADER + 16,
                       MIDICC::TAPE, MIDICC::TAPE_STOP,
                       [this](MData &cmd) -> MIDISTATUS {
                           focus_tape = cmd.status - CC_HEADER;
                           cmd.status = CC_HEADER;
                           return tapes[focus_tape]->midiIn(cmd);
                       });
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(GFXcanvas1 * screen) override;

private:

    std::vector<Tape *> tapes;
    uint8_t focus_tape;
    float phase;

};
