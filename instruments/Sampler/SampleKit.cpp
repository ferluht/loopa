//
// Created by ferluht on 31/07/2022.
//

#include "SampleKit.h"

void SampleKit::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    for (auto const& chain : activeChains) chain->process(outputBuffer, inputBuffer, nBufferFrames, streamTime);
}

void SampleKit::addSample(const char *sample_name_, const char note) {
    auto chain = new Sampler(sample_name_);
    notes[note] = chain;
    activeChains.push_back(chain);
}

void SampleKit::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    screen->print("D&B KIT");
}