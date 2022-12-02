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
#include <mutex>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG{

public:

    DAW(GFXcanvas1 * screen_) : DAW(3, 4, screen_) {};

    DAW (int n_tracks, int n_tapes, GFXcanvas1 * screen_) : AMG("DAW") {
        screen = screen_;

        midiMap = new MIDIMap();
        midiMap->addHardwareControl("CTRL", CC_HEADER, 100);
        midiMap->addHardwareControl("SHIFT", CC_HEADER, 101);

        midiMap->addHardwareControl("P1", CC_HEADER, 110);
        midiMap->addHardwareControl("P2", CC_HEADER, 111);
        midiMap->addHardwareControl("P3", CC_HEADER, 112);
        midiMap->addHardwareControl("P4", CC_HEADER, 113);

        midiMap->addHardwareControl("K1", CC_HEADER, 60);
        midiMap->addHardwareControl("K2", CC_HEADER, 61);
        midiMap->addHardwareControl("K3", CC_HEADER, 62);
        midiMap->addHardwareControl("K4", CC_HEADER, 63);
        midiMap->addHardwareControl("K5", CC_HEADER, 64);
        midiMap->addHardwareControl("K6", CC_HEADER, 65);
        midiMap->addHardwareControl("K7", CC_HEADER, 66);

        midiMap->addMapping({"SHIFT", "K1"}, DAW_CTRL_HEADER, MIDICC::DAW_LEFT);
        midiMap->addMapping({"SHIFT", "K2"}, DAW_CTRL_HEADER, MIDICC::DAW_DOWN);
        midiMap->addMapping({"SHIFT", "K3"}, DAW_CTRL_HEADER, MIDICC::DAW_RIGHT);
        midiMap->addMapping({"SHIFT", "K4"}, DAW_CTRL_HEADER, MIDICC::DAW_UP);

        midiMap->addMapping({"P1"}, CC_HEADER + 0, MIDICC::TAPE_TRIG);
        midiMap->addMapping({"SHIFT", "P1"}, CC_HEADER + 0, MIDICC::TAPE_CLEAR);
        midiMap->addMapping({"CTRL", "P1"}, CC_HEADER + 0, MIDICC::TAPE_DOUBLE);
        midiMap->addMapping({"SHIFT", "CTRL", "P1"}, CC_HEADER + 0, MIDICC::TAPE_STOP);

        midiMap->addMapping({"P2"}, CC_HEADER + 1, MIDICC::TAPE_TRIG);
        midiMap->addMapping({"SHIFT", "P2"}, CC_HEADER + 1, MIDICC::TAPE_CLEAR);
        midiMap->addMapping({"CTRL", "P2"}, CC_HEADER + 1, MIDICC::TAPE_DOUBLE);
        midiMap->addMapping({"SHIFT", "CTRL", "P2"}, CC_HEADER + 1, MIDICC::TAPE_STOP);

        midiMap->addMapping({"P3"}, CC_HEADER + 2, MIDICC::TAPE_TRIG);
        midiMap->addMapping({"SHIFT", "P3"}, CC_HEADER + 2, MIDICC::TAPE_CLEAR);
        midiMap->addMapping({"CTRL", "P3"}, CC_HEADER + 2, MIDICC::TAPE_DOUBLE);
        midiMap->addMapping({"SHIFT", "CTRL", "P3"}, CC_HEADER + 2, MIDICC::TAPE_STOP);

        midiMap->addMapping({"P4"}, CC_HEADER + 3, MIDICC::TAPE_TRIG);
        midiMap->addMapping({"SHIFT", "P4"}, CC_HEADER + 3, MIDICC::TAPE_CLEAR);
        midiMap->addMapping({"CTRL", "P4"}, CC_HEADER + 3, MIDICC::TAPE_DOUBLE);
        midiMap->addMapping({"SHIFT", "CTRL", "P4"}, CC_HEADER + 3, MIDICC::TAPE_STOP);

        addMIDIHandler(DAW_CTRL_HEADER, MIDICC::DAW_LEFT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_out();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(DAW_CTRL_HEADER, MIDICC::DAW_DOWN, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_next();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(DAW_CTRL_HEADER, MIDICC::DAW_RIGHT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_in();
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(DAW_CTRL_HEADER, MIDICC::DAW_UP, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0)
                this->focus_rack = this->focus_rack->dive_prev();
            return MIDISTATUS::DONE;
        });

        tracks = spawnTracksRack(n_tracks);
        tapes = new TapeRack();
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
    TapeRack * tapes;
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

