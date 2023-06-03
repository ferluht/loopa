//
// Created by ferluht on 08/07/2022.
//

#include <iostream>
#include "SimpleInstrument.h"

void SimpleInstrument::updateVoice(SimpleInstrumentVoiceState * voiceState, MData cmd) {
    if (((cmd.status & 0xF0) == MIDI::GENERAL::NOTEON_HEADER) && (cmd.data2 != 0)) {
        voiceState->note = cmd.data1;
        voiceState->volume = (float)cmd.data2/127.f;
        voiceState->phase_inc = getPhaseIncrement(cmd.data1);
        if (!voiceState->isActive()) voiceState->phase = -2*M_PI;
        voiceState->adsr.gateOn();
        voiceState->enable();
    }else{
        voiceState->adsr.gateOff();
    }
}

void SimpleInstrument::processVoice(SimpleInstrumentVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                  unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {
    for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
        voiceState->adsr.process();
        float out = sinf(voiceState->phase) / (float)nvoices * voiceState->adsr.get() * voiceState->volume * instrument_volume;
        outputBuffer[i + 0] += out;
        outputBuffer[i + 1] += out;
        voiceState->phase += voiceState->phase_inc;
        if (voiceState->phase > M_PI * 2) voiceState->phase -= M_PI * 2;
        if (voiceState->adsr.end()) voiceState->disable();
    }
}