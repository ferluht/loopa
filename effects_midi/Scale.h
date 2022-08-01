//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <AMG.h>

class Scale : public AMG {

public:
    Scale() {

    }

    void midiIn(MData &cmd) override;

    void draw(NVGcontext * vg) override;
};
