//
// Created by ferluht on 09/07/2022.
//

#include "Plateau.h"

Plateau::Plateau() : AudioEffect("Plateau") {
    dattorro = new Dattorro();

    drywet = addParameter("D/W", 0, 1, 0.6, 0.025);
    size = addParameter("SIZE", 0, 1, 0.95, 0.025);
    decay = addParameter("DECA", 0, 1, 0.9, 0.025);
}

void Plateau::process(float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, Sync & sync) {
    float dry = 1 - drywet->getVal();
    dattorro->size = size->getVal();
    dattorro->decay = decay->getVal();
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        dattorro->leftInput = inputBuffer[i+0];
        dattorro->rightInput = inputBuffer[i+1];

        dattorro->process();

        outputBuffer[i+0] = dattorro->leftOut * (1 - dry) + inputBuffer[i+0] * dry;
        outputBuffer[i+1] = dattorro->rightOut * (1 - dry) + inputBuffer[i+1] * dry;
    }
}

namespace {
    DeviceFactory::AddToRegistry<Plateau> _("Plateau");
}