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
    DummyInstrument() = default;

    void draw(NVGcontext * vg) override {
        nvgFontSize(vg, 10);
        nvgText(vg, 2, 12, "BYPASS INSTR", NULL);
    }
};