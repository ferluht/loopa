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

    float s_phase = 0;

    int looper_state = STOP;

    bool waitingforsync = false;

    unsigned long position = 0;

    std::vector<float> audio;

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

    uint8_t cc_code = 0;

public:

    enum TAPE_STATE {
        STOP,
        PLAY,
        REC,
        OVERDUB
    };

    static const float looper_ratio;

    Tape() : Tape(110) {};

    Tape(uint8_t cc_code) : AMG() {
        this->cc_code = cc_code;
        clear();
    }

    void trig() {
        switch (looper_state) {
            case STOP:
                looper_state = REC;
                std::cout << "Looper: REC" << std::endl;
                break;
            case REC:
                looper_state = OVERDUB;
                std::cout << "Looper: OVERDUB" << std::endl;
                break;
            case OVERDUB:
                looper_state = PLAY;
                std::cout << "Looper: PLAY" << std::endl;
                break;
            case PLAY:
                looper_state = OVERDUB;
                std::cout << "Looper: OVERDUB" << std::endl;
                break;
            default:
                break;
        }
    }

    void clear() {
        looper_state = STOP;
        audio.clear();
        position = 0;
        avg = 0;
        avg_env = 0;
        std::cout << "Looper: CLEAR" << std::endl;
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


    void midiIn(MData &cmd) override;

    void draw(GFXcanvas1 * screen) override;

    float getPosition();

    int getState();
};