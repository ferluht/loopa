//
// Created by ferluht on 28/07/2022.
//

#pragma once

#include "WavFile.hpp"
#include "Instrument.h"

class SamplerState : public VoiceState{
public:

    unsigned char velocity;
    float note;

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

//    GUI::Plot<GUI::TimeGraph> * plot;

public:

    Sampler(const char * sample_name_);

    static float InterpolateCubic(float x0, float x1, float x2, float x3, float t);
    static float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t);

    void updateVoice(SamplerState * state, MData md) override;

//    void MIn(double beat) override ;

    void processVoice(SamplerState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime, uint8_t nvoices) override ;

    void draw(NVGcontext * vg) override;

};
