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

    bool altparams;
    std::vector<Parameter> inputparams;

public:

    Instrument(const char * name) : AMG(name) {
        altparams = false;
        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::ALT_PARAMS, [this](MData &cmd) -> MIDISTATUS {
            this->altparams = cmd.data2 > 0;
            return MIDISTATUS::DONE;
        });

        addMIDIHandler({MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2}, [this](MData &cmd) -> MIDISTATUS {
            if (this->altparams) {
                if (cmd.data1 == CC_E1 && inputparams.size() > 2) inputparams[2].update(cmd.data2);
                if (cmd.data1 == CC_E2 && inputparams.size() > 3) inputparams[3].update(cmd.data2);
            } else {
                if (cmd.data1 == CC_E1 && inputparams.size() > 0) inputparams[0].update(cmd.data2);
                if (cmd.data1 == CC_E2 && inputparams.size() > 1) inputparams[1].update(cmd.data2);
            }
            return MIDISTATUS::DONE;
        });
    }

    void draw(GFXcanvas1 * screen) override {

        for (int i = 0; i < 4; i ++) {
            if (inputparams.size() <= i || !inputparams[i].enabled) continue;
            int xoffset = (int)(i / 2) * 46;
            int yoffset = (i % 2) * 10;
            screen->setCursor(4 + xoffset, 17 + yoffset);
            screen->setTextSize(1);
            screen->print(inputparams[i].name.c_str());
            screen->drawRect(26 + xoffset, 14 + yoffset, 20, 4, 1);
            screen->drawRect(26 + xoffset, 15 + yoffset, inputparams[i].value * 19 + 1, 2, 1);
        }

//        screen->print("par1");
    }

    Parameter * addParameter(std::string name) {
        return addParameter(name, 0);
    }


    Parameter * addParameter(std::string name, float default_value) {
        inputparams.emplace_back();
        inputparams.back().name = name;
        inputparams.back().enabled = true;
        inputparams.back().value = default_value;
        return &inputparams.back();
    }
};

template <class TVoiceState>
class PolyInstrument : public Instrument {

    unsigned int num_voices = 6;
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

    float base_note = 57.0;
    float power_base = 2.0;
    float semitones = 12.0;
    float instrument_volume = 1;
    double base_frequency = 440.0;

    float pitch = 0;
    float pitch_distance = 2.0;

    PolyInstrument(const char * name) : Instrument(name){
        pitch = 0;
        for (int i = 0; i < num_voices; i++) voices.push_back(new TVoiceState());

        addMIDIHandler({MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, [this](MData &cmd) -> MIDISTATUS {
            keyPressed(cmd);
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::PITCHWHEEL_HEADER, [this](MData &cmd) -> MIDISTATUS {
            pitch = ((float)((cmd.data2 << 7) + cmd.data1) / 16384.f - 0.5) * 2 * pitch_distance;
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
                              unsigned int nBufferFrames, double streamTime, uint8_t nvoices) {};

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
