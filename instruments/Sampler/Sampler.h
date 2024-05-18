//
// Created by ferluht on 28/07/2022.
//

#pragma once

#include "WavFile.hpp"
#include "ADSR.h"
#include "AMG.h"


class SamplerState : public VoiceState{
public:

    SamplerState() {
        adsr.set(0.01, 0.1, 1.0, 5.0);
        out = 0;
        time = 0;
        transient = false;
        alpha = 0;
        speed = -1;
    }

    unsigned char velocity;
    float note;

    ADSR adsr;
    float volume;

    bool transient;
    float time;
    float speed;

    float llim, rlim;

    float out;
    float alpha;
};

class Sampler : public PolyInstrument<SamplerState> {

    enum MODE {
        ONESHOT,
        ADSR,
        LOOPED
    };

    int mode;

    WavFile<float> sample;
    char sample_name[100];
    char display_name[100];

//    GUI::TapButton * trig;
//    GUI::Encoder * pitch;
    bool const_pitch;

    bool recording = false;

    bool triggered = false;

    Parameter * decay, * n_voices;

    float init_speed = 1;
    int playback_start_sample = 0;
    int playback_end_sample = 0;

    int loop_start_sample = 70000;
    int loop_end_sample = 170000;

    const int wf_width = 62;

    bool encoders_loop_mode = false;

//    GUI::Plot<GUI::TimeGraph> * plot;

    std::vector<float> waveformPoints;
    void computePoints(int start_sample, int end_sample, int start_x, int start_y, int w, int h);
    void drawWaveform(GFXcanvas1 * screen, int start_sample, int end_sample, int start_x, int start_y, int w, int h);

public:
    static DeviceFactory* create() { return new Sampler(); }

    Sampler();

    static float InterpolateCubic(float x0, float x1, float x2, float x3, float t);
    static float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t);

    void updateVoice(SamplerState * state, MData &md) override;

//    void MIn(double beat) override ;

    void processVoice(SamplerState * voiceState, float *outputBuffer, float * inputBuffer,
                      unsigned int nBufferFrames, Sync & sync, uint8_t nvoices) override ;

    void save(tinyxml2::XMLDocument * xmlDoc, tinyxml2::XMLElement * state) override;
    void load(tinyxml2::XMLElement * state) override;
    void init(const char * sample_name_, int8_t note);

    void setName(const char * name);
    const char * getName() override;
};
