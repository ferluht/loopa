//
// Created by ferluht on 28/07/2022.
//

#include "Sampler.h"

Sampler::Sampler(const char * sample_name_) {
    sample_name = sample_name_;

    const_pitch = false;

    sample.load(sample_name);

//    pitch = new GUI::Encoder("pitch", 0, -24, 24);
//    pitch->GPlace({0.1, 0.73});
//    pitch->GSetHeight(0.25);
//    GAttach(pitch);
//    MConnect(pitch);

    base_frequency = SAMPLERATE / 2 / M_PI;

//    trig = new GUI::TapButton("trig", [this] (bool state) {triggered = true;});
//    trig->GPlace({0.75, 0.75});
//    trig->GSetHeight(0.2);
//    trig->GSetWidth(0.2);
//    GAttach(trig);
//    MConnect(trig);
}

//void Sampler::MRender(double beat) {
//    if (triggered) {
//        MIn({beat, NOTEON_HEADER, 62, 100});
//        triggered = false;
//    }
//    pitch->MRender(beat);
//}

void Sampler::updateVoice(SamplerState *state, MData md) {
    if (((md.status & 0xF0) == NOTEON_HEADER) && (md.data2 != 0)) {
        if (const_pitch) state->note = base_note;
        else state->note = md.data1;
        state->volume = (float)md.data2/127.f;
        if (state->isActive()){
            state->transient = true;
            state->alpha = 0.001;
        }
        state->time = 0;
        state->enable();
//        std::cout << "note" << std::endl;
    }else{

    }
}

float Sampler::InterpolateCubic(float x0, float x1, float x2, float x3, float t)
{
    float a0, a1, a2, a3;
    a0 = x3 - x2 - x0 + x1;
    a1 = x0 - x1 - a0;
    a2 = x2 - x0;
    a3 = x1;
    return (a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + (a3);
}

float Sampler::InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t)
{
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}

void Sampler::processVoice(SamplerState *voiceState, float *outputBuffer, float *inputBuffer,
                           unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {

    for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
        if (!voiceState->isActive()) continue;
        if (voiceState->time + 4 < sample.getNumSamplesPerChannel()) {

            float x = InterpolateHermite4pt3oX(
                    sample.samples[0][(int) voiceState->time],
                    sample.samples[0][((int) voiceState->time) + 1],
                    sample.samples[0][((int) voiceState->time) + 2],
                    sample.samples[0][((int) voiceState->time) + 3], voiceState->time - (int) voiceState->time) *
                      voiceState->volume;

            if (voiceState->transient) {
                voiceState->out = voiceState->alpha * voiceState->alpha * x +
                                  (1 - voiceState->alpha * voiceState->alpha) * voiceState->out;
                voiceState->alpha += 0.003;
            } else {

                if (voiceState->time < 200) {
                    x *= (voiceState->time / 200) * (voiceState->time / 200);
                }

                voiceState->out = x;
            }

            if (voiceState->alpha > 1) {
                voiceState->transient = false;
            }

            outputBuffer[i + 0] += voiceState->out * instrument_volume;
            outputBuffer[i + 1] += voiceState->out * instrument_volume;
//            std::cout << voiceState->out << std::endl;
            voiceState->time += getPhaseIncrement(voiceState->note);// + *pitch);

        } else {
            voiceState->disable();
//            std::cout << "note off" << std::endl;
//            sample.printSummary();
        }
    }
}

void Sampler::draw(NVGcontext *vg) {
    nvgFontSize(vg, 10);
    nvgText(vg, 2, 12, "SAMPLER", NULL);
}