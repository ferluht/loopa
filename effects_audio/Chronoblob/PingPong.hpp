//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <SmoothCrossFade.hpp>
#include <ExpFilter.hpp>
#include <Delay/Delay.hpp>

/// @private
class PingPong : public AudioEffect {
public:

    Delay* leftDelay = new Delay();
    Delay* rightDelay = new Delay();

    float feedback = 0.35;
    float dw = 0.95;

    bool sync = false;
    bool prevSync = false;

    bool mode = false;
    bool prevMode = false;

    float leftTime = 0;
    float rightTime = 0;

    float leftIn = 0;
    float rightIn = 0;
    float leftOut = 0;
    float rightOut = 0;

    PingPong() : AudioEffect("PINGPONG") {

    }

    void updateParam();
    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;
};