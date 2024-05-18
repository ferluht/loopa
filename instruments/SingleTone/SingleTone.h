//
// Created by ferluht on 11/07/2022.
//

#pragma once

#include "AMG.h"
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

    Parameter * glide_time;

    Parameter * crossmod;
    Parameter * interp1, * interp2;

public:
    static DeviceFactory* create() { return new SingleTone(); }

    SingleTone() : PolyInstrument<SingleToneVoiceState>("SingleTone") {
        set_voices(1);
        crossmod = addParameter("XMOD", 0, 1, 0.5, 0.05);
        interp1 = addParameter("I_1", 0, 1, 0.4, 0.05);
        interp2 = addParameter("I_2", 0, 1, 0.3, 0.05);
        glide_time = addParameter("TIME", 0, 40000, 4000, 500);
    }

    void updateVoice(SingleToneVoiceState * voiceState, MData &cmd) override;

    void processVoice(SingleToneVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, Sync & sync, uint8_t nvoices) override;

    const char * getName() override { return "SYNTH"; }
};
