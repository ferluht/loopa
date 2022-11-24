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

    void draw(GFXcanvas1 * screen) override {
        screen->setCursor(30, 6);
        screen->setTextSize(1);
        screen->print("BYPASS AUDIO");
    }
};