//
// Created by ferluht on 09/07/2022.
//

#include "Plateau.h"

void Plateau::process(float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime) {
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        dattorro->leftInput = inputBuffer[i+0];
        dattorro->rightInput = inputBuffer[i+1];

        dattorro->process();

        outputBuffer[i+0] = dattorro->leftOut * 0.5 + inputBuffer[i+0] * 0.5;
        outputBuffer[i+1] = dattorro->rightOut * 0.5 + inputBuffer[i+1] * 0.5;
    }
}

void Plateau::draw(NVGcontext *vg) {
    nvgFontSize(vg, 10);
    nvgText(vg, 2, 12, "PLATEAU", NULL);
}