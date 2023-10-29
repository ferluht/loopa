//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include <cmath>
#include <cstdint>
#include <A.h>

/**
 * @private
 */
class ADSR {

    enum STAGES {
        stageA, stageD, stageS, stageR, stageEnd
    };

    float phase = 0;
    uint8_t stage = stageR;

    float switch_level = 0;

    bool gate = false;
    float env = 0;

    float A, D, S, R;
    float dA, dD, dR;

    float d = 0;
    float alpha = 0.3;

    inline void update_d(float target) {
        d = target * alpha + d * (1 - alpha);
//        if (std::abs(d - target) > 0.001) std::cout << d << "\n";
    }

public:

    ADSR() {
        set(0.01, 0.05, 0.9, 1);
        reset();
    }

    inline void reset() {
        stage = stageEnd;
        phase = 0;
        env = 0;
    }

    inline void set(float A_, float D_, float S_, float R_) {
        A = A_ * SAMPLERATE;
        D = D_ * SAMPLERATE;
        S = S_;
        R = R_ * SAMPLERATE;

        dA = 1.f / A;
        dD = - 1.f / D;
        dR = - 1.f / R;
        d = 0;
    }

    inline void gateOn() { gate = true; }
    inline void gateOff() { gate = false; }

    inline float get() const { return env; }

    inline bool end() const { return stage == stageEnd; }
    inline void forcePeak() { gate = true; stage = stageA; env = 1; phase = 1; }

    void process();
};
