//
// Created by ferluht on 13/08/2023.
//

#include "Tanhx.h"

void Tanhx::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    float sat_drive = db_to_k(driveP->getVal());
    float bias_offset = biasP->getVal() / 10 * 0.1;
    sL.setSatThreshold(db_to_k(14) / sat_drive, bias_offset);
    sR.setSatThreshold(db_to_k(14) / sat_drive, bias_offset);
    float output_gain = db_to_k(outputP->getVal());
    sL.use_emphasis = emphasisP->getVal() > 0;
    sR.use_emphasis = emphasisP->getVal() > 0;

    for (int i = 0; i < nBufferFrames * 2; i += 2) {
        outputBuffer[i + 0] = sL.processSample(inputBuffer[i + 0]) * output_gain;
        outputBuffer[i + 1] = sR.processSample(inputBuffer[i + 1]) * output_gain;
    }
}