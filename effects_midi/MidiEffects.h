//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "Scale.h"

class DummyMidi : public AMG {

public:
    DummyMidi() = default;

    void draw(GFXcanvas1 * screen) override {
        screen->setCursor(30, 6);
        screen->setTextSize(1);
        screen->print("BYPASS MIDI");
    }
};

