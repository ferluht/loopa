//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include "Instrument.h"
#include <cmath>
#include "ADSR.h"

class SimpleInstrumentVoiceState : public VoiceState {

public:
    float phase, note, volume, phase_inc;
    ADSR adsr;

    SimpleInstrumentVoiceState() : VoiceState() {
        phase = 0;
        note = 60;
        volume = 1;
        phase_inc = 0;
    }
};

class SimpleInstrument : public PolyInstrument<SimpleInstrumentVoiceState> {
public:

    SimpleInstrument() : PolyInstrument<SimpleInstrumentVoiceState>() {

    }

    void updateVoice(SimpleInstrumentVoiceState * voiceState, MData cmd) override;

    void processVoice(SimpleInstrumentVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
        unsigned int nBufferFrames, double streamTime, uint8_t nvoices) override;

    void draw(GFXcanvas1 * screen) override;
};

