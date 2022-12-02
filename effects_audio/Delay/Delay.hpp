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

#define HISTORY_SIZE (1<<21)

/// @private
class Delay {

    DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
    DoubleRingBuffer<float, 16> outBuffer;
    SRC_STATE* src;
    float lastWet = 0.f;

public:

    float in = 0;
    float out = 0;
    float feedback = 0;
    float mix = 0;
    float delay = 0;
    float wet = 0;

    Delay() {
        src = src_new(SRC_SINC_FASTEST, 1, nullptr);
        assert(src);
    }

    ~Delay() {
        src_delete(src);
    }

    void process();

private:

    inline float clamp(float x, float a, float b) {
        return std::max(std::min(x, b), a);
    }

    inline float crossfade(float a, float b, float p) {
        return a + (b - a) * p;
    }
};