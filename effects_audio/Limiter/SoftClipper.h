//
// Created by ferluht on 08/07/2022.
//

#ifndef TEST_ALSA_SOFTCLIPPER_H
#define TEST_ALSA_SOFTCLIPPER_H

#include "A.h"
#include "Utils.hpp"
#include <cmath>

/// @private
class SoftClipper : public A {
public:

    SoftClipper() : SoftClipper(5) {}
    SoftClipper(float a) {
        this->setAlpha(a);
    }

    void setAlpha(float alpha);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override {
        for (unsigned int i=0; i<nBufferFrames; i+=2 ) {
            outputBuffer[i+0] = soft_clip(inputBuffer[i+0]);
            outputBuffer[i+1] = soft_clip(inputBuffer[i+1]);
        }
    };

private:
    float alpha;
};


#endif //TEST_ALSA_SOFTCLIPPER_H
