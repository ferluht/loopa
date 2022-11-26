//
// Created by ferluht on 31/07/2022.
//

#pragma once

#include <Instrument.h>
#include "Sampler.h"

class SampleKit : public AMG {

    AMG * notes[128];
    std::list<AMG *> activeChains;

public:

    SampleKit() : AMG("KIT") {
        for (int i = 0; i < 128; i ++) notes[i] = nullptr;
    }

    void addSample(const char * sample_name_, const char note);

    void midiIn(MData & cmd) override;

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(GFXcanvas1 * screen) override;
};