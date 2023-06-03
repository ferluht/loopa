//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include <Instrument.h>
#include "SimpleInstrument/SimpleInstrument.h"
#include "SingleTone/SingleTone.h"
#include "Sampler/Sampler.h"
#include "Sampler/SampleKit.h"

class DummyInstrument : public Instrument {

public:
    DummyInstrument() : Instrument("EMPTY") {

    }

    void draw(GFXcanvas1 * screen) override {
        screen->print("NO INSTRUMENT");
    }
};