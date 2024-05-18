//
// Created by ferluht on 31/07/2022.
//

#include "SampleKit.h"

SampleKit::SampleKit() : Instrument("DrumKit") {
    for (int i = 0; i < 128; i ++) notes[i] = nullptr;

    addMIDIHandler({}, {MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, {}, [this](MData &cmd, Sync & sync) -> void {
        if (notes[cmd.data1])
            notes[cmd.data1]->midiIn(cmd, sync);
    });
}

void SampleKit::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, Sync & sync) {
    for (auto const& chain : activeChains) chain->process(outputBuffer, inputBuffer, nBufferFrames, sync);
}

void SampleKit::addSample(const char *sample_name_, const char note) {
    auto chain = new Sampler();
    chain->init(sample_name_, note);
    notes[note] = chain;
    activeChains.push_back(chain);
}

void SampleKit::save(tinyxml2::XMLDocument *xmlDoc, tinyxml2::XMLElement *state) {
    for (const auto & chain : activeChains) {
        tinyxml2::XMLElement * chainState = xmlDoc->NewElement("Sampler");
        chain->save(xmlDoc, chainState);
        state->InsertEndChild(chainState);
    }
}

void SampleKit::load(tinyxml2::XMLElement *state) {
    activeChains.clear();
    for (int i = 0; i < 128; i ++) notes[i] = nullptr;
    auto chainState = state->FirstChildElement();
    while (chainState != nullptr) {
        auto chain = new Sampler();
        chain->load(chainState);
        notes[(int)chain->base_note] = chain;
        activeChains.push_back(chain);
        chainState = chainState->NextSiblingElement();
    }
}

namespace {
    DeviceFactory::AddToRegistry<SampleKit> _("SampleKit");
}