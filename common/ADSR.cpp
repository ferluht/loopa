//
// Created by ferluht on 08/07/2022.
//

#include <iostream>
#include "ADSR.h"

#include <cstdint> // uint32_t

inline float Q_rsqrt(float number)
{
    union {
        float    f;
        uint32_t i;
    } conv = { .f = number };
    conv.i  = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5F - (number * 0.5F * conv.f * conv.f);
    return conv.f;
}

inline float ADSR::calc_env_value() {
    float env = 0;

    if (gate) {
        switch (stage) {
            case stageEnd:
            case stageR:
                stage = stageA;
                phase = out * out * prev_a;
            case stageA:
//                if (out == 0) out = 1;
                env = 1 / Q_rsqrt(phase / prev_a);
                if (env >= 1) {
                    stage = stageD;
                    switch_level = env;
                }
                break;
            case stageD:
                env = S + (switch_level - 1 / Q_rsqrt((phase - prev_a) / D)) * (switch_level - S);
                if (env <= S) stage = stageS;
                break;
            case stageS:
                env = out;
                break;
        }
    } else {
        prev_a = A;
        switch (stage) {
            case stageA:
            case stageD:
            case stageS:
                stage = stageR;
                phase = phase_inc * clockdiv;
                switch_level = out;
            case stageR:
                env = (1 - 1 / Q_rsqrt(phase / R)) * switch_level;
                if (env <= 0) {
                    env = 0;
                }
                break;
            default:
                break;
        }
    }

    if (env > 1) env = 1;
    return env;
}

void ADSR::process() {

    if (stage == stageEnd && !gate) {
        out = 0;
        return;
    }

    if (smpcount >= clockdiv) {

        phase += phase_inc * clockdiv;

        if (out_points[0] > 0 && out_points[1] == 0) {
            reset();
        } else {
            out_points[0] = out_points[1];
            out_points[1] = calc_env_value();
            smpcount = 0;
        }
    }

    if (stage != stageEnd) {
        out = out_points[0] + (out_points[1] - out_points[0]) * smpcount / clockdiv;
        smpcount += 1;
    } else {
        out = 0;
    }
}