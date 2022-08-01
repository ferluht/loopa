//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "Scale.h"

class DummyMidi : public AMG {

public:
    DummyMidi() = default;

    void draw(NVGcontext * vg) override {
        nvgFontSize(vg, 10);
        nvgText(vg, 2, 12, "BYPASS MIDI", NULL);
    }
};

