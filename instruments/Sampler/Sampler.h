//
// Created by ferluht on 28/07/2022.
//

#pragma once

#include "WavFile.hpp"
#include "ADSR.h"
#include "Instrument.h"


class SamplerState : public VoiceState{
public:

    SamplerState() {
        adsr.set(0.01, 0.1, 1.0, 5.0);
        out = 0;
        time = 0;
        transient = false;
        alpha = 0;
    }

    unsigned char velocity;
    float note;

    ADSR adsr;
    float volume;

    bool transient;
    float time;

    float out;
    float alpha;
};

class Sampler : public PolyInstrument<SamplerState> {

    WavFile<float> sample;
    const char * sample_name;

//    GUI::TapButton * trig;
//    GUI::Encoder * pitch;
    bool const_pitch;

    bool triggered = false;

    Parameter * decay, * voices;

//    GUI::Plot<GUI::TimeGraph> * plot;

public:

    Sampler(const char * name, const char * sample_name_, int8_t note);

    static float InterpolateCubic(float x0, float x1, float x2, float x3, float t);
    static float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t);

    void updateVoice(SamplerState * state, MData md) override;

//    void MIn(double beat) override ;

    void processVoice(SamplerState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime, uint8_t nvoices) override ;

};
