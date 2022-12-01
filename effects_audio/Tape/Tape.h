//
// Created by ferluht on 10/07/2022.
//

#pragma once

#include <AMG.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <Utils.hpp>

class Tape : public AMG {

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

    Tape() {
        audio.reserve(max_loop_size);

        addHandler( CC_HEADER,MIDICC::TAPE_TRIG, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) return trig();
            return MIDISTATUS::DONE;
        });

        addHandler( CC_HEADER,MIDICC::TAPE_CLEAR, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) return clear();
            return MIDISTATUS::DONE;
        });

        addHandler( CC_HEADER,MIDICC::TAPE_DOUBLE, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) return double_loop();
            return MIDISTATUS::DONE;
        });

        addHandler( CC_HEADER,MIDICC::TAPE_STOP, [this](MData &cmd) -> MIDISTATUS {
            looper_state = STOP;
            position = 0;
            return MIDISTATUS::DONE;
        });

        clear();
    };

    MIDISTATUS trig() {
        switch (looper_state) {
            case STOP:
                if (audio.size() > 0) looper_state = PLAY;
                else looper_state = REC;
                break;
            case REC:
                looper_state = OVERDUB;
                break;
            case OVERDUB:
                looper_state = PLAY;
                break;
            case PLAY:
                looper_state = OVERDUB;
                break;
            default:
                break;
        }
        return MIDISTATUS::DONE;
    }

    MIDISTATUS clear() {
        looper_state = STOP;
        audio.clear();
        position = 0;
        avg = 0;
        avg_env = 0;
        return MIDISTATUS::DONE;
    }

    MIDISTATUS double_loop() {
        if (doubling_progress == 0)
            doubling_size = audio.size();
        if (doubling_size * 2 > max_loop_size) {
            return MIDISTATUS::DONE;
        }
        auto beg = audio.begin() + doubling_progress;
        unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
        auto end = beg + step_size;
        std::copy(beg, end, std::back_inserter(audio));
        doubling_progress += step_size;
        if (doubling_progress == doubling_size) {
            doubling_progress = 0;
            return MIDISTATUS::DONE;
        }
        return MIDISTATUS::WAITING;
    }

    inline float envelope(float sample, float w, float w_env) {
        avg = w * sample + (1 - w) * avg;
        float i = std::abs(sample - avg);
        avg_env = w_env * i + (1 - w_env) * avg_env;
        return avg_env;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override {

        float envl, envr;

        for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
            switch (looper_state) {
                case STOP:
                    outputBuffer[i + 0] = inputBuffer[i + 0];
                    outputBuffer[i + 1] = inputBuffer[i + 1];
                    break;
                case REC:
                    audio.push_back(inputBuffer[i + 0]);
                    audio.push_back(inputBuffer[i + 1]);
                    outputBuffer[i + 0] = inputBuffer[i + 0];
                    outputBuffer[i + 1] = inputBuffer[i + 1];
                    break;
                case OVERDUB:
                    audio[position+0] = soft_clip(audio[position+0] + inputBuffer[i + 0]);
                    audio[position+1] = soft_clip(audio[position+1] + inputBuffer[i + 1]);
                    outputBuffer[i + 0] = audio[position+0];
                    outputBuffer[i + 1] = audio[position+1];
                    position = (position + 2) % audio.size();
                    break;
                case PLAY:
                    if (audio.empty()) break;
                    outputBuffer[i + 0] = soft_clip(audio[position+0] + inputBuffer[i + 0]);
                    outputBuffer[i + 1] = soft_clip(audio[position+1] + inputBuffer[i + 1]);
                    position = (position + 2) % audio.size();
                    break;
                default:
                    break;
            }
        }
    }

    void draw(GFXcanvas1 * screen) override;

    float getPosition();

    int getState();
};