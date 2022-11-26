//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <AMG.h>

class Scale : public AMG {

public:
    Scale() : AMG("SCALE") {

    }

    void midiIn(MData &cmd) override;

    void draw(GFXcanvas1 * screen) override;
};
