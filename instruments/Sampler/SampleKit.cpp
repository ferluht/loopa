//
// Created by ferluht on 31/07/2022.
//

#include "SampleKit.h"

SampleKit::SampleKit(const char * name) : Instrument(name) {
    for (int i = 0; i < 128; i ++) notes[i] = nullptr;

    addMIDIHandler({MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, [this](MData &cmd) -> MIDISTATUS {
        if (notes[cmd.data1])
            return notes[cmd.data1]->midiIn(cmd);
        return MIDISTATUS::DONE;
    });
}

void SampleKit::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    for (auto const& chain : activeChains) chain->process(outputBuffer, inputBuffer, nBufferFrames, streamTime);
}

void SampleKit::addSample(const char *sample_name_, const char note) {
    auto chain = new Sampler(sample_name_, sample_name_, note);
    notes[note] = chain;
    activeChains.push_back(chain);
}