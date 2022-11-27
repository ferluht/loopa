//
// Created by ferluht on 11/07/2022.
//

#include "SingleTone.h"

void SingleTone::updateVoice(SingleToneVoiceState * voiceState, MData cmd) {
    if ((cmd.status == NOTEON_HEADER) && (cmd.data2 != 0)) {
        if (!voiceState->isActive()) {
            voiceState->glide = 0;
            voiceState->glide_inc = 0;
            voiceState->glide_dir = 0;
            voiceState->phase1 = 0;
            voiceState->phase2 = 0;
        } else {
            voiceState->glide = - cmd.data1 + voiceState->note;
            voiceState->glide_dir = voiceState->glide / abs(voiceState->glide);
            voiceState->glide_inc = - voiceState->glide / glide_time;
        }
        voiceState->note = cmd.data1;
        voiceState->volume = (float)cmd.data2/127.f;
        voiceState->enable();
        voiceState->adsr.gateOn();
    }else{
        if (cmd.data1 == voiceState->note){
            voiceState->adsr.gateOff();
        }
    }
}

void SingleTone::processVoice(SingleToneVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                                    unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {
    for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
        float first = sinf(voiceState->phase1) * interp1 + (voiceState->phase1 / M_PI*2 * 2 - 1) * (1 - interp1);
        float second = sinf(voiceState->phase2) * interp2;

        if (voiceState->phase2 / M_PI*2 > 0.5) second += 1 - interp2;
        else second -= 1 - interp2;
//    + (state->phase2 / 6.283f * 2 - 1) * (1 - (*interp2 + 1)/2);

        voiceState->adsr.process();

        float sample = (first * crossmod + first * second * (1 - crossmod)) * voiceState->volume * voiceState->adsr.get() * instrument_volume;

        if (voiceState->adsr.end()) voiceState->disable();

        voiceState->phase1 += getPhaseIncrement(voiceState->note - 12 + voiceState->glide);
        if (voiceState->phase1 > M_PI*2) voiceState->phase1 -= M_PI*2;

        voiceState->phase2 += getPhaseIncrement(voiceState->note + voiceState->glide);
        if (voiceState->phase2 > M_PI*2) voiceState->phase2 -= M_PI*2;

        if (voiceState->glide * voiceState->glide_dir > 0)
            voiceState->glide += voiceState->glide_inc;

        outputBuffer[i+0] = inputBuffer[i+0] + sample * 0.1;
        outputBuffer[i+1] = inputBuffer[i+1] + sample * 0.1;
    }
}

void SingleTone::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    screen->print("SINGLETONE");
}