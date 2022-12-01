//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include <mutex>
#include <cmath>
#include <set>
#include <iostream>
#include "AMG.h"


class VoiceState {

    unsigned char key;
    bool active;

public:

    VoiceState() {
        active = false;
    };

    inline void enable() { active = true; }
    inline void disable() { active = false; }
    inline bool isActive() { return active; }

    template <class VoiceState> friend class PolyInstrument;
};

class Instrument : public AMG {
public:
    Instrument(const char * name) : AMG(name) {

    }
};

template <class TVoiceState>
class PolyInstrument : public Instrument {

    unsigned int num_voices = 4;
    std::list<TVoiceState *> voices;

    std::mutex keyPressedLock;

//    bool damper_pedal;
//    bool portamento;
//    bool sostenuto;
//    bool soft_pedal;
//    bool chorus;
//    bool celeste;
//    bool phaser;

    int _prev_active_voices = 0;

public:

    float base_note = 69.0;
    float power_base = 2.0;
    float semitones = 12.0;
    float instrument_volume = 1;
    double base_frequency = 440.0;

    float pitch = 0;
    float pitch_distance = 12.0;

    PolyInstrument(const char * name) : Instrument(name){
        pitch = 0;
        for (int i = 0; i < num_voices; i++) voices.push_back(new TVoiceState());

        addHandler({NOTEON_HEADER, NOTEOFF_HEADER}, [this](MData &cmd) -> MIDISTATUS {
            keyPressed(cmd);
            return MIDISTATUS::DONE;
        });

        addHandler(PITCHWHEEL_HEADER, [this](MData &cmd) -> MIDISTATUS {
            pitch = (cmd.data1 + cmd.data2*128.0f) / (float)0xFFFF * pitch_distance;
            return MIDISTATUS::DONE;
        });
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void keyPressed(MData& md);

    inline float getFrequency(float note)
    {
        return base_frequency*(powf(power_base, (note - base_note + pitch)/semitones));
    }

    inline float getPhaseIncrement(float note)
    {
        return 2*M_PI*getFrequency(note) / SAMPLERATE;
    }

    void set_voices(uint8_t nvoices) {
        while (voices.size() > nvoices) voices.pop_front();
        while (voices.size() < nvoices) voices.push_back(new TVoiceState());
    }

    virtual void updateVoice(TVoiceState * voiceState, MData cmd) {};

    virtual void processVoice(TVoiceState * voiceState, float *outputBuffer, float * inputBuffer,
                              unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {}
};

template <class TVoiceState>
void PolyInstrument<TVoiceState>::keyPressed(MData &md) {
    keyPressedLock.lock();

    for (auto it = voices.begin(); it != voices.end(); it++ )
    {
        if ((*it)->key == md.data1){
            TVoiceState * state = (*it);
            voices.erase(it);
            updateVoice(state, md);
            state->key = md.data1;
            voices.push_back(state);
            keyPressedLock.unlock();
            return;
        }
    }

    for (auto it = voices.begin(); it != voices.end(); it++ )
    {
        if (!(*it)->isActive()){
            TVoiceState * state = (*it);
            voices.erase(it);
            updateVoice(state, md);
            state->key = md.data1;
            voices.push_back(state);
            keyPressedLock.unlock();
            return;
        }
    }

    TVoiceState * state = *voices.begin();
    updateVoice(state, md);
    state->key = md.data1;
    voices.pop_front();
    voices.push_back(state);

    keyPressedLock.unlock();
}

template <class TVoiceState>
void PolyInstrument<TVoiceState>::process(float *outputBuffer, float * inputBuffer,
                                unsigned int nBufferFrames, double streamTime)
{
    keyPressedLock.lock();

    for (auto it = voices.begin(); it != voices.end(); it++ ){
        if (PERF_TESTING || (*it)->isActive())
            processVoice(*it, outputBuffer, inputBuffer, nBufferFrames, streamTime, num_voices);
    }

    keyPressedLock.unlock();
}
