#include "PingPong.hpp"

void PingPong::updateParam() {
    leftDelay->delay = leftTime;
    rightDelay->delay = rightTime;

    leftOut = 0;
    rightOut = 0;

    leftDelay->feedback = CFFb->get();
    rightDelay->feedback = CFFb->get();
    leftDelay->mix = CFDW->get();
    rightDelay->mix = CFDW->get();
}

void PingPong::process() {
    updateParam();

    if ((prevSync != sync) && (sync)) {
        CFDW->set(1);
        CFFb->set(1);
        CFAmp->set(1);
    }

    if ((prevMode != mode) && (mode)) {
        CFMode->set(1);
    }

    if ((prevMode != mode) && (!mode)) {
        CFMode->set(0);
    }

    if (!sync) {
        CFDW->set(dw);
        CFFb->set(feedback);
        CFAmp->set(0);
    }

    if (!mode) {
        rightDelay->in = (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * CFMode->get() + (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * (1 - CFMode->get());;
        leftDelay->in = (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * CFMode->get() + (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * (1 - CFMode->get());
    } else {
        rightDelay->in = (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * CFMode->get() + (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * (1 - CFMode->get());
        leftDelay->in = (rightIn * (1 - CFAmp->get()) + rightDelay->out * CFAmp->get()) * CFMode->get() + (leftIn * (1 - CFAmp->get()) + leftDelay->out * CFAmp->get()) * (1 - CFMode->get());
    }

    leftDelay->process();
//    rightDelay->process();

    leftOut = leftDelay->out;
    rightOut = leftOut;//rightDelay->out;

    prevSync = sync;
    prevMode = mode;
}