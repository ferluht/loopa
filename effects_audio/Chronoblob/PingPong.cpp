#include "PingPong.hpp"

void PingPong::updateParam() {
//    leftDelay->delay = leftTime;
//    rightDelay->delay = rightTime;
//
//    leftOut = 0;
//    rightOut = 0;
//
//    leftDelay->feedback = CFFb->get();
//    rightDelay->feedback = CFFb->get();
//    leftDelay->mix = CFDW->get();
//    rightDelay->mix = CFDW->get();
}

void PingPong::process(float *outputBuffer, float * inputBuffer,
                       unsigned int nBufferFrames, double streamTime) {
    updateParam();

//    if (!mode) {
//        rightDelay->in = (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * CFMode->get() + (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * (1 - CFMode->get());;
//        leftDelay->in = (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * CFMode->get() + (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * (1 - CFMode->get());
//    } else {
//        rightDelay->in = (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * CFMode->get() + (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * (1 - CFMode->get());
//        leftDelay->in = (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * CFMode->get() + (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * (1 - CFMode->get());
//    }

    leftDelay->process(outputBuffer, inputBuffer, nBufferFrames, streamTime);
//    rightDelay->process();

//    leftOut = leftDelay->out;
//    rightOut = leftOut;//rightDelay->out;
}