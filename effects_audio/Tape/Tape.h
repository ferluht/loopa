//
// Created by ferluht on 10/07/2022.
//

#pragma once

#include <AudioEffects.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <Utils.hpp>

class Tape : public AudioEffect {

    const int TAPE_MAX_MINUTES = 6;

    float s_phase = 0;

    unsigned int doubling_progress = 0;
    unsigned int doubling_size = 0;
    const unsigned int max_doubling_stepsize = 8192;
    const unsigned int max_loop_size = TAPE_MAX_MINUTES*60*SAMPLERATE;

    int looper_state = STOP;

    bool waitingforsync = false;

    unsigned long position = 0;

    std::vector<float> audio;

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

public:

    enum TAPE_STATE {
        STOP,
        PLAY,
        REC,
        OVERDUB
    };

    static const float looper_ratio;

    Tape();

    MIDISTATUS trig();
    MIDISTATUS clear();
    MIDISTATUS double_loop();

//    inline float envelope(float sample, float w, float w_env) {
//        avg = w * sample + (1 - w) * avg;
//        float i = std::abs(sample - avg);
//        avg_env = w_env * i + (1 - w_env) * avg_env;
//        return avg_env;
//    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;
    void draw(GFXcanvas1 * screen) override;
    float getPosition();
    int getState();
};