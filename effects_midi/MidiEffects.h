//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "Scale.h"

class DummyMidiFX : public MIDIEffect {

public:

    DummyMidiFX() : MIDIEffect("NO FX") {

    }

    void draw(GFXcanvas1 * screen) override {
        screen->print("NO MIDI FX");
    }
};

