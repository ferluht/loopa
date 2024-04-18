//
// Created by ferluht on 11/07/2022.
//

#include "MicInput.h"

MicInput::MicInput() : PolyInstrument<MicInputVoiceState>("MICIN") {
    set_voices(1);
    line_in = false;
    std::system("amixer set 'Capture Mux' 'MIC_IN'");
    volume_knob = addParameter("VOL", 0.1);
    release_knob = addParameter("REL", 0.5);
}

void MicInput::updateVoice(MicInputVoiceState * voiceState, MData &cmd) {
    if ((cmd.status == MIDI::GENERAL::NOTEON_HEADER) && (cmd.data2 != 0)) {
        if (cmd.data1 % 12 == 11) {
            voiceState->note = cmd.data1;
            voiceState->velocity = (float)cmd.data2/127.f;
            voiceState->enable();

            voiceState->adsr.set(1, 0.01, 1, 1);
            voiceState->adsr.gateOn();
        }
    } else {
        if (cmd.data1 % 12 == 11) {
            if (cmd.data1 == voiceState->note) {
                voiceState->adsr.gateOff();
            }
        }
    }

//    if (cmd.status == MIDI::GENERAL::NOTEON_HEADER || cmd.status == MIDI::GENERAL::NOTEOFF_HEADER) {
//        switch (cmd.data1 % 12) {
//            case 0:
//                if (cmd.data2 > 0) {
//                    if (line_in) std::system("amixer set 'Capture Mux' 'MIC_IN'");
//                    else std::system("amixer set 'Capture Mux' 'LINE_IN'");
//                    line_in = ~line_in;
//                }
//                break;
//            case 5:
//            case 6:
//                cmd.status = MIDI::GENERAL::CC_HEADER + (cmd.data1 % 12) - 5;
//                cmd.data1 = MIDI::UI::TAPE::TRIG;
//                break;
//            case 7:
//            case 8:
//                cmd.status = MIDI::GENERAL::CC_HEADER + (cmd.data1 % 12) - 7;
//                cmd.data1 = MIDI::UI::TAPE::CLEAR;
//                break;
//            default:
//                break;
//        }
//    }
}

void MicInput::processVoice(MicInputVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                                    unsigned int nBufferFrames, Sync & sync, uint8_t nvoices) {

    for (unsigned int i=0; i<2*nBufferFrames; i+=2) {

        voiceState->adsr.process();
        if (voiceState->adsr.end()) voiceState->disable();
        float vol = voiceState->adsr.get() * (volume_knob->value * 5 + 0.2);

        outputBuffer[i+0] = inputBuffer[i+0] * vol * 10;
        outputBuffer[i+1] = inputBuffer[i+1] * vol * 10;
    }
}