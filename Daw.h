//
// Created by ferluht on 07/07/2022.
//

#pragma once

#include <cstdint>
#include <AMG.h>
#include <Instruments.h>
#include <AudioEffects.h>
#include <MidiEffects.h>
#include <Ui.h>
#include <unistd.h>
#include <chrono>

#define S1 100
#define S2 101

#define K1 60
#define K2 61
#define K3 62
#define K4 63
#define K5 64
#define K6 65
#define K7 66

#define P1 110
#define P2 111
#define P3 112
#define P4 113

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG{

public:

    DAW() : DAW(3, 4) {};

    DAW (int n_tracks, int n_tapes) {
        for (int i = 0; i < 128; i ++) {
            cc_states[i] = 0;
            cc_handlers[i] = nullptr;
        }
        for (int i = S1; i <= S2; i ++) cc_handlers[i] = &DAW::SHandler;
        for (int i = K1; i <= K7; i ++) cc_handlers[i] = &DAW::KHandler;
        for (int i = 110; i <= n_tapes; i ++) cc_handlers[i] = &DAW::PHandler;

        cc_handlers[105] = &DAW::EHandler;

        tracks = spawnTracksRack(n_tracks);
        tapes = new TapeRack();

        focus_rack = tracks;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    void midiIn(MData& cmd) override;

    void draw(GFXcanvas1 * screen) override;

private:

    static Rack * spawnTracksRack(int n);
    static Rack * spawnTapeRack(int n);
    static Rack * spawnSingleTrack();
    static Rack * spawnSingleTrack(int i, int j, int k);
    static Rack * spawnMidiRack();
    static Rack * spawnInstrumentRack();
    static Rack * spawnEffectRack();

    Rack * tracks;
    TapeRack * tapes;
    Rack * focus_rack;

    uint8_t cc_states[128];
    typedef void (DAW::*CCHandler)(MData & cmd);
    CCHandler cc_handlers[128];

    void SHandler(MData & cmd);
    void KHandler(MData & cmd);
    void PHandler(MData & cmd);
    void EHandler(MData & cmd);

    float avg = 0;
    float avg_env = 0;
    inline float envelope(float sample, float w, float w_env) {
        avg = w * sample + (1 - w) * avg;
        float i = std::abs(sample - avg);
        avg_env = w_env * i + (1 - w_env) * avg_env;
        return avg_env;
    }
};

