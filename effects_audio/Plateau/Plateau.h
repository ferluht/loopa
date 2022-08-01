//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <AMG.h>
#include "Dattorro.hpp"

class Plateau : public AMG{
public:
    Plateau() : AMG() {
        dattorro = new Dattorro();
        dattorro->size = 0.95;
        dattorro->decay = 0.9;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(NVGcontext * vg) override;

private:

    Dattorro * dattorro;
};

