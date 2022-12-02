//
// Created by ferluht on 09/07/2022.
//

#include "Plateau.h"

Plateau::Plateau() : AudioEffect("PLATEAU") {
    dattorro = new Dattorro();
    dattorro->size = 0.95;
    dattorro->decay = 0.9;

    addMIDIHandler(CC_HEADER, CC_E1, [this](MData &cmd) -> MIDISTATUS {
        dry = (float) cmd.data2 / 127.0;
        return MIDISTATUS::DONE;
    });
}

void Plateau::process(float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, double streamTime) {
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        dattorro->leftInput = inputBuffer[i+0];
        dattorro->rightInput = inputBuffer[i+1];

        dattorro->process();

        outputBuffer[i+0] = dattorro->leftOut * (1 - dry) + inputBuffer[i+0] * dry;
        outputBuffer[i+1] = dattorro->rightOut * (1 - dry) + inputBuffer[i+1] * dry;
    }
}

void Plateau::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    char output[50];
    sprintf(output, "SZ:0.95 | DC:0.9 | D/W:%.2f", dry);
    screen->print(output);
}