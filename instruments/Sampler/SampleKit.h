//
// Created by ferluht on 31/07/2022.
//

#pragma once

#include <Instrument.h>
#include "Sampler.h"

class SampleKit : public Instrument {

    AMG * notes[128];
    std::list<AMG *> activeChains;

public:

    SampleKit(const char * name);

    void addSample(const char * sample_name_, const char note);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;
};