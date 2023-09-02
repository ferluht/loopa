#include "Delay.hpp"
#include <A.h>
#include <iostream>

void Delay::process(float *outputBuffer, float * inputBuffer,
                    unsigned int nBufferFrames, double streamTime) {
    float linelength = std::round(time->value * (float)SAMPLERATE);
    if (linelength != prevlinelength) {
        historyBufferL.clear();
        historyBufferR.clear();
        lastWetL = 0;
        lastWetR = 0;
    }
    prevlinelength = linelength;

    float CFAmp = 1;
    float CFMode = 1;

    if (linelength > 0.01) {
        for (int i = 0; i < nBufferFrames * 2; i += 2) {

            float inL = inputBuffer[i] + lastWetR * feedback->value;
            float inR = lastWetL;

            if (!historyBufferL.full()) historyBufferL.push(inL);
            if (!historyBufferR.full()) historyBufferR.push(inR);

            wetL = 0.f;
            wetR = 0.f;

            if (!historyBufferL.empty() && historyBufferL.size() > linelength) {
                wetL = historyBufferL.shift();
            }

            if (!historyBufferR.empty() && historyBufferR.size() > linelength) {
                wetR = historyBufferR.shift();
            }

            lastWetL = wetL;
            lastWetR = wetR;

            outputBuffer[i] = crossfade(inputBuffer[i], wetL, drywet->value);
            outputBuffer[i + 1] = crossfade(inputBuffer[i + 1], wetR, drywet->value);
        }
    } else {
        for (int i = 0; i < nBufferFrames * 2; i += 2) {
            outputBuffer[i] = inputBuffer[i];
            outputBuffer[i + 1] = inputBuffer[i + 1];
        }
    }
}

//void Delay::draw(GFXcanvas1 * screen) {
//    screen->setCursor(4, 17);
//    char output[50];
//    sprintf(output, "SZ:0.95 | DC:0.9 | D/W:%.2f", dry);
//    screen->print(output);
//}