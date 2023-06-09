//
// Created by ferluht on 11/07/2022.
//

#pragma once

#include "Instrument.h"
#include "ADSR.h"

class MicInputVoiceState : public VoiceState{
public:
    unsigned char velocity;
    float note;
    ADSR adsr;
};

class MicInput : public PolyInstrument<MicInputVoiceState> {

    Parameter * volume_knob;
    Parameter * release_knob;

public:

    MicInput();

    void updateVoice(MicInputVoiceState * voiceState, MData cmd) override;

    void processVoice(MicInputVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime, uint8_t nvoices) override;
};
