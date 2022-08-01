//
// Created by ferluht on 09/07/2022.
//

#include "Chronoblob.h"

void Chronoblob::process(float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime) {
    for (unsigned int i=0; i<nBufferFrames; i+=2 ) {
        pingpong->leftIn = inputBuffer[i+0];
        pingpong->rightIn = inputBuffer[i+1];

        pingpong->process();

        outputBuffer[i+0] = pingpong->leftOut;
        outputBuffer[i+1] = pingpong->rightOut;
    }
};