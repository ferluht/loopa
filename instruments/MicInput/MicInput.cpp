//
// Created by ferluht on 11/07/2022.
//

#include "MicInput.h"

MicInput::MicInput() : PolyInstrument<MicInputVoiceState>("MICIN") {
    set_voices(1);
    volume_knob = addParameter("VOL", 0.5);
    release_knob = addParameter("REL", 0.5);
}

void MicInput::updateVoice(MicInputVoiceState * voiceState, MData cmd) {
    if ((cmd.status == MIDI::GENERAL::NOTEON_HEADER) && (cmd.data2 != 0)) {
        voiceState->note = cmd.data1;
        voiceState->velocity = (float)cmd.data2/127.f;
        voiceState->enable();

        voiceState->adsr.set(1, 0.01, 1, 1);
        voiceState->adsr.gateOn();
    }else{
        if (cmd.data1 == voiceState->note){
            voiceState->adsr.gateOff();
        }
    }
}

void MicInput::processVoice(MicInputVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                                    unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {
    for (unsigned int i=0; i<2*nBufferFrames; i+=2) {

        voiceState->adsr.process();
        if (voiceState->adsr.end()) voiceState->disable();
        float vol = voiceState->adsr.get() * (volume_knob->value * 10 + 0.5);

        outputBuffer[i+0] = inputBuffer[i+0] * vol * 10;
        outputBuffer[i+1] = inputBuffer[i+1] * vol * 10;
    }
}