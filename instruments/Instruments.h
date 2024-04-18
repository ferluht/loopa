//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include <Instrument.h>
#include "SimpleInstrument/SimpleInstrument.h"
#include "SingleTone/SingleTone.h"
#include "Sampler/Sampler.h"
#include "Sampler/SampleKit.h"
#include "MicInput/MicInput.h"

class DummyInstrument : public Instrument {

public:
    DummyInstrument() : Instrument("EMPTY") {
        addDrawHandler({SCREENS::LOOP_VIEW}, [this](GFXcanvas1 * screen) -> void {
            screen->print("NO INSTRUMENT");
        });
    }
};