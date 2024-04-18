//
// Created by ferluht on 28/07/2022.
//

#include "Sampler.h"
#include <numeric>
#include <algorithm>

Sampler::Sampler(const char * name, const char * sample_name_, int8_t note) : PolyInstrument<SamplerState>(name) {
    sample_name = sample_name_;

    mode = MODE::ADSR;

    base_note = note;

    const_pitch = false;

    sample.load(sample_name);

    base_frequency = sample.getSampleRate() / 2 / M_PI;

    addParameter("PITCH");
    n_voices = addParameter("VOICS", 0.4);
    addParameter("FILTR");
    decay = addParameter("DECAY", 0.15);

    int wf_width = 62;

    computePoints(0, sample.getNumSamplesPerChannel(), 2, 18, wf_width, 7);

    addMIDIHandler({SCREENS::TRACK_VIEW}, {MIDI::GENERAL::LOOP_HEADER},
                   {MIDI::UI::TAPE::TRIG}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        init_speed *= -1;
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addMIDIHandler({SCREENS::TRACK_VIEW}, {MIDI::GENERAL::LOOP_HEADER+2},
                   {MIDI::UI::TAPE::TRIG}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (mode == MODE::ONESHOT) mode = MODE::ADSR;
        else if (mode == MODE::ADSR) mode = MODE::LOOPED;
        else if (mode == MODE::LOOPED) mode = MODE::ONESHOT;
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addMIDIHandler({SCREENS::TRACK_VIEW}, {MIDI::GENERAL::LOOP_HEADER+3},
                   {MIDI::UI::TAPE::TRIG}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        instrument_volume += 0.1;
        if (instrument_volume > 1) instrument_volume = 0;
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addMIDIHandler({SCREENS::TRACK_VIEW}, {MIDI::GENERAL::CTRL_HEADER},
                   {MIDI::UI::PLAY_BUTTON}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 > 0) {
            this->clear_input = false;
            for (auto &v : voices) v->disable();
            voices.back()->enable();
            sample.clearAudioBuffer();
            recording = true;
        } else {
            this->clear_input = true;
            voices.back()->disable();
            computePoints(0, sample.getNumSamplesPerChannel(), 2, 18, 62, 7);
            recording = false;
        }
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addDrawHandler({SCREENS::TRACK_VIEW}, [this, wf_width](GFXcanvas1 * screen) -> void {

        if (!recording)
            drawWaveform(screen, 0, sample.getNumSamplesPerChannel(), 2, 18, wf_width, 7);
        else {
            screen->setCursor(36, 24);
            screen->print("REC");
        }

        screen->setCursor(71, 17);
        screen->print("DIR  ");
        if (init_speed > 0) screen->print("->");
        else screen->print("<-");

        screen->setCursor(71, 28);
        screen->print("MOD ");
        if (mode == MODE::ONESHOT) screen->print("=>|");
        else if (mode == MODE::ADSR) screen->print("->|");
        else if (mode == MODE::LOOPED) screen->print("<->");

        screen->setCursor(100, 17);
        screen->print("TON ");
        if (init_speed > -10) screen->print("0");
        else screen->print("0");

        screen->setCursor(100, 28);
        screen->print("VOL ");
        char vol[4];
        sprintf(vol, "%.2f", instrument_volume);
        screen->print(vol);

        if (!recording) {
            int bt = 2 + (float) playback_start_sample / (float) sample.getNumSamplesPerChannel() * wf_width;
            int et = 2 + (float) playback_end_sample / (float) sample.getNumSamplesPerChannel() * wf_width;
            int blt = 2 + (float) loop_start_sample / (float) sample.getNumSamplesPerChannel() * wf_width;
            int elt = 2 + (float) loop_end_sample / (float) sample.getNumSamplesPerChannel() * wf_width;
            int yt = 31;
            screen->drawTriangle(bt - 2, yt, bt + 2, yt, bt, yt - 4, 1);
            screen->drawFastVLine(bt, yt - 7, -6, 1);
            screen->drawTriangle(et - 2, yt, et + 2, yt, et, yt - 4, 1);
            screen->drawFastVLine(et, yt - 7, -6, 1);

            screen->drawFastHLine(blt - 2, yt, 5, 1);
            screen->drawPixel(blt - 1, yt - 1, 1);
            screen->drawPixel(blt + 1, yt - 1, 1);
            screen->drawFastVLine(blt, yt - 4, 4, 1);
            screen->drawFastVLine(blt, yt - 7, -6, 1);

            screen->drawFastHLine(elt - 2, yt, 5, 1);
            screen->drawPixel(elt - 1, yt - 1, 1);
            screen->drawPixel(elt + 1, yt - 1, 1);
            screen->drawFastVLine(elt, yt - 4, 4, 1);
            screen->drawFastVLine(elt, yt - 7, -6, 1);

            screen->drawFastHLine(0, 18, 2, 1);
            screen->drawFastVLine(66, 8, 24, 1);
        } else {
            screen->setLed(0, 50, 0, 0);
        }
    });

    addMIDIHandler({SCREENS::TRACK_VIEW}, {MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2, CC_E3, CC_E4}, [this](MData &cmd, Sync &sync) -> void {

        if (cmd.data2 > 0)
            switch (cmd.data1) {
                case CC_E1:
                    playback_start_sample += (cmd.data2 - 64) * 100;
                    break;
                case CC_E2:
                    playback_end_sample += (cmd.data2 - 64) * 100;
                    break;
                case CC_E3:
                    loop_start_sample += (cmd.data2 - 64) * 100;
                    break;
                case CC_E4:
                    loop_end_sample += (cmd.data2 - 64) * 100;
                    break;
                default:
                    break;
            }

        if (playback_start_sample > playback_end_sample) playback_start_sample = playback_end_sample;
        if (playback_start_sample < 0) playback_start_sample = 0;
        if (playback_end_sample < playback_start_sample) playback_end_sample = playback_start_sample;
        if (playback_end_sample > sample.getNumSamplesPerChannel()) playback_end_sample = sample.getNumSamplesPerChannel();
        if (playback_start_sample > loop_start_sample) loop_start_sample = playback_start_sample;
        if (playback_start_sample > loop_end_sample) loop_end_sample = playback_start_sample;
        if (playback_end_sample < loop_start_sample) loop_start_sample = playback_end_sample;
        if (playback_end_sample < loop_end_sample) loop_end_sample = playback_end_sample;
        if (loop_end_sample < loop_start_sample) loop_end_sample = loop_start_sample;

        cmd.status = MIDI::GENERAL::INVALIDATED;
    });
}

void Sampler::computePoints(int start_sample, int end_sample, int start_x, int start_y, int w, int h) {
    waveformPoints.clear();
    int step = (end_sample - start_sample) / w;
    float mag = 0;
    for (int i = 0; i < w; i++) {
        float p = std::accumulate(sample.samples.back().begin() + step * i,
                                  sample.samples.back().begin() + step * (i + 1),
                                  0.f) / step;
        waveformPoints.push_back(p);
        if (abs(p) > mag) mag = abs(p);
    }
    for (int i = 0; i < w; i++) waveformPoints[i] = -waveformPoints[i] / mag;

    float peak = 0;
    for (int j = 0; j < sample.getNumChannels(); j++)
        for (int i = 0; i < sample.getNumSamplesPerChannel(); i++)
            if (abs(sample.samples[j][i]) > peak) peak = abs(sample.samples[j][i]);

    for (int j = 0; j < sample.getNumChannels(); j++)
        for (int i = 0; i < sample.getNumSamplesPerChannel(); i++)
            sample.samples[j][i] /= peak;

    playback_start_sample = 0;
    playback_end_sample = sample.getNumSamplesPerChannel();

    loop_start_sample = playback_end_sample * 0.2;
    loop_end_sample = playback_end_sample * 0.5;
}

void Sampler::drawWaveform(GFXcanvas1 * screen,
                           int start_sample, int end_sample,
                           int start_x, int start_y,
                           int w, int h) {
    for (int i = 0; i < w; i++) {
        if (i > 0) screen->drawLine(start_x + i - 1, start_y + waveformPoints[i-1] * h, start_x + i, start_y + waveformPoints[i] * h, 1);
        else screen->drawLine(start_x, start_y, start_x + i, start_y + waveformPoints[i] * h, 1);
        for (const auto &v : voices) {
            if (v->isActive()) {
                screen->drawFastVLine(start_x + v->time / sample.getNumSamplesPerChannel() * w,
                                      start_y - h / 2 * v->adsr.get(), h * v->adsr.get() + 1, 1);
            }
        }
    }
}

void Sampler::updateVoice(SamplerState *state, MData &md) {
    if (((md.status & 0xF0) == MIDI::GENERAL::NOTEON_HEADER) && (md.data2 != 0)) {
        if (const_pitch) state->note = base_note;
        else state->note = md.data1;
        state->volume = (float)md.data2/127.f;
        if (state->isActive()){
            state->transient = true;
            state->alpha = 0.001;
        }
        state->adsr.set(0.01, 0.1, 1.0, decay->value * 5);
        state->adsr.gateOn();
        if (mode == MODE::ONESHOT) state->adsr.forcePeak();
        if (init_speed > 0) state->time = playback_start_sample + 5;
        else state->time = playback_end_sample - 5;
        state->llim = playback_start_sample;
        state->rlim = playback_end_sample;
        state->speed = init_speed;
        state->enable();

//        set_voices((int)(voices->value * 5) + 1);
//        std::cout << "note" << std::endl;
    }else{
        state->adsr.gateOff();
    }
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
                           unsigned int nBufferFrames, Sync & sync, uint8_t nvoices) {

    if (recording && voiceState->isActive()) {
        sample.appendAudioBuffer(inputBuffer, nBufferFrames, 2);
        for (unsigned int i = 0; i < 2*nBufferFrames; i ++)
            outputBuffer[i] = inputBuffer[i];
        return;
    }

    float vol = instrument_volume * instrument_volume;
    std::vector<float> & samples = sample.samples[0];

    auto & alpha = voiceState->alpha;
    auto & out = voiceState->out;
    auto & time = voiceState->time;
    auto & adsr = voiceState->adsr;
    auto & speed = voiceState->speed;
    auto & note = voiceState->note;

    for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
        if (!voiceState->isActive()) continue;

        if (voiceState->time + 20 >= voiceState->rlim && voiceState->speed != -1 && mode == MODE::LOOPED) {
            voiceState->speed -= 0.05;
            voiceState->llim = loop_start_sample;
            if (voiceState->speed < -1) voiceState->speed = -1;
        }

        if (voiceState->time - 20 <= voiceState->llim && voiceState->speed != 1 && mode == MODE::LOOPED) {
            voiceState->speed += 0.05;
            voiceState->rlim = loop_end_sample;
            if (voiceState->speed > 1) voiceState->speed = 1;
        }

        if (voiceState->time + 4 >= voiceState->rlim && voiceState->speed > 0 && mode != MODE::LOOPED) {
            voiceState->disable();
            continue;
        }

        if (voiceState->time - 4 <= voiceState->llim && voiceState->speed < 0 && mode != MODE::LOOPED) {
            voiceState->disable();
            continue;
        }

        if (mode != MODE::ONESHOT) adsr.process();

        float x = InterpolateHermite4pt3oX(
                samples[(int) time - 1],
                samples[(int) time + 0],
                samples[(int) time + 1],
                samples[(int) time + 2],
                time - (int) time) * voiceState->volume;

        if (voiceState->transient) {
            out = alpha * alpha * x + (1 - alpha * alpha) * out;
            alpha += 0.003;
        } else {
            if (time < 200) {
                x *= (time / 200) * (time / 200);
            }
            out = x;
        }

        if (alpha > 1) {
            voiceState->transient = false;
        }

        float adsr_val = adsr.get();
        outputBuffer[i + 0] += out * vol * adsr_val;
        outputBuffer[i + 1] += out * vol * adsr_val;

        if (adsr.end()) voiceState->disable();

        time += speed * getPhaseIncrement(voiceState->note);// + *pitch);
    }
}