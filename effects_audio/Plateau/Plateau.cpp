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

void Plateau::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 18);
    screen->setTextSize(1);
    screen->print("SZ:0.95 | DC:0.9 | D/W:0.5");
}