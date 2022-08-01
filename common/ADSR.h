//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include <cmath>
#include <cstdint>

class ADSR {

    enum STAGES {
        stageA, stageD, stageS, stageR, stageEnd
    };

    float phase = 0;
    uint8_t stage = stageR;

    float switch_level = 0;

    bool gate = false;
    float out = 0;
    float prev_a = 0.1;

    float A, D, S, R;

    float clockdiv;
    float smpcount;
    float out_points[2];
    const float phase_inc = 0.001;

public:

    ADSR() : ADSR(10) {}

    ADSR(float clockdiv_) {
        set(0.2, 1, 0.8, 30);
        clockdiv = clockdiv_;
        reset();
    }

    inline void reset() {
        stage = stageEnd;
        smpcount = clockdiv;
        phase = 0;
        out_points[0] = 0;
        out_points[1] = 0;
    }

    inline void set(float A_, float D_, float S_, float R_) {
        A = A_ + 0.1;
        D = D_ + 0.1;
        S = S_ + 0.000001;
        R = R_ + 0.1;
    }

    inline void gateOn() { gate = true; }
    inline void gateOff() { gate = false; }

    inline float get() const { return out; }

    inline bool end() const { return stage == stageEnd; }

    inline float calc_env_value();

    void process();
};
