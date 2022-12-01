//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <AMG.h>
#include "Dattorro.hpp"

class Plateau : public AMG{
public:
    Plateau() : AMG("PLATEAU") {
        dattorro = new Dattorro();
        dattorro->size = 0.95;
        dattorro->decay = 0.9;

        addHandler( CC_HEADER,CC_E1, [this](MData &cmd) -> MIDISTATUS {
            dry = (float)cmd.data2 / 127.0;
            return MIDISTATUS::DONE;
        });
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void draw(GFXcanvas1 * screen) override;

private:
    float dry = 0.5;
    Dattorro * dattorro;
};

