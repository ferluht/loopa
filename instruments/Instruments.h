//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "SimpleInstrument/SimpleInstrument.h"
#include "SingleTone/SingleTone.h"
#include "Sampler/Sampler.h"
#include "Sampler/SampleKit.h"

class DummyInstrument : public AMG {

public:
    DummyInstrument() : AMG("EMPTY") {

    }

    void draw(GFXcanvas1 * screen) override {
        screen->setCursor(30, 6);
        screen->setTextSize(1);
        screen->print("BYPASS INSTR");
    }
};