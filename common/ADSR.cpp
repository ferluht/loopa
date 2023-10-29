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

void ADSR::process() {

    if (stage == stageEnd && !gate) {
        env = 0;
        return;
    }

    if (gate) {
        switch (stage) {
            case stageEnd:
            case stageR:
                stage = stageA;
                phase = env * env;
                d = dR;
                update_d(dA);
                phase += d;
                if (phase < 0) {
                    d = 0;
                    phase = 0;
                }
            case stageA:
                env = 1 / Q_rsqrt(phase);
                phase += d;
                if (phase < 0) {
                    d = 0;
                    phase = 0;
                }
                if (phase >= 1) phase = 1;
                update_d(dA);

                if (phase >= 1) {
                    stage = stageD;
                    phase = 1;
                    switch_level = env;
                }
                break;
            case stageD:
                env = S + phase * phase * (switch_level - S);
                phase += dD;
                if (phase < 0) phase = 0;
                if (env <= S) stage = stageS;
                if (env < 0) env = 0;
                break;
            case stageS:
                break;
        }
    } else {
        switch (stage) {
            case stageA:
            case stageD:
            case stageS:
                stage = stageR;
                phase = 1;
                switch_level = env;
            case stageR:
                env = phase * phase * switch_level;
                phase += dR;
                if (phase < 0) phase = 0;
                if (env <= 1e-16) {
                    env = 0;
                    stage = stageEnd;
                    d = 0;
                }
                break;
            default:
                break;
        }
    }

    if (env > 1) env = 1;
    if (env < 1e-16) env = 0;
}