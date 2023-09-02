//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <Effect.h>
#include "Dattorro.hpp"

class Plateau : public AudioEffect{
public:
    Plateau();

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(GFXcanvas1 * screen) override;

private:
    float dry = 0;
    Dattorro * dattorro;
};

