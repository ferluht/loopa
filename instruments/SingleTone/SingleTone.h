//
// Created by ferluht on 11/07/2022.
//

#pragma once

#include "Instrument.h"
#include "ADSR.h"

class SingleToneVoiceState : public VoiceState{
public:
    double beat;
    unsigned char velocity;
    float note;

    float glide;
    float glide_inc;
    float glide_dir;

    ADSR adsr;

    float volume;

    float phase1;
    float phase2;
};

class SingleTone : public PolyInstrument<SingleToneVoiceState> {

    float glide_time = 8000;

    float interp1 = 0.4, interp2 = 0.3;
    float crossmod = 0.5;

public:

    SingleTone() : PolyInstrument<SingleToneVoiceState>("SNGLTN") {
        set_voices(1);
    }

    void updateVoice(SingleToneVoiceState * voiceState, MData &cmd) override;

    void processVoice(SingleToneVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime, uint8_t nvoices) override;
};
