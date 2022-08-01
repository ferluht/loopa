//
// Created by ferluht on 08/07/2022.
//

#pragma once
#include <cstdint>

#define SAMPLERATE 44100
#define BUF_SIZE 128
#define PERF_TESTING false

class A {

public:

    A() {

    }

    virtual void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) {};

};

