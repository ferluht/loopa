//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <AMG.h>
#include "Dattorro.hpp"

class Plateau : public AudioEffect{
public:
    static DeviceFactory* create() { return new Plateau(); }
    Plateau();

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

private:

    Parameter * drywet, * size, * decay;
    Dattorro * dattorro;
};

