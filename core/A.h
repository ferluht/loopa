//
// Created by ferluht on 08/07/2022.
//

#pragma once
#include <cstdint>
#include "Sync.h"

#define SAMPLERATE 44100
#define AUDIO_BUF_SIZE 256
#define PERF_TESTING false

/**
 * A stands for Audio. The base class for all audio objects.
*/
class A {

public:

    A() {

    }

    /**
     * Virtual audio process method
     *
     * @param outputBuffer output buffer of floats
     * @param inputBuffer input buffer of floats
     * @param nBufferFrames amount samples in buffers
     * @param sync time sync object
     *
     * Pass-thru example of usage in child class:
     * @code
     * void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, Sync & sync) override {
     *   for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
     *     outputBuffer[i + 0] = inputBuffer[i + 0];
     *     outputBuffer[i + 1] = inputBuffer[i + 1];
     *   }
     * }
     * @endcode
     */
    virtual void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) {};

};

