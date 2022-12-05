//
// Created by ferluht on 07/07/2022.
//

#pragma once

#include <cstdint>
#include <AudioEffects.h>
#include <MidiEffects.h>
#include <Instruments.h>
#include <Rack.h>
#include <unistd.h>
#include <chrono>
#include <mutex>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG{

public:

    DAW(GFXcanvas1 * screen_) : DAW(3, 4, screen_) {};

    DAW (int n_tracks, int n_tapes, GFXcanvas1 * screen_) : AMG("DAW") {
        screen = screen_;

        midiMap = new MIDIMap();
        midiMap->addHardwareControl("CTRL", MIDI::GENERAL::CC_HEADER, 100);
        midiMap->addHardwareControl("SHIFT", MIDI::GENERAL::CC_HEADER, 101);
        midiMap->addHardwareControl("COPY", MIDI::GENERAL::CC_HEADER, 66);

        midiMap->addHardwareControl("P0", MIDI::GENERAL::CC_HEADER, 110);
        midiMap->addHardwareControl("P1", MIDI::GENERAL::CC_HEADER, 111);
        midiMap->addHardwareControl("P2", MIDI::GENERAL::CC_HEADER, 112);
        midiMap->addHardwareControl("P3", MIDI::GENERAL::CC_HEADER, 113);

        midiMap->addHardwareControl("K1", MIDI::GENERAL::CC_HEADER, 60);
        midiMap->addHardwareControl("K2", MIDI::GENERAL::CC_HEADER, 61);
        midiMap->addHardwareControl("K3", MIDI::GENERAL::CC_HEADER, 62);
        midiMap->addHardwareControl("K4", MIDI::GENERAL::CC_HEADER, 63);
        midiMap->addHardwareControl("K5", MIDI::GENERAL::CC_HEADER, 64);
        midiMap->addHardwareControl("K6", MIDI::GENERAL::CC_HEADER, 65);

        midiMap->addMapping({"SHIFT", "K1"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::LEFT);
        midiMap->addMapping({"SHIFT", "K2"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWN);
        midiMap->addMapping({"SHIFT", "K3"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::RIGHT);
        midiMap->addMapping({"SHIFT", "K4"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UP);

        for (int i = 0; i < n_tapes; i ++) {
            auto kn = "P" + std::to_string(i);
            midiMap->addMapping({kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::TRIG);
            midiMap->addMapping({"SHIFT", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::CLEAR);
            midiMap->addMapping({"CTRL", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::DOUBLE);
            midiMap->addMapping({"SHIFT", "CTRL", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::STOP);
            midiMap->addMapping({"COPY", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::COPY);
        }

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::LEFT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_out();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWN, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_next();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::RIGHT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_in();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UP, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_prev();
            return MIDISTATUS::DONE;
        });

        tracks = spawnTracksRack(n_tracks);
        tapes = new LoopMatrix<2,2>();
        focus_rack = tracks;

        dawMidiStatus = MIDISTATUS::DONE;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    MIDISTATUS midiIn(MData& cmd) override;

    void draw(GFXcanvas1 * screen) override;

private:

    GFXcanvas1 * screen;

    std::mutex midiLock;
    std::deque<MData> midiQueue;

    static Rack * spawnTracksRack(int n);
    static Rack * spawnTapeRack(int n);
    static Rack * spawnSingleTrack();
    static Rack * spawnSingleTrack(const char * name, int i, int j, int k);
    static Rack * spawnMidiRack();
    static Rack * spawnInstrumentRack();
    static Rack * spawnEffectRack();

    Rack * tracks;
    LoopMatrix<2,2> * tapes;
    Rack * focus_rack;

    MIDISTATUS dawMidiStatus;

    MIDIMap * midiMap;

    float avg = 0;
    float avg_env = 0;
    inline float envelope(float sample, float w, float w_env) {
        avg = w * sample + (1 - w) * avg;
        float i = std::abs(sample - avg);
        avg_env = w_env * i + (1 - w_env) * avg_env;
        return avg_env;
    }
};

