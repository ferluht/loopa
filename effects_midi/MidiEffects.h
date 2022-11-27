//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "Scale.h"

class DummyMidi : public AMG {

public:

    DummyMidi() : AMG("NO FX") {

    }

    void draw(GFXcanvas1 * screen) override {
        screen->setCursor(4, 16);
        screen->setTextSize(1);
        screen->print("NO MIDI FX");
    }
};

