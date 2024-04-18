//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include "PingPong.hpp"
#include <AMG.h>

/// @private
class Chronoblob : public AMG{
public:
    Chronoblob() : AMG("CHRNBLB") {
        pingpong = new PingPong();
        pingpong->sync = false;
        pingpong->mode = false;
        pingpong->leftTime = 0.657;
        pingpong->rightTime = 0.523;
        pingpong->feedback = 0.5;
        pingpong->dw = 0.95;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

private:

    PingPong * pingpong;
};
