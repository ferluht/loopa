//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include "PingPong.hpp"
#include <A.h>

class Chronoblob : public A{
public:
    Chronoblob() : A() {
        pingpong = new PingPong();
        pingpong->sync = false;
        pingpong->mode = false;
        pingpong->leftTime = 4.257;
        pingpong->rightTime = 1.257;
        pingpong->feedback = 0.85;
        pingpong->dw = 0.95;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

private:

    PingPong * pingpong;
};
