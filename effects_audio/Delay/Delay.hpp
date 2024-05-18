//
// Created by ferluht on 09/07/2022.
//

#pragma once

#include <cstdio>
#include <cstdlib>
#include <libsamplerate/include/samplerate.h>
#include <RingBuffer.hpp>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <AMG.h>

#define HISTORY_SIZE (1<<21)

/// @private
class Delay : public AudioEffect {

    DoubleRingBuffer<float, HISTORY_SIZE> historyBufferL;
    DoubleRingBuffer<float, HISTORY_SIZE> historyBufferR;
    DoubleRingBuffer<float, 512> outBuffer;
    SRC_STATE* src;
    float lastWetL = 0.f;
    float lastWetR = 0.f;

    float prevlinelength = 0;

public:
    static DeviceFactory* create() { return new Delay(); }

    float wetL = 0;
    float wetR = 0;

    Parameter * time, * drywet, * feedback;

    Delay() : AudioEffect("Delay") {
        src = src_new(SRC_SINC_FASTEST, 1, nullptr);
        assert(src);

        drywet = addParameter("D/W", 0.60);
        time = addParameter("TIME", 0.0);
        feedback = addParameter("FB", 0.45);
    }

    ~Delay() {
        src_delete(src);
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

//    void draw(GFXcanvas1 * screen) override;

private:

    inline float clamp(float x, float a, float b) {
        return std::max(std::min(x, b), a);
    }

    inline float crossfade(float a, float b, float p) {
        return a + (b - a) * p;
    }
};