#include "Delay.hpp"
#include <A.h>

void Delay::process() {
    // Get input to delay block
    // in = _in;
    // feedback = _feedback;
    feedback = clamp(feedback, 0.f, 1.f);
    float dry = in * (1 - feedback) + lastWet * feedback;

    // Compute delay time in seconds
    // float delay = _delay;
    delay = clamp(delay, 0.f, 1.f);
    delay = 1e-3 * std::pow(10.f / 1e-3, delay);
    // Number of delay samples
    float index = std::round(delay * SAMPLERATE);

    // Push dry sample into history buffer
    if (!historyBuffer.full()) {
        historyBuffer.push(dry);
    }

    // How many samples do we need consume to catch up?
    float consume = index - historyBuffer.size();

    if (outBuffer.empty()) {
        double ratio = 1.f;
        if (std::fabs(consume) >= 16.f) {
            // Here's where the delay magic is. Smooth the ratio depending on how divergent we are from the correct delay time.
            ratio = std::pow(10.f, clamp(consume / 10000.f, -1.f, 1.f));
        }

        SRC_DATA srcData;
        srcData.data_in = (const float*) historyBuffer.startData();
        srcData.data_out = (float*) outBuffer.endData();
        srcData.input_frames = std::min((int) historyBuffer.size(), 16);
        srcData.output_frames = outBuffer.capacity();
        srcData.end_of_input = false;
        srcData.src_ratio = ratio;
        src_process(src, &srcData);
        historyBuffer.startIncr(srcData.input_frames_used);
        outBuffer.endIncr(srcData.output_frames_gen);
    }

    wet = 0.f;
    if (!outBuffer.empty()) {
        wet = outBuffer.shift();
    }

    // // Apply color to delay wet output
    // float color = params[COLOR_PARAM].getValue() + inputs[COLOR_INPUT].getVoltage() / 10.f;
    // color = clamp(color, 0.f, 1.f);
    // float colorFreq = std::pow(100.f, 2.f * color - 1.f);

    // float lowpassFreq = clamp(20000.f * colorFreq, 20.f, 20000.f);
    // lowpassFilter.setCutoffFreq(lowpassFreq / 44100);
    // lowpassFilter.process(wet);
    // wet = lowpassFilter.lowpass();

    // float highpassFreq = clamp(20.f * colorFreq, 20.f, 20000.f);
    // highpassFilter.setCutoff(highpassFreq / 44100);
    // highpassFilter.process(wet);
    // wet = highpassFilter.highpass();

    lastWet = wet;

    // float mix = _mix;
    mix = clamp(mix, 0.f, 1.f);
    out = crossfade(in, wet, mix);

    // return out;
}