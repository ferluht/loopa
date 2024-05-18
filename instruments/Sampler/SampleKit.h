//
// Created by ferluht on 31/07/2022.
//

#pragma once

#include <AMG.h>
#include "Sampler.h"

class SampleKit : public Instrument {

    AMG * notes[128];
    std::list<AMG *> activeChains;

public:
    static DeviceFactory* create() { return new SampleKit(); }

    SampleKit();

    void addSample(const char * sample_name_, const char note);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

    void save(tinyxml2::XMLDocument * xmlDoc, tinyxml2::XMLElement * state) override;
    void load(tinyxml2::XMLElement * state) override;
//    void init(const char * sample_name_, int8_t note);
};