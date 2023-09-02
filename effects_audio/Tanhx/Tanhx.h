//
// Created by ferluht on 13/08/2023.
//

#pragma once
#include "Effect.h"

inline float db_to_k(float x) {
    return std::pow(10, x / 20);
}

class FPFilter {

    float A, B, C, D;
    float a0, a1, b0, b1;
    float x1, y1;

public:

    FPFilter () {
        a0 = 1;
        a1 = 0;
        b1 = 0;
    }

    inline void reset() {
        x1 = 0;
        y1 = 0;
    }

    inline float processSample(float x) {
        float y = a0 * x + a1 * x1 + b1 * y1;
        x1 = x;
        y1 = y;
        return y;
    }

    inline void fplowshelf(float gain) {
        float sgain = std::sqrt(gain);
        A = 1;
        B = sgain;
        C = 1;
        D = 1 / sgain;
    }

    inline void performBLT(float freq_k) {
        float w0 = 2 * M_PI * freq_k;
        float KK = (w0 < M_PI) ? (1 / std::tan(w0 / 2)) : 0;
        a0 = B + A * KK;
        a1 = B - A * KK;
        b0 = D + C * KK;
        b1 = D - C * KK;

        if (b0 != 0) {
            a0 = a0 / b0;
            a1 = a1 / b0;
            b1 = -b1 / b0;
        } else {
            a0 = 1;
            a1 = 0;
            b1 = 0;
        }
    }

    inline void lowshelf(float freq_k, float gain) {
        fplowshelf(gain);
        performBLT(freq_k);
    }

    inline float getAmp(float freq_k) {
        float w = 2 * M_PI * freq_k;
        float cw = cos(w);
        return (a0 * a0 + 2 * a0 * a1 * cw + a1 * a1) / (1 - 2 * b1 * cw + b1 * b1);
    }
};

class Tanhx_saturator {

    const float saturate_sample_k = 7 / (71.4875 * std::log(10));

    const float dc_freq_hz = 5;
    const float pre_emphasis_db = -12;
    const float pre_emphasis_hz = 159;

    float f, s, gain, bias, bias_offset;

    FPFilter * pre_emphasis, * post_emphasis;

public:

    bool use_emphasis = false;

    Tanhx_saturator() {
        pre_emphasis = new FPFilter();
        post_emphasis = new FPFilter();
    }

    inline float tanh(float x) {
        float a = std::exp(x * 2);
        return (a - 1) / (a + 1);
    }

    inline float saturateSample(float x, float bias) {
        float a = std::pow(10, ((x - bias) * 20.425 + (x * x - bias * bias) * 12.75));
        return (a - 1) / (a + 1) * saturate_sample_k;
    }

    inline void setSampleRate(float srate) {
        f = dc_freq_hz / srate;

        pre_emphasis->lowshelf(pre_emphasis_hz / srate, db_to_k(pre_emphasis_db));
        post_emphasis->lowshelf(pre_emphasis_hz / srate, db_to_k(-pre_emphasis_db));
    }

    inline void reset() {
        s = 0;
        pre_emphasis->reset();
        post_emphasis->reset();
    }

    inline float processSample(float x) {
        x *= gain;

        if (use_emphasis)
            x = pre_emphasis->processSample(x);

        bias_offset += f * (bias - bias_offset);

        // saturation
        float y = saturateSample(x, s) + bias_offset;

        // DC remover
        s += f * y;

        if (use_emphasis)
            y = post_emphasis->processSample(y);

        return y / gain;
    }

    inline void setSatThreshold(float threshold_k, float bias_offset) {
        gain = 1.f / threshold_k * 0.1;
        bias = bias_offset * 0.1;
    }
};

class Tanhx : public AudioEffect  {

    Parameter * driveP, * biasP, * outputP, * emphasisP;

    Tanhx_saturator sL, sR;

public:

    Tanhx() : AudioEffect("TANHX")  {
        driveP = addParameter("DRIV", -30, 40, 0.1, 1);
        biasP = addParameter("BIAS", -30, 30, 0.1, 1);
        outputP = addParameter("OUT", -15, 15, 0.1, 1);
        emphasisP = addParameter("EMP", {"ON", "OFF"}, 1);

        sL.setSampleRate(SAMPLERATE);
        sR.setSampleRate(SAMPLERATE);

        sL.reset();
        sR.reset();
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;
};

