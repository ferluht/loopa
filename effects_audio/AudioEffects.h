//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include "Chronoblob/Chronoblob.h"
#include "Delay/Delay.hpp"
#include "Limiter/SoftClipper.h"
#include "Plateau/Plateau.h"
#include "Tape/Tape.h"

class DummyAudio : public AMG {

public:
    DummyAudio() = default;

    void draw(NVGcontext * vg) override {
        nvgFontSize(vg, 10);
        nvgText(vg, 2, 12, "BYPASS AUDIO", NULL);
    }
};