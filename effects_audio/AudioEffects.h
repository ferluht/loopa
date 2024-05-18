//
// Created by ferluht on 22/07/2022.
//

#pragma once

#include <AMG.h>
#include "Chronoblob/Chronoblob.h"
#include "Delay/Delay.hpp"
#include "Limiter/SoftClipper.h"
#include "Plateau/Plateau.h"
#include "Tanhx/Tanhx.h"

class DummyAudioFX : public AudioEffect {

public:
    DummyAudioFX() : AudioEffect("NO FX") {

    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override {
        for (int i = 0; i < nBufferFrames * 2; i += 2) {
            outputBuffer[i] = inputBuffer[i];
            outputBuffer[i + 1] = inputBuffer[i + 1];
        }
    }
};